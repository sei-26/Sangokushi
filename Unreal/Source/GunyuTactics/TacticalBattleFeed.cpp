#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "TacticalCommandOverlayLayout.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SScrollBox.h"
using namespace gunyuui;
void ATacticalController::BuildBattleFeed(TSharedRef<SCanvas> Canvas)
{
    auto Feed=SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4,0,12,0)[Label([] { return TEXT("伝令"); },14,Gold)]
        + SHorizontalBox::Slot().FillWidth(1)
          [SNew(SVerticalBox)
           + SVerticalBox::Slot().FillHeight(1)[SNew(SScrollBox)+SScrollBox::Slot()[Label([this] { return Field->Presentation().Report; },12)]]
           + SVerticalBox::Slot().AutoHeight().Padding(0,3,0,0)
             [Label([this] { return Field->IsPlaying() ? FString(TEXT("行動を再生中。完了後に次の一手へ。")):Field->Presentation().CommandGuide; },12,Gold)]]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6,0)
          [Button([] { return TEXT("戦法記録 / Y"); },[this] { ToggleChainReview(); },[this] { return Field->HasChainReview(); },true)]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10,0,6,0)
          [Button([this] { return Field->Presentation().bAlertPause ? TEXT("節目で停止 ON") : TEXT("節目で停止 OFF"); },[this] { Field->Issue(ETacticalCommand::RealtimeAlertPause); },[this] { return Field->CanIssue(ETacticalCommand::RealtimeAlertPause); },true)]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
          [Button([this] { return FString::Printf(TEXT("履歴 %d / L"),Field->Presentation().BattleNotices.Num()); },[this] { ToggleBattleLog(); },[this] { return Field->bRealtimeCommand; },true)];
    AddBattlePanel(Canvas,FVector2D(16,gunyucommandlayout::FeedTop),FVector2D(gunyucommandlayout::FeedWidth,gunyucommandlayout::FeedHeight),Feed,8);
}
