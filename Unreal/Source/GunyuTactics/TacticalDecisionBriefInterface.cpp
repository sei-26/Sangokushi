#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
using namespace gunyuui;

TSharedRef<SWidget> ATacticalController::DecisionBriefWidget()
{
    auto PressedTakeover=MakeShared<FTacticalPresentation::FDecisionBrief>();
    auto PressedResume=MakeShared<FTacticalPresentation::FDecisionBrief>();
    auto Action=[this](bool Takeover,TSharedRef<FTacticalPresentation::FDecisionBrief> Pressed)
    {
        return SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(FMargin(7,6))
          .IsEnabled_Lambda([this] { return Field->Presentation().Decision.bVisible; })
          .OnPressed_Lambda([this,Pressed] { *Pressed=Field->Presentation().Decision; })
          .OnClicked_Lambda([this,Pressed,Takeover] { Field->UseDecisionBrief(*Pressed,Takeover);return FReply::Handled(); })
          [Label([this,Takeover]
          {
              if(!Takeover) return FString(TEXT("そのまま任せて続行 / Space"));
              const auto& H=Field->Presentation();const int A=H.Decision.Officer;
              return H.Officers.IsValidIndex(A) ? H.Officers[A].Name+TEXT("で、自分が指揮する"):FString(TEXT("ここから自分で指揮"));
          },13,Gold)];
    };
    return SNew(SVerticalBox)
      .Visibility_Lambda([this] { return Field->Presentation().Decision.bVisible ? EVisibility::Visible:EVisibility::Collapsed; })
      + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,5)[Label([this] { return Field->Presentation().Decision.Title; },18,Gold)]
      + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,9)[Label([this] { return Field->Presentation().Decision.Context; },13)]
      + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,6)[Label([this] { return Field->Presentation().Decision.KeepPlan; },12)]
      + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)[Label([this] { return Field->Presentation().Decision.Takeover; },12,Gold)]
      + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)[Label([this] { return Field->Presentation().Decision.Ready; },11)]
      + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,5)[Action(true,PressedTakeover)]
      + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,5)[Action(false,PressedResume)]
      + SVerticalBox::Slot().AutoHeight()[Button([] { return TEXT("変化した場所を見る / U"); },[this] { FocusMissionObjective(); })];
}

TSharedRef<SWidget> ATacticalController::AutoCommandWidget()
{
    auto Content=SNew(SVerticalBox);
    Content->AddSlot().AutoHeight()[DecisionBriefWidget()];
    auto Running=SNew(SVerticalBox).Visibility_Lambda([this] { return Field->Presentation().Decision.bVisible ? EVisibility::Collapsed:EVisibility::Visible; });
    Running->AddSlot().AutoHeight().Padding(0,0,0,7)[Label([] { return TEXT("武将たちに任せている"); },18,Gold)];
    Running->AddSlot().AutoHeight().Padding(0,0,0,10)[Label([this] { return Field->Presentation().ClockLabel; },13)];
    for(int A=0;A<3;++A)
        Running->AddSlot().AutoHeight().Padding(0,0,0,9)[SNew(SHorizontalBox)
          + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([A] { return A; },32,38)]
          + SHorizontalBox::Slot().FillWidth(1).Padding(8,0)[SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[Label([this,A] { const auto& O=Field->Presentation().Officers;return O.IsValidIndex(A) ? O[A].Name:FString(); },14,Gold)]
            + SVerticalBox::Slot().AutoHeight()[Label([this,A] { const auto& O=Field->Presentation().Officers;return O.IsValidIndex(A) ? (O[A].bLiving ? O[A].Command:FString(TEXT("戦闘不能"))):FString(); },12)]]];
    Running->AddSlot().AutoHeight().Padding(0,3,0,8)[Label([] { return TEXT("戦況が変われば、いつでも軍師の出番。指揮を引き取ると戦場は止まる。"); },12)];
    Running->AddSlot().AutoHeight().Padding(0,0,0,7)[Button([] { return TEXT("ここから自分で指揮する"); },[this] { Field->Issue(ETacticalCommand::AutoBattle); },[this] { return Field->CanIssue(ETacticalCommand::AutoBattle); },true)];
    Running->AddSlot().AutoHeight()[Button([] { return TEXT("今の目標を見る / U"); },[this] { FocusMissionObjective(); })];
    Content->AddSlot().AutoHeight()[Running];
    Content->AddSlot().AutoHeight().Padding(0,8,0,0)[SNew(SHorizontalBox)
      + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,5,0)
        [SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Decision.bVisible ? EVisibility::Collapsed:EVisibility::Visible; })
         [Button([this] { return Field->RealtimePaused() ? TEXT("再開する"):TEXT("一時停止"); },
           [this] { Field->Issue(ETacticalCommand::RealtimePause); },[this] { return Field->CanIssue(ETacticalCommand::RealtimePause); })]]
      + SHorizontalBox::Slot().FillWidth(1)[Button([this] { return TEXT("速さ ")+Field->Presentation().SpeedLabel; },
        [this] { Field->Issue(ETacticalCommand::RealtimeSpeed); },[this] { return Field->CanIssue(ETacticalCommand::RealtimeSpeed); })]];
    return Content;
}
