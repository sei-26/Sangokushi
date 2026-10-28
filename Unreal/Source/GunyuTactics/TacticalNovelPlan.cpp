#include "TacticalTrial.h"
#include "TacticalPlanningMap.h"
void ATacticalController::OpenNovelPlan()
{
    if(!bEpisodeOpen || bSaveMenuOpen || bNovelHistory || !Field->Presentation().Journey.bPreparation) return;
    bNovelPlan=true;bNovelAdvice=bNovelMap=false;bPlanningDuties=false;PlanningOfficer=0;bPlanningLocationChosen=false;PlanningNotice.Empty();bPlanningExample=false;
    const auto& Sites=Field->Presentation().Journey.Map.Sites;PlanningTile=Sites.IsEmpty() ? -1:Sites[0].Tile;
}
void ATacticalController::SelectNovelPlan(int Index)
{
    if(!bEpisodeOpen || !bNovelPlan || Index<0 || Index>1) return;
    const auto Command=Index==0 ? ETacticalCommand::PlanAmbush:ETacticalCommand::PlanSupplies;
    if(Field->CanIssue(Command)) Field->Issue(Command);
    if(PlanningWidget) PlanningWidget->Invalidate(EInvalidateWidgetReason::Paint);
}
void ATacticalController::DeployNovelPlan()
{
    if(!bEpisodeOpen || !bNovelPlan || !Field->CanIssue(ETacticalCommand::Deploy)) return;
    Field->Issue(ETacticalCommand::Deploy);
    if(Field->Presentation().bBattle) CloseEpisode();
}
