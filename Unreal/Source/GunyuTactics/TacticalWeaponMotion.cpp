#include "TacticalWeaponMotion.h"
namespace gunyuweapon
{
    namespace
    {
        float Ease(float A,float B,float P) { return FMath::SmoothStep(A,B,P); }
        struct FKey { FVector Axis,Hand;float Turn; };
        FKey Blend(FKey A,FKey B,float T)
        { return {FMath::Lerp(A.Axis,B.Axis,T).GetSafeNormal(),FMath::Lerp(A.Hand,B.Hand,T),FMath::Lerp(A.Turn,B.Turn,T)}; }
        FKey Attack(gunyucompany::EWeapon Weapon,float P)
        {
            FKey Wind,Hit,Finish;
            if(Weapon==gunyucompany::EWeapon::Sword)
            {
                Wind={FVector(.10,-.65,.75),FVector(-2,21,67),-25};
                Hit={FVector(.97,.20,-.10),FVector(23,10,49),10};
                Finish={FVector(.25,.95,-.18),FVector(11,-10,39),32};
            }
            else if(Weapon==gunyucompany::EWeapon::Glaive)
            {
                Wind={FVector(-.05,-.90,.43),FVector(-6,22,56),-24};
                Hit={FVector(1,0,.06),FVector(14,10,50),0};
                Finish={FVector(.05,.98,.20),FVector(13,-2,48),30};
            }
            else
            {
                Wind={FVector(-.15,.12,.98),FVector(-9,21,48),-12};
                Hit={FVector(1,0,.015),FVector(29,8,48),0};
                Finish={FVector(.90,-.12,.38),FVector(9,20,49),8};
            }
            return P<.48f ? Blend(Wind,Hit,Ease(.28f,.48f,P)):Blend(Hit,Finish,Ease(.48f,.68f,P));
        }
    }
    gunyumotion::FPose Pose(const gunyumotion::FCompany& M,const gunyucompany::FEquipment& E,int Soldier,int UnitId,float Time)
    {
        auto P=gunyumotion::Pose(M,Soldier,UnitId,Time);
        if(!E.bCommander) return gunyucompany::EquippedPose(P,E);
        const float AttackWeight=M.bAttack ? Ease(.10f,.26f,M.Progress)*(1-Ease(.76f,.96f,M.Progress)):0;
        const float Weight=FMath::Max(AttackWeight,M.Salute);
        if(Weight<=0) return gunyucompany::EquippedPose(P,E);
        auto Key=Attack(E.Weapon,M.Progress);
        if(!M.bAttack) Key={FVector(.08,0,1),FVector(3,15,72),8};
        const FQuat OldTorso=P.Torso;
        P.Torso=FQuat::Slerp(OldTorso,FRotator(0,Key.Turn,0).Quaternion()*OldTorso,Weight).GetNormalized();
        const FVector Pivot(0,0,27);
        auto Reframe=[&](FVector V) { return Pivot+P.Torso.RotateVector(OldTorso.UnrotateVector(V-Pivot)); };
        auto Upper=[&](FVector V) { return Pivot+P.Torso.RotateVector(V-Pivot); };
        for(int I=0;I<2;++I) { P.Shoulder[I]=Reframe(P.Shoulder[I]);P.Hand[I]=Reframe(P.Hand[I]); }
        P.Hand[1]=FMath::Lerp(P.Hand[1],Upper(Key.Hand),Weight);
        const FVector Normal=E.Weapon==gunyucompany::EWeapon::Glaive && M.bAttack ? FVector::UpVector:FVector::RightVector;
        const FQuat Blade=FRotationMatrix::MakeFromZX(Key.Axis.GetSafeNormal(),Normal).ToQuat();
        P.Spear=FQuat::Slerp(P.Spear,Blade,Weight).GetNormalized();
        P.SpearCenter=P.Hand[1]+P.Spear.RotateVector(FVector(0,0,6));
        P.Shield=P.Torso*FRotator(0,-12,M.Recoil*12).Quaternion();
        P.ShieldCenter=P.Hand[0]+P.Shield.RotateVector(FVector(5,-2,1));
        // Step through the blow while keeping the rear foot nearly planted.
        const float Step=AttackWeight*Ease(.20f,.48f,M.Progress);
        const float Advance=Step*(E.Weapon==gunyucompany::EWeapon::Serpent ? 20.f:12.f);
        P.Root.X+=Advance;P.Foot[0].X-=Advance*.8f;P.Knee[0].X-=Advance*.35f;
        P.Foot[1].X+=Step*5;P.Knee[1].X+=Step*3;
        const float Lift=AttackWeight*Ease(.18f,.30f,M.Progress)*(1-Ease(.38f,.48f,M.Progress))*5;
        P.Foot[1].Z+=Lift;P.Knee[1].Z+=Lift*.35f;
        for(int I=0;I<2;++I)
            P.Elbow[I]=(P.Shoulder[I]+P.Hand[I])*.5f+P.Torso.RotateVector(FVector(-6,(I==0 ? -1:1)*5,-6));
        return gunyucompany::EquippedPose(P,E);
    }
    FVector BladeTip(const gunyumotion::FPose& P,const gunyucompany::FEquipment& E)
    {
        if(E.Weapon==gunyucompany::EWeapon::Sword) return P.Hand[1]+P.Spear.RotateVector(FVector(0,0,85*.62f));
        return P.SpearCenter+P.Spear.RotateVector(FVector(0,0,E.ShaftLength*.5f+(E.Weapon==gunyucompany::EWeapon::Spear ? 20:34)));
    }
    TArray<FRibbon> Ribbons(const TArray<FVisualUnit>& Units,const FBattleBeat* Beat,float Progress,float Time,TFunctionRef<float(FVector)> Height)
    {
        TArray<FRibbon> Out;
        if(Progress<.36f || Progress>=.78f || !IsAttack(Units,Beat,Progress)) return Out;
        const auto& U=Units[Beat->Actor];
        const int Count=gunyucompany::RepresentativeCount(U);
        const auto E=gunyucompany::Equipment(U,3,Count);
        auto Edge=[&](float Phase)
        {
            const auto M=gunyumotion::Read(U,Units,Beat,Phase);
            const auto P=Pose(M,E,3,U.Id,Time);
            const FQuat Heading=M.Facing.Rotation().Quaternion();
            FVector Base=U.Position+Heading.RotateVector(gunyumotion::FormationOffset(M,3,Count));Base.Z=Height(Base);
            auto World=[&](FVector V) { V+=P.Root;V.Z*=M.Stature;return Base+Heading.RotateVector(V*E.VisualScale); };
            const FVector Tip=BladeTip(P,E);
            const FVector Root=E.Weapon==gunyucompany::EWeapon::Sword ? P.Hand[1]+P.Spear.RotateVector(FVector(0,0,17*.62f))
                : P.SpearCenter+P.Spear.RotateVector(FVector(0,0,E.ShaftLength*.5f));
            return TPair<FVector,FVector>(World(Root),World(Tip));
        };
        const float Start=FMath::Max(.30f,Progress-.12f);
        auto Before=Edge(Start);
        const float Fade=Ease(.36f,.42f,Progress)*(1-Ease(.64f,.78f,Progress));
        auto Tint=gunyucompany::Livery(U)*.65f+FLinearColor(.28f,.35f,.30f,0);
        for(int I=0;I<8;++I)
        {
            const float T=(I+1)/8.f;
            const auto After=Edge(FMath::Lerp(Start,Progress,T));
            Tint.A=Fade*T*.60f;
            Out.Add({Before.Key,Before.Value,After.Value,After.Key,Tint});Before=After;
        }
        return Out;
    }
    bool IsAttack(const TArray<FVisualUnit>& Units,const FBattleBeat* Beat,float Progress)
    {
        if(!Beat || Beat->Actor<0 || Beat->Actor>=3 || !Units.IsValidIndex(Beat->Actor)) return false;
        const auto& U=Units[Beat->Actor];
        return U.Id==Beat->Actor && !U.bEnemy && U.Troops>0 && gunyumotion::Read(U,Units,Beat,Progress).bAttack;
    }
}
