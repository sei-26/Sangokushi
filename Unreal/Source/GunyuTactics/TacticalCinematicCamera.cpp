#include "TacticalTrial.h"
#include "TacticalSpectacle.h"
#include "TacticalActionFocus.h"
void ATacticalController::ApplyTacticCamera(FVector& ViewFocus,float& ViewRadius,float& ViewTilt) const
{
    if(!bTacticCinema || bSaveMenuOpen || bBattleLogOpen || bMarchPlanOpen || bTacticPickerOpen) return;
    const auto Read=Field->ReadTacticCinema();const auto& U=Field->VisualUnits();
    if(!Read.bVisible || Read.CameraWeight<=0 || !U.IsValidIndex(Read.Actor) || U[Read.Actor].Id<0) return;
    const auto Shot=gunyuspectacle::Read(Field->CurrentBeat(),U,Field->BeatProgress());if(!Shot.bValid) return;
    FVector Center=Shot.Center;float Separation=0;
    const auto FocusRead=Field->ReadActionFocus();
    if(FocusRead.bActive && FocusRead.bHasTarget) Center=FMath::Lerp(Center,FocusRead.Center,.90f);
    if(U.IsValidIndex(Read.Target) && U[Read.Target].Id>=0 && Field->CurrentBeat()->Kind!=EBattleCue::Rally)
        Separation=FVector::Dist2D(U[Read.Actor].Position,U[Read.Target].Position);
    const float Yaw=FMath::DegreesToRadians(CameraYaw);
    const FVector Right(FMath::Sin(Yaw),-FMath::Cos(Yaw),0);
    const float P=Field->BeatProgress();
    Center+=Right*(70+Shot.Sweep*FMath::Sin(P*PI));
    const float Weight=Shot.Weight*.96f;
    ViewFocus=FMath::Lerp(ViewFocus,Center,Weight);
    const float Close=FocusRead.bActive && FocusRead.bHasTarget ? FMath::Clamp(Separation*1.2f+620,920.f,1550.f)
        : FMath::Clamp(Separation*1.2f+720,Shot.CloseDistance,2200.f);
    ViewRadius=FMath::Lerp(ViewRadius,FMath::Min(ViewRadius,Close),Weight);
    ViewTilt=FMath::Lerp(ViewTilt,FocusRead.bActive && FocusRead.bHasTarget ? 38.f:Shot.Tilt,Weight*.8f);
    ViewFocus+=Right*Shot.Shake;
}
