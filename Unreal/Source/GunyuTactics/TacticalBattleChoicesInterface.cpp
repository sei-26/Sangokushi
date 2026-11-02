#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
using namespace gunyuui;
TSharedRef<SWidget> ATacticalController::BuildBattleChoicesInterface()
{
    auto Choices=SNew(SVerticalBox);
    Choices->AddSlot().AutoHeight().Padding(0,0,0,6)[Label([this]
        { const auto& H=Field->Presentation();return H.BattleChoiceTarget.IsEmpty() ? TEXT("次の一手"):H.BattleChoiceTarget+TEXT("に、どう打つ？"); },14,Gold)];
    for(int Index=0;Index<2;++Index)
    {
        auto Read=[this,Index]() { const auto& H=Field->Presentation();return H.BattleChoices.IsValidIndex(Index) ? H.BattleChoices[Index]:FTacticalPresentation::FBattleChoice{}; };
        auto PressedVersion=MakeShared<int>(-1);
        auto Card=SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(FMargin(8,6))
          .Visibility_Lambda([this,Index] { return Field->Presentation().BattleChoices.IsValidIndex(Index) ? EVisibility::Visible:EVisibility::Collapsed; })
          .IsEnabled_Lambda([this,Index] { return !Field->IsPlaying() && !Field->IsAutoBattle() && Field->Presentation().BattleChoices.IsValidIndex(Index); })
          .ToolTipText_Lambda([Read] { return FText::FromString(Read().Detail); })
          .OnPressed_Lambda([this,PressedVersion] { *PressedVersion=Field->Presentation().BattleChoiceVersion; })
          .OnClicked_Lambda([this,Index,PressedVersion] { Field->UseBattleChoice(Index,*PressedVersion);return FReply::Handled(); })
          [SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[PortraitWidget([Read] { return Read().Actor; },34,44)]
            + SHorizontalBox::Slot().FillWidth(1).Padding(9,0)[SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Title; },16,Gold)]
              + SVerticalBox::Slot().AutoHeight().Padding(0,3)[Label([Read] { return Read().Outcome; },12)]
              + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Cost; },11)]]];
        Choices->AddSlot().AutoHeight().Padding(0,0,0,7)[SNew(SBox).HeightOverride(112)
          .Visibility_Lambda([this,Index] { return Field->Presentation().BattleChoices.IsValidIndex(Index) ? EVisibility::Visible:EVisibility::Collapsed; })[Card]];
    }
    Choices->AddSlot().AutoHeight()[SNew(SBox)
      .Visibility_Lambda([this] { return Field->Presentation().BattleChoices.IsEmpty() ? EVisibility::Visible:EVisibility::Collapsed; })
      [Label([this] { return Field->Presentation().BattleChoiceReason; },15)]];
    return SNew(SBox).HeightOverride_Lambda([this] { const int Count=Field->Presentation().BattleChoices.Num();return FOptionalSize(Count>0 ? 26+Count*119:90); })[Choices];
}
