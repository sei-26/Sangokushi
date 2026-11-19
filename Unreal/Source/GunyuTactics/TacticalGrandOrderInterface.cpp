#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
using namespace gunyuui;

TSharedRef<SWidget> ATacticalController::BuildGrandOrderInterface()
{
    auto Choices=SNew(SVerticalBox);
    Choices->AddSlot().AutoHeight().Padding(0,0,0,6)[Label([] { return TEXT("軍師、本隊をどちらへ？"); },18,Gold)];
    const TCHAR* Titles[]={TEXT("北の仲間を救う"),TEXT("中央を一気に突破する")};
    const TCHAR* Reasons[]={TEXT("本隊は北の林道へ。中央の支援は後になる。"),TEXT("本隊が兵糧と士気を使い強行軍。左翼は自力で耐える。")};
    for(int Index=0;Index<2;++Index)
    {
        const FString Title=Titles[Index],Reason=Reasons[Index];
        Choices->AddSlot().AutoHeight().Padding(0,0,0,7)
          [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(FMargin(8,7))
            .IsEnabled_Lambda([this] { return Field->CanUseGrandApproach(); })
            .OnClicked_Lambda([this,Index] { Field->UseGrandApproach(Index);return FReply::Handled(); })
            [SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[Label([Title] { return Title; },16,Gold)]
              + SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)[Label([Reason] { return Reason; },12)]]];
    }
    return SNew(SBox).Visibility_Lambda([this]
        { return !Field->Presentation().GrandOrderStatus.IsEmpty() && !Field->IsPlaying() ? EVisibility::Visible:EVisibility::Collapsed; })
      [SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[SNew(SBox).Visibility_Lambda([this]
            { return Field->Presentation().bGrandOrdersPending ? EVisibility::Visible:EVisibility::Collapsed; })[Choices]]
        + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,4)[Label([this]
            { return TEXT("青い矢印：本隊の進軍予定"); },12,Gold)]];
}
