#include "TacticalTrial.h"
#include "TacticalPlanningMap.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
using namespace gunyuui;

TSharedRef<SWidget> ATacticalController::BuildPlanningMap()
{
    auto MapColumn=SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[Label([this] { return Field->Presentation().Journey.Map.Name+TEXT("　／　戦場見取り図"); },22,Gold)]
        + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Label([this] { return Field->Presentation().Journey.Map.Objective+TEXT(" ／ 敵配置は未確認"); },14)]
        + SVerticalBox::Slot().FillHeight(1).Padding(0,6)
          [SAssignNew(PlanningWidget,SPlanningMap)
           .Map_Lambda([this] { return &Field->Presentation().Journey.Map; })
           .SelectedTile_Lambda([this] { return PlanningTile; })
           .OnTile_Lambda([this](int32 Tile) { SelectPlanningTile(Tile); })]
        + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("金：注目地点／護送路　青：出陣位置　朱：担当（青縁は出陣位置も兼ねる）"); },13,Gold)];
    MapColumn->AddSlot().AutoHeight().Padding(0,5)[Label([this]
    {
        const auto& Plans=Field->Presentation().Journey.Plans;
        const auto* Plan=Plans.FindByPredicate([](const FPreparationPresentation& P) { return P.bSelected; });
        return Plan ? FString::Printf(TEXT("選択中：%s　／　兵糧%d・資金%d"),*Plan->Title,Plan->Food,Plan->Gold):FString();
    },14,Gold)];
    MapColumn->AddSlot().AutoHeight().Padding(0,3)
        [SNew(SBox).Visibility_Lambda([this] { return bPlanningDuties ? EVisibility::Visible:EVisibility::Collapsed; })
         [Label([this]
         {
             return FString(Field->bRealtimeCommand ? TEXT("出陣後は停止。Spaceで行軍開始／Xで継続指示解除。"):TEXT("手動指揮：出陣後は自分で移動・固守・伏兵準備する。"))+
                 TEXT("\n配置成立＋任務達成＋生還で、本人の信頼＋2。");
         },13,Gold)]];
    auto Route=SNew(SHorizontalBox);
    Route->AddSlot().FillWidth(1)[Label([this] { return Field->Presentation().Journey.PlanRoute; },14)];
    Route->AddSlot().AutoWidth().Padding(8,0)[Button([this] { return Field->Presentation().Journey.Map.bSouthern ? TEXT("街道へ変更 / R"):TEXT("南の林道へ / R"); },[this] { TogglePlanningRoute(); })];
    MapColumn->AddSlot().AutoHeight().Padding(0,7,0,0)
        [SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Journey.Map.bChooseRoute ? EVisibility::Visible:EVisibility::Collapsed; })[Route]];
    auto Detail=SNew(SVerticalBox);
    Detail->AddSlot().AutoHeight()[Label([] { return TEXT("注目する場所"); },18,Gold)];
    // The number of authored sites is small. Slots update when the mission changes.
    for(int Index=0;Index<5;++Index)
        Detail->AddSlot().AutoHeight().Padding(0,3)
          [SNew(SBox).Visibility_Lambda([this,Index] { return Field->Presentation().Journey.Map.Sites.IsValidIndex(Index) ? EVisibility::Visible:EVisibility::Collapsed; })
           [Button([this,Index]
              { const auto& Sites=Field->Presentation().Journey.Map.Sites;return Sites.IsValidIndex(Index) ? FString::Printf(TEXT("%s %d　%s"),PlanningTile==Sites[Index].Tile ? TEXT("●"):TEXT("○"),Index+1,*Sites[Index].Name):FString(); },
              [this,Index] { const auto& Sites=Field->Presentation().Journey.Map.Sites;if(Sites.IsValidIndex(Index)) SelectPlanningTile(Sites[Index].Tile); },[] { return true; },true)]];
    Detail->AddSlot().AutoHeight().Padding(0,12,0,4)[Label([this] { return PlanningTitle(); },23,Gold)];
    Detail->AddSlot().AutoHeight()[Label([this] { return PlanningDescription(); },16)];
    Detail->AddSlot().AutoHeight().Padding(0,12,0,3)[Label([] { return TEXT("ここでの動きを考える"); },16,Gold)];
    const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
    for(int Officer=0;Officer<3;++Officer)
        Detail->AddSlot().AutoHeight().Padding(0,6)
          [SNew(SHorizontalBox)
           + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([Officer] { return Officer; },27,32)]
           + SHorizontalBox::Slot().FillWidth(1).Padding(8,0)
             [SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[Label([Name=FString(Names[Officer])] { return Name; },14,Gold)]
              + SVerticalBox::Slot().AutoHeight()[Label([this,Officer] { return PlanningHint(Officer); },15)]]];
    auto Side=SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,10)
          [Button([this] { return bPlanningDuties ? TEXT("地形と武将の見立てへ"):TEXT("担当と配置を決める"); },[this] { bPlanningDuties=!bPlanningDuties; })]
        + SVerticalBox::Slot().FillHeight(1)
          [SNew(SOverlay)
           + SOverlay::Slot()[SNew(SScrollBox).Visibility_Lambda([this] { return bPlanningDuties ? EVisibility::Collapsed:EVisibility::Visible; })+SScrollBox::Slot()[Detail]]
           + SOverlay::Slot()[SNew(SScrollBox).Visibility_Lambda([this] { return bPlanningDuties ? EVisibility::Visible:EVisibility::Collapsed; })+SScrollBox::Slot()[BuildPlanningDuties()]]];
    return SNew(SHorizontalBox).Visibility_Lambda([this] { return bNovelMap ? EVisibility::Visible:EVisibility::Collapsed; })
        + SHorizontalBox::Slot().FillWidth(1)[MapColumn]
        + SHorizontalBox::Slot().AutoWidth().Padding(20,0)[SNew(SBox).WidthOverride(320)
          [Side]];
}
