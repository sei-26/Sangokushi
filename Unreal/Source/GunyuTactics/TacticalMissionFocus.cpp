#include "TacticalTrial.h"
#include "StrategistJourney.hpp"
void ATacticalController::FocusMissionObjective()
{
    if(!Field || Field->IsPlaying()) return;
    const auto& H=Field->Presentation();
    if(!H.bBattle || H.Journey.bVisible || !frontline::StrategistJourney::OnMap(H.MissionFocusTile)) return;
    Focus=Field->SurfaceLocation(H.MissionFocusTile);Distance=2200;CameraPreset=-1;bOrbitDragging=false;
    IgnoreMapInputThroughFrame=Frames+1;
}
