#include "TacticalTrial.h"
#include "TacticalPlanningMap.h"
void ATacticalController::ApplyPlanningExample()
{
    if(!bEpisodeOpen || !bNovelPlan || !bNovelMap || !bPlanningExample || !Field->CanIssue(ETacticalCommand::PlanExample)) return;
    Field->Issue(ETacticalCommand::PlanExample);bPlanningExample=false;bPlanningDuties=true;
    SelectPlanningOfficer(0);
    PlanningNotice=TEXT("見本の担当を配置しました。出陣後の手順は、右上の「作戦の見本」で再確認できます。");
    if(PlanningWidget) PlanningWidget->Invalidate(EInvalidateWidgetReason::Paint);
}
