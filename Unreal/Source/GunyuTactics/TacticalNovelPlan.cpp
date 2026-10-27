#include "TacticalTrial.h"
void ATacticalController::OpenNovelPlan()
{
    if(!bEpisodeOpen || bSaveMenuOpen || bNovelHistory || !Field->Presentation().Journey.bPreparation) return;
    bNovelPlan=true;
}
void ATacticalController::SelectNovelPlan(int Index)
{
    if(!bEpisodeOpen || !bNovelPlan || Index<0 || Index>1) return;
    const auto Command=Index==0 ? ETacticalCommand::PlanAmbush:ETacticalCommand::PlanSupplies;
    if(Field->CanIssue(Command)) Field->Issue(Command);
}
void ATacticalController::DeployNovelPlan()
{
    if(!bEpisodeOpen || !bNovelPlan || !Field->CanIssue(ETacticalCommand::Deploy)) return;
    Field->Issue(ETacticalCommand::Deploy);
    if(Field->Presentation().bBattle) CloseEpisode();
}
