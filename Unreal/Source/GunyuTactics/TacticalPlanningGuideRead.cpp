#include "TacticalTrial.h"
#include "TacticalPlanningGuide.h"
#include "TacticalPlanningMap.h"
void ATacticalController::SelectPlanningOfficer(int Officer)
{
    if(!bEpisodeOpen || !bNovelPlan || !bNovelMap || Officer<0 || Officer>=3) return;
    PlanningOfficer=Officer;PlanningNotice.Empty();
    const auto& Duties=Field->Presentation().Journey.Map.Duties;
    const auto* Duty=Duties.FindByPredicate([Officer](const FPlanningDuty& D) { return D.Officer==Officer; });
    bPlanningLocationChosen=Duty && Duty->Kind!=0;
    PlanningTile=bPlanningLocationChosen ? Duty->Tile:-1;
    if(PlanningWidget) PlanningWidget->Invalidate(EInvalidateWidgetReason::Paint);
}
FString ATacticalController::PlanningDutyReason(int Kind) const
{
    if(!bPlanningLocationChosen) return TEXT("② 地図のマスを選んでください。");
    const auto Reason=Field->PlanningDutyProblem(PlanningOfficer,PlanningTile,Kind);
    if(!Reason.IsEmpty()) return Reason;
    return PlanningPreviewRoute().IsEmpty() ? FString(TEXT("地形上、到達できる経路がありません。")):FString();
}
FString ATacticalController::PlanningOrder(int Officer) const
{ return planningguide::Order(Field->Presentation().Journey.Map,Officer); }
TArray<int> ATacticalController::PlanningPreviewRoute() const
{ return bNovelMap && bPlanningDuties && !bPlanningExample && bPlanningLocationChosen ? planningguide::Route(Field->Presentation().Journey.Map,PlanningOfficer,PlanningTile):TArray<int>(); }
FString ATacticalController::PlanningExecution() const
{
    if(bPlanningExample) return Field->Presentation().Journey.ExampleReason.IsEmpty() ? TEXT("右の見本を確認し、「この見本で担当を配置する」で反映。"):TEXT("紫は未採用の案。右の提案を確認し、採用ボタンで担当へ反映。");
    if(!PlanningNotice.IsEmpty()) return PlanningNotice;
    if(!bPlanningLocationChosen)
    {
        const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
        return FString(Names[PlanningOfficer])+TEXT("を選択中。次は地図で目的地を選ぼう。");
    }
    const auto Path=PlanningPreviewRoute();
    if(Path.IsEmpty()) return TEXT("この地点への仮経路はありません。別の陸地を選びましょう。");
    return Path.Num()==1 ? TEXT("出陣位置と同じ場所。移動せず、その場で準備します。"):
        FString::Printf(TEXT("仮経路：%dマス → 到着後に準備（敵・味方で変化）"),Path.Num()-1);
}
