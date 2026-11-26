#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"

using namespace gunyuui;

TSharedRef<SWidget> ATacticalController::OpeningOrderCard()
{
    auto Pressed=MakeShared<FOpeningOrderPresentation>();
    return SNew(SBox).Visibility_Lambda([this] { return ShowOpeningOrder() ? EVisibility::Visible:EVisibility::Collapsed; })
        [SNew(STacticalChrome).Padding(9).Accent(FLinearColor(.34f,.71f,.60f))
         [SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()
            [SNew(SHorizontalBox)
             + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
               [Label([this] { return TEXT("軍議の一手 ／ ")+Field->Presentation().OpeningOrder.Name; },13,FLinearColor(.68f,.94f,.86f))]
             + SHorizontalBox::Slot().AutoWidth()
               [Button([] { return TEXT("閉じる"); },[this] { Field->DismissOpeningOrder(); },[] { return true; },true)]]
          + SVerticalBox::Slot().AutoHeight().Padding(0,3)
            [Label([this] { return Field->Presentation().OpeningOrder.Place; },17,Gold)]
          + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,5)
            [Label([this] { return Field->Presentation().OpeningOrder.Instruction; },14)]
          + SVerticalBox::Slot().AutoHeight()
            [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(FMargin(7,5))
             .IsEnabled_Lambda([this] { return !Field->IsPlaying(); })
             .OnPressed_Lambda([this,Pressed] { *Pressed=Field->Presentation().OpeningOrder; })
             .OnClicked_Lambda([this,Pressed]
              { if(Pressed->Action==EOpeningOrderAction::Solo) Field->UseFirstStrike(Pressed->SoloVersion,Pressed->SoloTarget);else OpenFirstCommand();return FReply::Handled(); })
             [Label([this] { return Field->Presentation().OpeningOrder.ActionLabel; },14,Gold)]]]];
}
