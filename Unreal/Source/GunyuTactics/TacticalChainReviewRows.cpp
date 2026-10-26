#include "TacticalTrial.h"
#include "TacticalChainReview.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
using namespace gunyuui;
TSharedRef<SWidget> ATacticalController::ChainReviewRow(int Index)
{
    auto Read=[this,Index] { return ChainReviewSnapshot.IsValid() && ChainReviewSnapshot->Rows.IsValidIndex(Index) ? ChainReviewSnapshot->Rows[Index]:FChainReviewRow{}; };
    auto Text=SNew(SHorizontalBox)
      + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,12,0)[Label([Index] { return FString::FromInt(Index+1); },30,Gold)]
      + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,12,0)[PortraitWidget([Read] { return Read().Actor; },48,62)]
      + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,16,0)
        [SNew(SBox).WidthOverride(178)[SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Officer; },18,Gold)]
         + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Tactic; },21)]
         + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Label([Read] { return Read().Role; },12)]]]
      + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,18,0)
        [SNew(SBox).WidthOverride(236)[SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Bold",18)).AutoWrapText(true)
           .ColorAndOpacity_Lambda([Read] { return FSlateColor(Read().bSuccess ? FLinearColor(.4f,.9f,.73f):FLinearColor(1,.48f,.23f)); })
           .Text_Lambda([Read] { return FText::FromString(Read().Outcome); })]
         + SVerticalBox::Slot().AutoHeight().Padding(0,6)[Label([Read] { return TEXT("対象：")+Read().Subject; },13)]]]
      + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
        [SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Link; },16,Gold)]
         + SVerticalBox::Slot().AutoHeight().Padding(0,6)[Label([Read] { return Read().Reason; },13)]];
    return SNew(SBox).MinDesiredHeight(112)
      .Visibility_Lambda([this,Index] { return ChainReviewSnapshot.IsValid() && ChainReviewSnapshot->Rows.IsValidIndex(Index) ? EVisibility::Visible:EVisibility::Collapsed; })
      [SNew(STacticalChrome).Padding(14).Accent(FLinearColor(.46f,.39f,.22f))[Text]];
}
