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
    if(Kind!=0 && !PlanningDutyReason(Kind).IsEmpty()) return;
    if(Field->AssignPlanningDuty(PlanningOfficer,PlanningTile,Kind))
    {
        const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
        PlanningNotice=FString(Kind==0 ? TEXT("担当を解除："):TEXT("命令を確定："))+Names[PlanningOfficer]+TEXT(" ／ ")+PlanningOrder(PlanningOfficer);
    }
    if(PlanningWidget) PlanningWidget->Invalidate(EInvalidateWidgetReason::Paint);
}
