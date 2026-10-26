#include "TacticalTrial.h"
#include "TacticalSpectacle.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
void ATacticalController::UpdateTacticLens()
{
    gunyuspectacle::FShot Shot;
    if(bTacticCinema && !bSaveMenuOpen && !bMarchPlanOpen && !bBattleLogOpen && !bTacticPickerOpen && !Field->Presentation().Journey.bVisible)
        Shot=gunyuspectacle::Read(Field->CurrentBeat(),Field->VisualUnits(),Field->BeatProgress());
    auto& Lens=BattleCamera->GetCameraComponent()->PostProcessSettings;
    Lens.BloomIntensity=.35f+Shot.Pulse*Shot.Bloom;
    Lens.VignetteIntensity=.22f+Shot.Weight*.08f;
    Lens.AutoExposureBias=.25f-Shot.Weight*.035f+Shot.Pulse*.055f;
}
