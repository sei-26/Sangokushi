#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SScrollBox.h"
using namespace gunyuui;
void ATacticalController::BuildBattleFeed(TSharedRef<SCanvas> Canvas)
{
    auto Feed=SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4,0,12,0)[Label([] { return TEXT("伝令"); },14,Gold)]
        + SHorizontalBox::Slot().FillWidth(1)[SNew(SScrollBox)+SScrollBox::Slot()[Label([this] { return Field->Presentation().Report; },13)]]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6,0)
          [Button([] { return TEXT("戦法記録 / Y"); },[this] { ToggleChainReview(); },[this] { return Field->HasChainReview(); },true)]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10,0,6,0)
          [Button([this] { return Field->Presentation().bAlertPause ? TEXT("警報停止 ON") : TEXT("警報停止 OFF"); },[this] { Field->Issue(ETacticalCommand::RealtimeAlertPause); },[this] { return Field->CanIssue(ETacticalCommand::RealtimeAlertPause); },true)]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
          [Button([this] { return FString::Printf(TEXT("履歴 %d / L"),Field->Presentation().BattleNotices.Num()); },[this] { ToggleBattleLog(); },[this] { return Field->bRealtimeCommand; },true)];
    AddBattlePanel(Canvas,FVector2D(16,718),FVector2D(1240,50),Feed,8);
}
