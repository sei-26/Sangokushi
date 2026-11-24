#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "JourneyEpisode.hpp"
#include "JourneyMission.hpp"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
using namespace gunyuui;

TSharedRef<SWidget> ATacticalController::BuildGrandOrderInterface()
{
    auto Choices=SNew(SVerticalBox);
    Choices->AddSlot().AutoHeight().Padding(0,0,0,6)[Label([this] { return Field->Presentation().Mission==5 ? TEXT("孫堅と、どう戦う？"):TEXT("軍師、本隊をどちらへ？"); },18,Gold)];
    for(int Index=0;Index<2;++Index)
    {
        auto Read=[this,Index] { return Field->Presentation().GrandChoices.IsValidIndex(Index) ? Field->Presentation().GrandChoices[Index]:FTacticalPresentation::FGrandChoice{}; };
        Choices->AddSlot().AutoHeight().Padding(0,0,0,7)
          [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(FMargin(8,7))
            .IsEnabled_Lambda([this] { return Field->CanUseGrandApproach(); })
            .OnClicked_Lambda([this,Index] { Field->UseGrandApproach(Index);return FReply::Handled(); })
            [SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Title; },16,Gold)]
              + SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)[Label([Read] { return Read().Reason; },12)]]];
    }
    return SNew(SBox).Visibility_Lambda([this]
        { return !Field->Presentation().GrandOrderStatus.IsEmpty() && !Field->IsPlaying() ? EVisibility::Visible:EVisibility::Collapsed; })
      [SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,7)[SNew(SBox).Visibility_Lambda([this]
            { return Field->Presentation().bYangrenCounter ? EVisibility::Visible:EVisibility::Collapsed; })
          [SNew(STacticalChrome).Padding(8).Accent(FLinearColor(.9f,.24f,.10f))
            [SNew(SHorizontalBox)
              + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(42).HeightOverride(54)
                [SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
                  [SNew(SImage).Image_Lambda([this] { return HistoricalPortraitBrush(frontline::LuBu); })]]]
              + SHorizontalBox::Slot().FillWidth(1).Padding(8,0)[SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("呂布の反撃！"); },18,Gold)]
                + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("騎兵を崩せ。孫堅を一人にするな。"); },13)]]]]]
        + SVerticalBox::Slot().AutoHeight()[SNew(SBox).Visibility_Lambda([this]
            { return Field->Presentation().bGrandOrdersPending ? EVisibility::Visible:EVisibility::Collapsed; })[Choices]]
        + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,4)[Label([this]
            { const auto& H=Field->Presentation();return H.GrandRouteLegend.IsEmpty() ?
                FString(H.GrandAdvanceRoute.Num()>1 ? TEXT("青い矢印：本隊の進軍予定"):TEXT("本隊は待機・交戦中")):H.GrandRouteLegend; },12,Gold)]
        + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,4)[SNew(SBox).Visibility_Lambda([this]
            { const auto& H=Field->Presentation();return H.Mission==frontline::CoalitionMission && H.bGrandBattle ? EVisibility::Visible:EVisibility::Collapsed; })
            [Label([this] { return Field->Presentation().GrandOrderStatus; },12,Gold)]]];
}
