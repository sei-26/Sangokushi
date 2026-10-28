#include "TacticalTrial.h"
#include "TacticalPlanningMap.h"
#include "CampaignTypes.hpp"

namespace
{
    const FPlanningSite* SiteAt(const FPreparationMap& Map,int Tile)
    { return Map.Sites.FindByPredicate([Tile](const FPlanningSite& Site) { return Site.Tile==Tile; }); }
}
void ATacticalController::TogglePlanningMap()
{
    if(!bEpisodeOpen || !bNovelPlan || !Field->Presentation().Journey.bPreparation) return;
    bNovelMap=!bNovelMap;bNovelAdvice=false;
    if(bNovelMap) { bPlanningDuties=true;bPlanningExample=false; }
    const auto& Map=Field->Presentation().Journey.Map;
    if(!Map.Cells.ContainsByPredicate([this](const FPlanningCell& Cell) { return Cell.Tile==PlanningTile; }))
        PlanningTile=Map.Sites.IsEmpty() ? -1:Map.Sites[0].Tile;
    if(PlanningWidget) PlanningWidget->Invalidate(EInvalidateWidgetReason::Paint);
}
void ATacticalController::SelectPlanningTile(int Tile)
{
    if(!bNovelMap || !bNovelPlan || !bEpisodeOpen) return;
    if(Field->Presentation().Journey.Map.Cells.ContainsByPredicate([Tile](const FPlanningCell& Cell) { return Cell.Tile==Tile; }))
    { PlanningTile=Tile;bPlanningLocationChosen=true;PlanningNotice.Empty(); }
    if(PlanningWidget) PlanningWidget->Invalidate(EInvalidateWidgetReason::Paint);
}
void ATacticalController::TogglePlanningRoute()
{
    if(!bNovelMap || !bEpisodeOpen || !Field->Presentation().Journey.Map.bChooseRoute) return;
    const auto Command=Field->Presentation().Journey.Map.bSouthern ? ETacticalCommand::RouteRoad:ETacticalCommand::RouteSouthernWood;
    if(Field->CanIssue(Command)) Field->Issue(Command);
    if(PlanningWidget) PlanningWidget->Invalidate(EInvalidateWidgetReason::Paint);
}
FString ATacticalController::PlanningTitle() const
{
    const auto& Map=Field->Presentation().Journey.Map;
    if(const auto* Site=SiteAt(Map,PlanningTile)) return Site->Name;
    const auto* Cell=Map.Cells.FindByPredicate([this](const FPlanningCell& C) { return C.Tile==PlanningTile; });
    if(!Cell) return TEXT("地図のマスを選ぶ");
    using frontline::Terrain;
    return Cell->Terrain==uint8(Terrain::Forest) ? TEXT("森林"):Cell->Terrain==uint8(Terrain::Mountain) ? TEXT("山地"):
        Cell->Terrain==uint8(Terrain::River) ? TEXT("川"):Cell->Terrain==uint8(Terrain::Bridge) ? TEXT("橋"):TEXT("平地");
}
FString ATacticalController::PlanningDescription() const
{
    const auto& Map=Field->Presentation().Journey.Map;
    if(const auto* Site=SiteAt(Map,PlanningTile)) return Site->Description;
    const auto* Cell=Map.Cells.FindByPredicate([this](const FPlanningCell& C) { return C.Tile==PlanningTile; });
    if(!Cell) return TEXT("番号の地点や地形を選び、進路と役目を考えよう。");
    using frontline::Terrain;
    return Cell->Terrain==uint8(Terrain::Forest) ? TEXT("伏兵を準備できる地形。森は遠くへの見通しも遮るため、先の様子を確かめたい。"):
        Cell->Terrain==uint8(Terrain::Mountain) ? TEXT("山は見通しを遮る。向こう側が見えなくても、敵がいないとは限らない。"):
        Cell->Terrain==uint8(Terrain::River) ? TEXT("部隊も車も進入できない。渡れる橋と、渡った後に留まる場所を確認しよう。"):
        TEXT("移動と連動のための空間。車の通る道や、仲間が退く場所を塞がないように使おう。");
}
FString ATacticalController::PlanningHint(int Officer) const
{
    const auto& P=Field->Presentation().Journey;
    if(const auto* Site=SiteAt(P.Map,PlanningTile)) return Site->Hints.IsValidIndex(Officer) ? Site->Hints[Officer]:FString();
    return P.PlanAdvice.IsValidIndex(Officer) ? P.PlanAdvice[Officer].Text:FString();
}
