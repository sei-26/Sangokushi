#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
using namespace gunyuui;
void ATacticalController::BuildBattleHeader(TSharedRef<SCanvas> Canvas)
{
    auto Text=SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("群雄戦線    ―    軍師の一策"); },11,Gold)]
        + SVerticalBox::Slot().AutoHeight().Padding(0,3)[SNew(SBox).WidthOverride(1090)
          [Label([this] { return Field->Presentation().Title.Replace(TEXT("群雄戦線  /  "),TEXT("")); },23)]]
        + SVerticalBox::Slot().AutoHeight()[SNew(SBox).WidthOverride(1090)[Label([this] { return Field->Presentation().Objective; },15,Gold)]];
    auto Header=SNew(STacticalChrome).Padding(FMargin(16,9))[Text];
    InputPanels.Add(Header);Canvas->AddSlot().Position(FVector2D(16,12)).Size(FVector2D(1568,94))[Header];
    auto Retreat=Button([this] { return Field->Presentation().WithdrawLabel; },[this] { Field->Issue(ETacticalCommand::Withdraw); },[this] { return Field->CanIssue(ETacticalCommand::Withdraw); },true);
    auto RetreatBox=SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().bBattle ? EVisibility::Visible : EVisibility::Collapsed; })[Retreat];
    InputPanels.Add(RetreatBox);Canvas->AddSlot().Position(FVector2D(1290,32)).Size(FVector2D(282,40))[RetreatBox];
    AddBattlePanel(Canvas,FVector2D(1268,820),FVector2D(316,64),Button([this] { return Field->Presentation().NextLabel; },[this]
    {
        const auto& H=Field->Presentation();
        Field->Issue(H.bRealtime && H.bBattle ? ETacticalCommand::RealtimePause : H.bBusy ? ETacticalCommand::Skip : ETacticalCommand::EndTurn);
    }),8);
    auto Cue=SNew(STacticalChrome).Padding(12).Accent(FLinearColor(.65f,.43f,.12f))
        .Visibility_Lambda([this] { return Field->IsPlaying() && (!bTacticCinema || !Field->ReadTacticCinema().bVisible) ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
        [SNew(SHorizontalBox)
         + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([this] { return Field->CurrentBeat() ? Field->CurrentBeat()->Actor : -1; },70,78)]
         + SHorizontalBox::Slot().FillWidth(1).Padding(14,0)[SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[Label([this] { return Field->CueTitle(); },23,Gold)]
            + SVerticalBox::Slot().AutoHeight().Padding(0,5)[Label([this] { return Field->CueReason(); },14)]
            + SVerticalBox::Slot().FillHeight(1).VAlign(VAlign_Bottom)[SNew(SBox).HeightOverride(3)
              [SNew(SProgressBar).Style(&ProgressStyle()).Percent_Lambda([this] { return Field->BeatProgress(); }).FillColorAndOpacity(Gold)]]]];
    Canvas->AddSlot().Position(FVector2D(290,106)).Size(FVector2D(760,104))[Cue];
}
