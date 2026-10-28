#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalPlanningMap.h"
using namespace frontline;
FString ATacticalField::PlanningDutyProblem(int Officer,int Tile,int Kind) const
{ return JourneyText(State->Journey.AssignmentProblem(Officer,Tile,JourneyDuty(Kind))); }
bool ATacticalField::AssignPlanningDuty(int Officer,int Tile,int Kind)
{
    if(IsPlaying() || !State->Journey.AssignDuty(Officer,Tile,JourneyDuty(Kind))) return false;
    State->Report=JourneyText(State->Journey.message);Refresh();return true;
}
void ATacticalController::SetPlanningDuty(int Kind)
{
    if(!bNovelMap || !bNovelPlan || !bEpisodeOpen || !bPlanningDuties) return;
    Field->AssignPlanningDuty(PlanningOfficer,PlanningTile,Kind);
    if(PlanningWidget) PlanningWidget->Invalidate(EInvalidateWidgetReason::Paint);
}
