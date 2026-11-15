#include "TacticalTrial.h"
#include "TacticalPlanningMap.h"
void ATacticalController::OpenNovelPlan()
{
    if(!bEpisodeOpen || bSaveMenuOpen || bNovelHistory || !Field->Presentation().Journey.bPreparation) return;
    bNovelPlan=true;bNovelAdvice=bNovelMap=bNovelDetails=false;bPlanningDuties=false;PlanningOfficer=0;bPlanningLocationChosen=false;PlanningNotice.Empty();bPlanningExample=false;
    bCouncilLedger=false;CouncilStageTime=0;for(float& Presence:CouncilPresence) Presence=0;OpenCouncilVoice(-1);
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
    int Officer=CouncilTopic;
    const auto& Map=Field->Presentation().Journey.Map;
    if(bNovelMap && !bPlanningDuties)
    {
        const int AtMarker=Map.CouncilRoles.IndexOfByPredicate(
            [this](const FPlanningRole& Briefing) { return Briefing.Tile==PlanningTile; });
        if(AtMarker>=0) Officer=AtMarker;
    }
    else if(bNovelMap && !bPlanningExample && bPlanningLocationChosen)
    {
        if(Map.Duties.IsValidIndex(PlanningOfficer) && Map.Duties[PlanningOfficer].Tile==PlanningTile)
            Officer=PlanningOfficer;
    }
    Field->Issue(ETacticalCommand::Deploy);
    if(Field->Presentation().bBattle)
    {
        CloseEpisode();
        if(Officer>=0) Field->PreferOpeningOfficer(Officer);
    }
}
