#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
using namespace gunyuui;
TSharedRef<SWidget> ATacticalController::BuildSupportChoicesInterface()
{
    auto Compact=[this] { return !Field->Presentation().BattleChoices.IsEmpty(); };
    auto Choices=SNew(SVerticalBox);
    Choices->AddSlot().AutoHeight().Padding(0,0,0,4)[Label([this,Compact]
      { return Compact() ? TEXT("援護へ回すなら"):TEXT("この隊を、どちらの援護へ？"); },13,Gold)];
    auto Row=SNew(SHorizontalBox);
    for(int Index=0;Index<2;++Index)
    {
        auto Read=[this,Index]() { const auto& H=Field->Presentation();return H.SupportChoices.IsValidIndex(Index) ? H.SupportChoices[Index]:FSupportChoicePresentation{}; };
        auto Pressed=MakeShared<int>(-1);
        auto Card=[this,Index,Read,Pressed](TSharedRef<SWidget> Content)
        {
            return SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(FMargin(7,5))
              .IsEnabled_Lambda([this,Read] { return !Field->IsPlaying() && Read().bEnabled; })
              .ToolTipText_Lambda([Read] { return FText::FromString(Read().Detail); })
              .OnPressed_Lambda([this,Pressed] { *Pressed=Field->Presentation().SupportVersion; })
              .OnClicked_Lambda([this,Index,Pressed] { Field->UseSupportChoice(Index,*Pressed);return FReply::Handled(); })[Content];
        };
        auto Full=SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Title; },16,Gold)]
          + SVerticalBox::Slot().AutoHeight().Padding(0,3)[Label([Read] { return Read().Outcome; },12)]
          + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Cost; },11)];
        Choices->AddSlot().AutoHeight().Padding(0,0,0,6)[SNew(SBox).HeightOverride(96)
          .Visibility_Lambda([Compact] { return Compact() ? EVisibility::Collapsed:EVisibility::Visible; })[Card(Full)]];
        Row->AddSlot().FillWidth(1).Padding(Index==0 ? FMargin(0,0,4,0):FMargin(0))[Card(Label([Read] { return Read().Title; },12,Gold))];
    }
    Choices->AddSlot().AutoHeight()[SNew(SBox).HeightOverride(28)
      .Visibility_Lambda([Compact] { return Compact() ? EVisibility::Visible:EVisibility::Collapsed; })[Row]];
    return SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().SupportChoices.IsEmpty() ? EVisibility::Collapsed:EVisibility::Visible; })[Choices];
}
