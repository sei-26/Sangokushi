#include "TacticalActionFocus.h"
#include "TacticalWeaponMotion.h"
#include "TacticalCinematicStyle.h"
#include "TacticalTrial.h"
#include "TacticalState.h"
namespace gunyufocus
{
    FRead Read(const TArray<FVisualUnit>& U,const FBattleBeat* B,float P,float Time,TFunctionRef<float(FVector)> Height)
    {
        FRead R;
        auto Present=[](const TArray<FVisualUnit>& A,int I) { return I>=0 && A.IsValidIndex(I) && A[I].Id==I; };
        if(!B || !gunyucinema::IsTactic(B->Kind) || B->Kind==EBattleCue::Stop || B->Actor<0 || B->Actor>=3 ||
           !Present(U,B->Actor) || !Present(B->Before,B->Actor) || U[B->Actor].bEnemy || U[B->Actor].Troops<=0) return R;
        const bool Support=B->Kind==EBattleCue::Rally;
        if(!Support && (!Present(U,B->Target) || !Present(B->Before,B->Target))) return R;
        R.Actor=B->Actor;const auto& Unit=U[R.Actor];
        const auto M=gunyumotion::Read(Unit,U,B,P);
        const auto E=gunyucompany::Equipment(Unit,3,gunyucompany::RepresentativeCount(Unit));
        const auto Pose=gunyuweapon::Pose(M,E,3,Unit.Id,Time);
        const FQuat Heading=M.Facing.Rotation().Quaternion();
        FVector Base=Unit.Position+Heading.RotateVector(gunyumotion::FormationOffset(M,3,gunyucompany::RepresentativeCount(Unit)));
        Base.Z=Height(Base);
        auto World=[&](FVector V) { V+=Pose.Root;V.Z*=M.Stature;return Base+Heading.RotateVector(V*E.VisualScale); };
        R.Hero=World(FVector(0,0,27)+Pose.Torso.RotateVector(FVector(0,0,25)));
        R.Blade=World(gunyuweapon::BladeTip(Pose,E));R.Center=R.Hero;
        R.Weight=FMath::SmoothStep(.06f,.28f,P)*(1-FMath::SmoothStep(.82f,.99f,P));
        R.bActive=R.Weight>.001f;
        R.Clearings.Add({R.Hero,37});R.Clearings.Add({FMath::Lerp(R.Hero,R.Blade,.78f),25});
        if(!Support)
        {
            R.Target=B->Target;R.bHasTarget=true;R.Victim=U[R.Target].Position+FVector(0,0,48);
            const float Bias=FMath::Lerp(.28f,.52f,FMath::SmoothStep(.36f,.62f,P));
            R.Center=FMath::Lerp(R.Hero,R.Victim,Bias);
            R.Clearings.Add({R.Victim,65});
        }
        return R;
    }
    float ClothOpacity(FVector Point,FVector Camera,const FRead& R)
    {
        if(!R.bActive) return 1;
        float Opacity=1;
        for(const auto& Subject:R.Clearings)
        {
            const FVector Ray=Subject.Center-Camera;const double Distance=Ray.Size();
            if(Distance<1) continue;
            const FVector Direction=Ray/Distance,ToPoint=Point-Camera;
            const double Along=FVector::DotProduct(ToPoint,Direction);
            if(Along<=0 || Along>=Distance-2) continue;
            const float Radius=Subject.Radius*Along/Distance;
            const float Side=(ToPoint-Direction*Along).Size();
            const float Clear=1-FMath::SmoothStep(Radius*.55f,Radius+8,Side);
            Opacity=FMath::Min(Opacity,1-.88f*R.Weight*Clear);
        }
        return FMath::Clamp(Opacity,.12f,1.f);
    }
}
gunyufocus::FRead ATacticalField::ReadActionFocus() const
{
    return gunyufocus::Read(VisualUnits(),CurrentBeat(),BeatProgress(),State->VisualTime,[this](FVector P) { return SurfaceHeight(P); });
}
void ATacticalField::SetActionView(FVector Camera)
{
    if(Camera.ContainsNaN()) return;
    const bool Changed=!State->bHasActionCamera || !Camera.Equals(State->ActionCamera,.05);
    State->ActionCamera=Camera;State->bHasActionCamera=true;
    if(Changed) RenderStandards();
}
