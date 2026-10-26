#include "TacticalSoldierMotion.h"

namespace gunyumotion
{
    namespace
    {
        float Ease(float A,float B,float P) { return FMath::SmoothStep(A,B,P); }
        float Pulse(float A,float B,float C,float D,float P) { return Ease(A,B,P)*(1-Ease(C,D,P)); }
        bool Strike(EBattleCue K)
        { return K==EBattleCue::Strike || K==EBattleCue::Ambush || K==EBattleCue::Breakthrough || K==EBattleCue::Feint || K==EBattleCue::Disrupt || K==EBattleCue::AzureDragon; }
        bool Displayed(const TArray<FVisualUnit>& V,int I) { return I>=0 && V.IsValidIndex(I) && V[I].Id==I; }
        bool Seen(const TArray<FVisualUnit>& V,int I) { return Displayed(V,I) && V[I].Troops>0; }
        gunyuformation::EShape Rest(FVisualUnit U) { U.Motion=0;return gunyuformation::Shape(U,false); }
    }
    FCompany Read(const FVisualUnit& U,const TArray<FVisualUnit>& Visible,const FBattleBeat* B,float P)
    {
        FCompany M;M.Progress=FMath::Clamp(P,0.f,1.f);M.Facing=U.bEnemy ? -FVector::ForwardVector:FVector::ForwardVector;
        M.Before=M.Rest=M.Action=Rest(U);M.Brace=U.bGuarded ? 1.f:0.f;M.Stature=U.bConcealed ? .65f:1.f;
        if(U.Id<0 || !B || !Seen(B->Before,U.Id) || !B->After.IsValidIndex(U.Id)) return M;
        M.Before=Rest(B->Before[U.Id]);M.StateBlend=Ease(.48f,.84f,P);
        M.Stature=FMath::Lerp(B->Before[U.Id].bConcealed ? .65f:1.f,M.Stature,Ease(.48f,.70f,P));
        M.Brace=FMath::Lerp(B->Before[U.Id].bGuarded ? 1.f:0.f,M.Brace,M.StateBlend);
        const bool TargetSeen=Displayed(Visible,B->Target) && Seen(B->Before,B->Target);
        const FVector Travel=B->After[U.Id].Id==U.Id ? B->After[U.Id].Position-B->Before[U.Id].Position:FVector::ZeroVector;
        if(Travel.SizeSquared2D()>1)
        {
            M.Facing=Travel.GetSafeNormal2D();
            float Start=.18f,End=.78f;
            if(B->Kind==EBattleCue::Lure) { Start=U.Id==B->Actor ? .12f:.38f;End=U.Id==B->Actor ? .57f:.86f; }
            if(B->Kind==EBattleCue::Breakthrough) { Start=U.Id==B->Actor ? .26f:.46f;End=.82f; }
            M.March=Pulse(Start-.06f,Start+.08f,End-.08f,End+.06f,P);
        }
        if(B->Kind!=EBattleCue::Stop && U.Id==B->Actor)
        {
            M.bAttack=TargetSeen && Strike(B->Kind);
            if(M.bAttack) M.Facing=(Visible[B->Target].Position-U.Position).GetSafeNormal2D();
            M.Salute=B->Kind==EBattleCue::Rally || (B->Kind==EBattleCue::SerpentRoar && TargetSeen) ? Pulse(.14f,.34f,.66f,.90f,P):0;
        }
        if(B->Kind==EBattleCue::Enemy)
            for(const auto& Other:B->Before)
            {
                if(!Displayed(Visible,Other.Id) || Other.bEnemy==U.bEnemy || !Displayed(B->After,Other.Id)) continue;
                const bool Loss=B->After[Other.Id].Troops<Other.Troops || B->After[U.Id].Troops<B->Before[U.Id].Troops;
                if(Loss && FVector::DistSquared2D(U.Position,Other.Position)<250*250)
                { M.bAttack=true;M.Facing=(Other.Position-U.Position).GetSafeNormal2D();break; }
            }
        // Recoil starts when casualties become public, never at the wind-up.
        if(B->Kind!=EBattleCue::Stop && U.Loss>0 && P>=.48f) M.Recoil=Pulse(.48f,.55f,.66f,.86f,P);
        if(B->Kind==EBattleCue::Guard && U.Id==B->Actor) M.Brace=Ease(.08f,.40f,P);
        M.Action=M.March>.001f ? gunyuformation::EShape::Column : M.bAttack && !U.bDisordered ? gunyuformation::EShape::Wedge:M.Rest;
        M.FormationBlend=M.March>.001f ? M.March : M.bAttack ? Pulse(.12f,.32f,.74f,.96f,P):0;
        if(M.Facing.IsNearlyZero()) M.Facing=U.bEnemy ? -FVector::ForwardVector:FVector::ForwardVector;
        return M;
    }
    FVector FormationOffset(const FCompany& M,int Soldier,int Count)
    {
        const FVector Base=FMath::Lerp(gunyuformation::Offset(M.Before,Soldier,Count),gunyuformation::Offset(M.Rest,Soldier,Count),M.StateBlend);
        return FMath::Lerp(Base,gunyuformation::Offset(M.Action,Soldier,Count),M.FormationBlend);
    }
    FPose Pose(const FCompany& M,int Soldier,int UnitId,float Time)
    {
        FPose R;
        // Small rank delays keep a whole company from moving like one rigid object.
        const float P=M.Progress-FMath::Min(Soldier/8.f*.008f,.04f);
        const float Wind=M.bAttack ? Pulse(.14f,.30f,.33f,.46f,P):0;
        const float Thrust=M.bAttack ? Pulse(.32f,.48f,.58f,.84f,P):0;
        const float Hit=M.Recoil*(.65f+.35f*FMath::Abs(FMath::Sin(Soldier*2.1f)));
        const float Gait=Time*12+Soldier*.91f+UnitId*.7f;
        const float Idle=FMath::Sin(Time*2+Soldier*.73f+UnitId)*.3f;
        R.Root=FVector(Thrust*7-Wind*3-Hit*9,Hit*FMath::Sin(Soldier*1.3f)*2,FMath::Abs(FMath::Sin(Gait))*M.March*1.7f);
        R.Torso=FQuat::FindBetweenNormals(FVector::UpVector,FVector(.16f*Thrust-.09f*Wind-.23f*Hit+.04f*M.March,0,1).GetSafeNormal());
        auto Upper=[&](FVector V) { return FVector(0,0,27)+R.Torso.RotateVector(V-FVector(0,0,27)); };
        for(int I=0;I<2;++I)
        {
            const float Side=I==0 ? -1.f:1.f,Step=FMath::Sin(Gait+I*PI)*M.March,Lift=FMath::Max(0.f,Step);
            R.Hip[I]=FVector(0,Side*7,25);
            R.Knee[I]=FVector(Step*4+Lift*2+M.Brace*2,Side*7,14-M.Brace*2);
            R.Foot[I]=FVector(Step*9+Thrust*(I==1 ? 6:-3),Side*7,3+Lift*6);
            R.Shoulder[I]=Upper(FVector(0,Side*17,46));
        }
        R.Hand[0]=Upper(FVector(13+Thrust*7+M.Brace*6,-23,37+M.Brace*9+Hit*5));
        R.Hand[1]=Upper(FVector(7-Wind*8+Thrust*18,22,43+Wind*6-Thrust*5+M.Salute*13+Idle));
        for(int I=0;I<2;++I)
            R.Elbow[I]=(R.Shoulder[I]+R.Hand[I])*.5f+R.Torso.RotateVector(FVector(-6,(I==0 ? -1:1)*5,-6));
        const float Tilt=8+Wind*25+Thrust*78+M.Brace*40-M.Salute*18+Idle*2;
        R.Spear=FQuat::FindBetweenNormals(FVector::UpVector,FVector(FMath::Sin(FMath::DegreesToRadians(Tilt)),0,FMath::Cos(FMath::DegreesToRadians(Tilt))));
        R.SpearCenter=R.Hand[1]+R.Spear.RotateVector(FVector(0,0,6));
        R.Shield=R.Torso*FRotator(0,-12,Hit*12).Quaternion();
        R.ShieldCenter=R.Hand[0]+R.Shield.RotateVector(FVector(5,-2,1));
        return R;
    }
}
