#include "TacticalTrial.h"
void ATacticalController::ApplyTacticCamera(FVector& ViewFocus,float& ViewRadius,float& ViewTilt) const
{
    if(!bTacticCinema || bSaveMenuOpen || bBattleLogOpen || bMarchPlanOpen || bTacticPickerOpen) return;
    const auto Read=Field->ReadTacticCinema();const auto& U=Field->VisualUnits();
    if(!Read.bVisible || Read.CameraWeight<=0 || !U.IsValidIndex(Read.Actor) || U[Read.Actor].Id<0) return;
    FVector Center=U[Read.Actor].Position;float Separation=0;
    if(U.IsValidIndex(Read.Target) && U[Read.Target].Id>=0)
    { Separation=FVector::Dist2D(Center,U[Read.Target].Position);Center=(Center+U[Read.Target].Position)*.5; }
    const float Yaw=FMath::DegreesToRadians(CameraYaw);
    const FVector Right(FMath::Sin(Yaw),-FMath::Cos(Yaw),0);
    Center+=Right*180; // Leave breathing room before the command column.
    const float Weight=Read.CameraWeight*.82f;
    ViewFocus=FMath::Lerp(ViewFocus,Center,Weight);
    ViewRadius=FMath::Lerp(ViewRadius,FMath::Min(ViewRadius,FMath::Clamp(Separation*1.6f+1100,1700.f,2600.f)),Weight);
    ViewTilt=FMath::Lerp(ViewTilt,42.f,Weight*.8f);
    const float P=Field->BeatProgress();
    if(Read.bHit && P<.59f)
        ViewFocus+=Right*(FMath::Sin((P-.48f)*180)*4*(1-FMath::Clamp((P-.48f)/.11f,0.f,1.f)));
}
