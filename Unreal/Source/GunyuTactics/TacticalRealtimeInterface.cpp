#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
using namespace gunyuui;
void ATacticalController::BuildRealtimeInterface(TSharedRef<SCanvas> Canvas)
{
    auto Command=[this](ETacticalCommand Action,TFunction<FString()> Text)
    { return Button(Text,[this,Action] { Field->Issue(Action); },[this,Action] { return Field->CanIssue(Action); },true); };
    auto Content=SNew(SVerticalBox);
    Content->AddSlot().AutoHeight()[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Label([] { return TEXT("指揮時計"); },15,Gold)]
        + SHorizontalBox::Slot().AutoWidth()[Command(ETacticalCommand::RealtimeMode,[] { return TEXT("切替 F5"); })]];
    Content->AddSlot().AutoHeight().Padding(0,7)[Label([this] { return Field->bRealtimeCommand ? Field->Presentation().ClockLabel : TEXT("手動指揮 / Spaceで更新"); },12,Gold)];
    auto Live=SNew(SVerticalBox);
    Live->AddSlot().AutoHeight()[SNew(SBox).HeightOverride(4)[SNew(SProgressBar).Style(&ProgressStyle()).Percent_Lambda([this] { return Field->Presentation().ClockProgress; }).FillColorAndOpacity(FLinearColor(.23f,.65f,.61f))]];
    Live->AddSlot().AutoHeight().Padding(0,8)[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::RealtimeSpeed,[this] { return Field->Presentation().SpeedLabel; })]
        + SHorizontalBox::Slot().FillWidth(1).Padding(5,0,0,0)[Command(ETacticalCommand::RealtimeStep,[] { return TEXT("1周期"); })]];
    Live->AddSlot().AutoHeight()[Label([this] { return Field->Presentation().StandingOrderLabel; },12)];
    Live->AddSlot().AutoHeight().Padding(0,8)[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,5,0)[Button([] { return TEXT("経路 V"); },[this] { ToggleMarchPlan(); },[this] { return !Field->IsPlaying(); },true)]
        + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::RealtimeStop,[] { return TEXT("解除 X"); })]];
    Content->AddSlot().AutoHeight()[SNew(SBox).Visibility_Lambda([this] { return Field->bRealtimeCommand ? EVisibility::Visible : EVisibility::Collapsed; })[Live]];
    Content->AddSlot().AutoHeight().Padding(0,4)[Label([] { return TEXT("右クリック：行軍・攻撃\nShift＋右：経由地追加"); },11,FLinearColor(.55f,.62f,.62f))];
    auto Panel=SNew(STacticalChrome).Padding(12).Accent(FLinearColor(.21f,.48f,.43f))
        .Visibility_Lambda([this] { return Field->Presentation().bBattle && !(bTacticCinema && Field->ReadTacticCinema().Intro>.001f) ? EVisibility::Visible : EVisibility::Collapsed; })[Content];
    InputPanels.Add(Panel);Canvas->AddSlot().Position_Lambda([this] { return FVector2D(16,Field->bRealtimeCommand ? 486:586); }).Size_Lambda([this] { return FVector2D(230,Field->bRealtimeCommand ? 218:118); })[Panel];
    auto Signal=SNew(SVerticalBox);
    Signal->AddSlot().AutoHeight()[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Label([] { return TEXT("連鎖の合図"); },14,Gold)]
        + SHorizontalBox::Slot().AutoWidth()[Command(ETacticalCommand::RealtimeChainSignal,[this] { return Field->Presentation().bChainSignalArmed ? TEXT("解除 B") : TEXT("合図 B"); })]];
    Signal->AddSlot().AutoHeight().Padding(0,7)[Label([this] { return Field->Presentation().ChainSignalLabel; },12)];
    auto SignalPanel=SNew(STacticalChrome).Padding(12).Accent(FLinearColor(.6f,.43f,.13f))
        .Visibility_Lambda([this] { return Field->bRealtimeCommand && Field->Presentation().bBattle ? EVisibility::Visible : EVisibility::Collapsed; })[Signal];
    InputPanels.Add(SignalPanel);Canvas->AddSlot().Position(FVector2D(1268,704)).Size(FVector2D(316,104))[SignalPanel];
}
