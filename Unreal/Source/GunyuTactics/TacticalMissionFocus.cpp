#include "TacticalTrial.h"
#include "StrategistJourney.hpp"
int ATacticalController::SelectedDutyTile() const
{
    if(!Field || Field->IsPlaying()) return -1;
    const auto& H=Field->Presentation();
    if(!H.bBattle || H.Journey.bVisible || !H.Officers.IsValidIndex(H.Selected)) return -1;
    const auto& Officer=H.Officers[H.Selected];
    return Officer.bLiving && frontline::StrategistJourney::OnMap(Officer.DutyTile) ? Officer.DutyTile:-1;
}

void ATacticalController::FocusSelectedDuty()
{
    const int Tile=SelectedDutyTile();
    if(Tile<0) return;
    Focus=Field->SurfaceLocation(Tile);Distance=2200;CameraPreset=-1;bOrbitDragging=false;
    IgnoreMapInputThroughFrame=Frames+1;LabelLayoutFrame=MAX_uint64;
}

void ATacticalController::FocusMissionObjective()
{
    if(!Field || Field->IsPlaying()) return;
    const auto& H=Field->Presentation();
    if(!H.bBattle || H.Journey.bVisible || !frontline::StrategistJourney::OnMap(H.MissionFocusTile)) return;
    Focus=Field->SurfaceLocation(H.MissionFocusTile);Distance=2200;CameraPreset=-1;bOrbitDragging=false;
    IgnoreMapInputThroughFrame=Frames+1;
}
