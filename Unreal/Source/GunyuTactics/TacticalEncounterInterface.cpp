#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
using namespace gunyuui;

void ATacticalController::BuildEncounterInterface(TSharedRef<SCanvas> Canvas)
{
    auto Visible=[this]
    { const auto& J=Field->Presentation().Journey;return J.bVisible && !J.bPreparation && !J.bDebrief && !J.bComplete ? EVisibility::Visible : EVisibility::Collapsed; };
    auto Intro=SNew(STacticalChrome).Padding(24).Opacity(.9f).Visibility_Lambda(Visible)
        [SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("一八四年    ―    乱世への一歩"); },14,Gold)]
         + SVerticalBox::Slot().AutoHeight().Padding(0,12)[Label([this] { return Field->Presentation().Journey.Heading; },38)]
         + SVerticalBox::Slot().AutoHeight()[Label([this] { return Field->Presentation().Journey.Outcome; },18,FLinearColor(.70f,.75f,.74f))]];
    Canvas->AddSlot().Position(FVector2D(80,174)).Size(FVector2D(960,226))[Intro];InputPanels.Add(Intro);
    auto Portrait=SNew(STacticalChrome).Padding(4).Accent(FLinearColor(.64f,.45f,.16f)).Visibility_Lambda(Visible)
        [PortraitWidget([this] { return Field->Presentation().Journey.Portrait; },392,414)];
    Canvas->AddSlot().Position(FVector2D(1120,138)).Size(FVector2D(400,422))[Portrait];InputPanels.Add(Portrait);
    auto Dialogue=SNew(STacticalChrome).Padding(24).Accent(FLinearColor(.64f,.45f,.16f)).Visibility_Lambda(Visible)
        [SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[Label([this] { return Field->Presentation().Journey.Speaker; },30,Gold)]
         + SVerticalBox::Slot().FillHeight(1).Padding(0,10)[Label([this] { return Field->Presentation().Journey.Dialogue; },24)]
         + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Label([] { return TEXT("この出会いから、あなたの采配が始まる。"); },13)]
            + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(330).HeightOverride(40)
              [Button([this] { return Field->Presentation().Journey.PrimaryLabel; },[this] { Field->Issue(ETacticalCommand::Dialogue); },[this] { return Field->CanIssue(ETacticalCommand::Dialogue); })]]]];
    Canvas->AddSlot().Position(FVector2D(80,554)).Size(FVector2D(1440,260))[Dialogue];InputPanels.Add(Dialogue);
    auto Context=SNew(STacticalChrome).Padding(8).Opacity(.85f).Visibility_Lambda(Visible)
        [Label([this] { return Field->Presentation().Journey.Context; },12)];
    Canvas->AddSlot().Position(FVector2D(80,832)).Size(FVector2D(1440,38))[Context];InputPanels.Add(Context);
}
