#include "TacticalTrial.h"
#include "TacticalPlanningMap.h"
void ATacticalController::ApplyPlanningExample()
{
    if(!bEpisodeOpen || !bNovelPlan || !bNovelMap || !bPlanningExample || !Field->CanIssue(ETacticalCommand::PlanExample)) return;
    Field->Issue(ETacticalCommand::PlanExample);bPlanningExample=false;bPlanningDuties=true;
    SelectPlanningOfficer(0);
    PlanningNotice=Field->Presentation().Journey.ExampleReason.IsEmpty() ? TEXT("見本の担当を配置しました。右上で手順を再確認できます。"):TEXT("提案を採用。担当を配置しました。右上で狙いと代償を再確認でき、各武将の担当は自由に編集できます。");
    if(PlanningWidget) PlanningWidget->Invalidate(EInvalidateWidgetReason::Paint);
}

void ATacticalController::OpenPlanningExample()
{
    if(!bEpisodeOpen || !bNovelPlan || !Field->Presentation().Journey.bPreparation) return;
    if(!bNovelMap) TogglePlanningMap();
    bPlanningExample=true;bPlanningDuties=true;bNovelAdvice=false;
    if(PlanningWidget) PlanningWidget->Invalidate(EInvalidateWidgetReason::Paint);
}
