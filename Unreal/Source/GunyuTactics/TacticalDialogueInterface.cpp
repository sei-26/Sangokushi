#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
using namespace gunyuui;
TSharedRef<SWidget> ATacticalController::StoryConversationWidget()
{
    auto Lines=SNew(SVerticalBox);
    Lines->AddSlot().AutoHeight().Padding(0,0,0,8)[Label([this] { return Field->Presentation().Journey.StoryTitle; },19,Gold)];
    for(int Index=0;Index<10;++Index)
    {
        auto Read=[this,Index] { const auto& C=Field->Presentation().Journey.Conversation;return C.IsValidIndex(Index) ? C[Index]:FDialoguePresentation{}; };
        auto Face=PortraitWidget([Read] { return Read().Speaker<3 ? Read().Speaker:-1; },38,44);
        Face->SetVisibility(TAttribute<EVisibility>::CreateLambda([Read] { return Read().bProtagonist ? EVisibility::Collapsed:EVisibility::Visible; }));
        auto Hero=SNew(SBox).WidthOverride(40).HeightOverride(44).Visibility_Lambda([Read] { return Read().bProtagonist ? EVisibility::Visible:EVisibility::Collapsed; })
            [SNew(SBorder).Padding(4).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.045f,.16f,.19f))[Label([] { return TEXT("軍\n師"); },13,Gold)]];
        auto Bubble=SNew(SBorder).Padding(FMargin(12,7)).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor_Lambda([Read] { return Read().bProtagonist ? FLinearColor(.025f,.085f,.11f):FLinearColor(.026f,.035f,.04f); })
            [SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Name; },12,Gold)]
              + SVerticalBox::Slot().AutoHeight().Padding(0,2,0,0)[Label([Read] { return Read().Text; },17)]];
        Lines->AddSlot().AutoHeight().Padding(0,0,0,7)
            [SNew(SBox).MinDesiredHeight(54).Visibility_Lambda([this,Index] { return Field->Presentation().Journey.Conversation.IsValidIndex(Index) ? EVisibility::Visible:EVisibility::Collapsed; })
             [SNew(SHorizontalBox)
              + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)[Face]
              + SHorizontalBox::Slot().FillWidth(1).Padding(8,0)[Bubble]
              + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)[Hero]]];
    }
    Lines->AddSlot().AutoHeight()[SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Journey.Conversation.IsEmpty() ? EVisibility::Visible:EVisibility::Collapsed; })[Label([this] { return Field->Presentation().Journey.Dialogue; },18)]];
    return Lines;
}
