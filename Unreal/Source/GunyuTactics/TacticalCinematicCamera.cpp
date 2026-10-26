#include "TacticalTrial.h"
#include "TacticalSpectacle.h"
void ATacticalController::ApplyTacticCamera(FVector& ViewFocus,float& ViewRadius,float& ViewTilt) const
{
    if(!bTacticCinema || bSaveMenuOpen || bBattleLogOpen || bMarchPlanOpen || bTacticPickerOpen) return;
    const auto Read=Field->ReadTacticCinema();const auto& U=Field->VisualUnits();
    if(!Read.bVisible || Read.CameraWeight<=0 || !U.IsValidIndex(Read.Actor) || U[Read.Actor].Id<0) return;
    const auto Shot=gunyuspectacle::Read(Field->CurrentBeat(),U,Field->BeatProgress());if(!Shot.bValid) return;
    FVector Center=Shot.Center;float Separation=0;
    if(U.IsValidIndex(Read.Target) && U[Read.Target].Id>=0 && Field->CurrentBeat()->Kind!=EBattleCue::Rally)
        Separation=FVector::Dist2D(U[Read.Actor].Position,U[Read.Target].Position);
    const float Yaw=FMath::DegreesToRadians(CameraYaw);
    const FVector Right(FMath::Sin(Yaw),-FMath::Cos(Yaw),0);
    const float P=Field->BeatProgress();
    Center+=Right*(180+Shot.Sweep*FMath::Sin(P*PI));
    const float Weight=Shot.Weight*.82f;
    ViewFocus=FMath::Lerp(ViewFocus,Center,Weight);
    ViewRadius=FMath::Lerp(ViewRadius,FMath::Min(ViewRadius,FMath::Clamp(Separation*1.5f+900,Shot.CloseDistance,2600.f)),Weight);
    ViewTilt=FMath::Lerp(ViewTilt,Shot.Tilt,Weight*.8f);
    ViewFocus+=Right*Shot.Shake;
}
