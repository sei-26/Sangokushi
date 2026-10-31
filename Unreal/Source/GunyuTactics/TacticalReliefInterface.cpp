#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
using namespace gunyuui;
TSharedRef<SWidget> ATacticalController::ReliefChoicesWidget()
{
    auto Choices=SNew(SHorizontalBox);
    for(int I=0;I<2;++I)
        Choices->AddSlot().FillWidth(1).Padding(I==0 ? FMargin(0,0,4,0):FMargin(0))
          [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(4)
           .Visibility_Lambda([this,I] { return Field->Presentation().Position.Relief.Officers.IsValidIndex(I) ? EVisibility::Visible:EVisibility::Collapsed; })
           .ButtonColorAndOpacity_Lambda([this,I] { const auto& R=Field->Presentation().Position.Relief;return FSlateColor(R.Officers.IsValidIndex(I) && R.Incoming==R.Officers[I].Army ? FLinearColor(1.3f,2.4f,1.8f):FLinearColor::White); })
           .ToolTipText_Lambda([this,I] { const auto& O=Field->Presentation().Position.Relief.Officers;return FText::FromString(O.IsValidIndex(I) ? O[I].Detail:FString()); })
           .OnClicked_Lambda([this,I] { const auto& O=Field->Presentation().Position.Relief.Officers;if(O.IsValidIndex(I)) Field->SelectReliefOfficer(O[I].Army);IgnoreMapInputThroughFrame=Frames+1;return FReply::Handled(); })
           [SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[Label([this,I] { const auto& O=Field->Presentation().Position.Relief.Officers;return O.IsValidIndex(I) ? O[I].Name:FString(); },12,Gold)]
            + SVerticalBox::Slot().AutoHeight()[Label([this,I] { const auto& O=Field->Presentation().Position.Relief.Officers;return O.IsValidIndex(I) ? O[I].Status:FString(); },10)]]];
    return SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Position.Relief.bVisible ? EVisibility::Visible:EVisibility::Collapsed; })
      [SNew(SVerticalBox)
       + SVerticalBox::Slot().AutoHeight()[Label([this] { const auto& H=Field->Presentation();const int Source=H.Position.Relief.Source;return (Source==H.GoalTile ? H.GoalLabel:Source==H.SecondGoalTile ? H.SecondGoalLabel:FString(TEXT("守備地点")))+TEXT("の後任 / 緑の破線＝移動案"); },11,FLinearColor(.38f,.94f,.68f))]
       + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Choices]];
}
bool ATacticalController::ReliefInterfaceSmokeTest()
{
    Field->PrepareReliefPreview(0);const auto Before=Field->VisualUnits();
    const auto& R=Field->Presentation().Position.Relief;
    bool Passed=R.bVisible && R.bCanIssue && R.Incoming==0 && R.Source>=0;
    Field->SelectReliefOfficer(1);
    Passed=Passed && Field->Presentation().Selected==2 && Field->Presentation().Position.Relief.Incoming==1 && !Field->Presentation().Position.Relief.bCanIssue;
    Field->SelectReliefOfficer(0);
    Passed=Passed && Field->VisualUnits()[0].Position.Equals(Before[0].Position) && Field->VisualUnits()[2].Position.Equals(Before[2].Position);
    Field->Issue(ETacticalCommand::Cancel);Passed=Passed && !Field->Presentation().Position.Relief.bVisible;
    if(Passed) { UE_LOG(LogTemp,Display,TEXT("GUNYU_RELIEF_UI: PASS (explicit successor selection, unchanged outgoing officer and visible units, cancellation)")); }
    else { UE_LOG(LogTemp,Error,TEXT("GUNYU_RELIEF_UI: FAIL")); }
    return Passed;
}
