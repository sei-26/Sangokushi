#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
using namespace gunyuui;
void ATacticalController::BuildEncounterInterface(TSharedRef<SCanvas> Canvas)
{
    auto Visible=[this] { const auto& J=Field->Presentation().Journey;return J.bVisible && !J.bPreparation && !J.bDebrief && !J.bComplete ? EVisibility::Visible:EVisibility::Collapsed; };
    auto Scene=SNew(SBox).Visibility_Lambda(Visible)[NovelBackdropWidget([] { return false; })];
    Canvas->AddSlot().Position(FVector2D::ZeroVector).Size(FVector2D(1600,900))[Scene];InputPanels.Add(Scene);
    for(int Slot=0;Slot<2;++Slot)
    {
        auto Figure=SNew(SBox).Visibility_Lambda(Visible)[NovelPortraitWidget([this,Slot] { return Slot==0 ? 3:Field->Presentation().Journey.Portrait; },Slot==0 ? 1020:650,Slot==0 ? 1530:975)];
        if(Slot==1) { Figure->SetRenderTransformPivot(FVector2D(.5,.5));Figure->SetRenderTransform(FSlateRenderTransform(FScale2D(-1,1))); }
        Canvas->AddSlot().Position(Slot==0 ? FVector2D(-30,-100):FVector2D(850,60)).Size(Slot==0 ? FVector2D(1020,1530):FVector2D(650,975))[Figure];
    }
    auto Intro=SNew(STacticalChrome).Padding(FMargin(22,12)).Opacity(.88f).Visibility_Lambda(Visible)
        [SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("一八四年  /  乱世への一歩"); },12,Gold)]
         + SVerticalBox::Slot().AutoHeight().Padding(0,5)[Label([this] { return Field->Presentation().Journey.Heading; },27)]];
    Canvas->AddSlot().Position(FVector2D(40,28)).Size(FVector2D(820,88))[Intro];InputPanels.Add(Intro);
    auto Dialogue=SNew(STacticalChrome).Padding(FMargin(34,20)).Accent(gunyustyle::Brass).Visibility_Lambda(Visible)
        [SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[Label([this] { return Field->Presentation().Journey.Speaker; },29,Gold)]
         + SVerticalBox::Slot().FillHeight(1).Padding(0,10)[Label([this] { return Field->Presentation().Journey.Dialogue; },23)]
         + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Label([] { return TEXT("この出会いから、あなたの采配が始まる。"); },13)]
            + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(330)
              [Button([this] { return Field->Presentation().Journey.PrimaryLabel; },[this] { Field->Issue(ETacticalCommand::Dialogue); },[this] { return Field->CanIssue(ETacticalCommand::Dialogue); })]]]];
    Canvas->AddSlot().Position(FVector2D(64,590)).Size(FVector2D(1472,244))[Dialogue];InputPanels.Add(Dialogue);
    auto Context=SNew(SBox).Visibility_Lambda(Visible)[Label([this] { return Field->Presentation().Journey.Context; },12)];
    Canvas->AddSlot().Position(FVector2D(80,854)).Size(FVector2D(1440,30))[Context];
}
