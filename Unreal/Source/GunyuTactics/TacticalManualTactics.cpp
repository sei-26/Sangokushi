#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalWidgets.h"
#include "TacticalMissionTarget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
using namespace gunyuui;

bool ATacticalField::CanFireManualTactics() const
{
    return State->Journey.stage==frontline::JourneyStage::Battle && !IsPlaying() && !State->bAutoBattle &&
        !State->Orders.empty() && !State->ChainSignal.Armed() && !State->bRealtimeChainRequested && !State->bRealtimeStepping;
}
bool ATacticalField::FireManualTactics()
{
    if(!CanFireManualTactics()) return false;
    const bool paused=bRealtimeCommand && State->Realtime.paused;
    if(paused) { State->bRealtimeStepping=true;State->bRealtimePauseAfterPlayback=true; }
    DispatchQueuedChain();
    if(paused && !IsPlaying()) { State->bRealtimeStepping=State->bRealtimePauseAfterPlayback=false;State->Realtime.paused=true; }
    Refresh();return true;
}

bool ATacticalController::ShowManualTactics() const
{ return Field && Field->Presentation().bBattle && !Field->IsAutoBattle() && !bDetailedBattle; }

void ATacticalController::ToggleTacticPicker()
{
    if(!Field || !Field->Presentation().bBattle || Field->IsPlaying()) return;
    if(!bTacticPickerOpen)
    {
        Field->DismissGrandMoment();
        Field->StopAutoBattle();
        if(Field->bRealtimeCommand && !Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
        bInterfaceVisible=true;
    }
    bTacticPickerOpen=!bTacticPickerOpen;IgnoreMapInputThroughFrame=Frames+1;
}

TSharedRef<SWidget> ATacticalController::ManualTacticsWidget()
{
    auto Command=[this](ETacticalCommand C,TFunction<FString()> Name)
    { return Button(Name,[this,C] { Field->Issue(C); },[this,C] { return Field->CanIssue(C); },true); };
    auto PressedTarget=MakeShared<FTacticTargetPresentation>();
    auto ChooseEnemy=SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(FMargin(4,3))
      .IsEnabled_Lambda([this] { return !Field->IsPlaying() && !Field->IsAutoBattle() && Field->Presentation().bTacticTargetOverlay &&
          (gunyucommand::MissionEnemy(Field->Presentation()) || Field->Presentation().ReadyTacticTargets>0); })
      .ToolTipText_Lambda([this]
      { return FText::FromString(gunyucommand::MissionEnemy(Field->Presentation()) ?
          TEXT("役目に関わる捕捉中の敵を標的に選ぶ。停止して予測を確認し、戦法は自分の合図で発動。"):
          TEXT("選択中の戦法が届く捕捉中の敵へ切り替える。選択だけでは発動しない。")); })
      .OnPressed_Lambda([this,PressedTarget]
      { const auto* T=gunyucommand::MissionEnemy(Field->Presentation());*PressedTarget=T ? *T:FTacticTargetPresentation{}; })
      .OnClicked_Lambda([this,PressedTarget]
      {
          if(PressedTarget->Army>=0)
          {
              if(gunyucommand::SelectMissionEnemy(*Field,PressedTarget->Army,PressedTarget->Tile)) FocusMissionObjective();
          }
          else FocusNextTacticTarget();
          IgnoreMapInputThroughFrame=Frames+1;return FReply::Handled();
      })
      [Label([this] { return gunyucommand::MissionEnemy(Field->Presentation()) ? TEXT("役目の敵へ"):TEXT("届く敵へ"); },12,Gold)];
    return SNew(SBox).Visibility_Lambda([this] { return ShowManualTactics() ? EVisibility::Visible:EVisibility::Collapsed; })
      [SNew(STacticalChrome).Padding(8).Accent(FLinearColor(.92f,.62f,.20f))
        .ToolTipText_Lambda([this] { return FText::FromString(Field->Presentation().TacticAvailability); })
        [SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([this] { return Field->Presentation().Selected; },28,34)]
            + SHorizontalBox::Slot().FillWidth(1).Padding(8,0)[SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[Label([this]
                { const auto& H=Field->Presentation();return H.Officers.IsValidIndex(H.Selected) ? H.Officers[H.Selected].Name:FString(); },17,Gold)]
              + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("戦法・連鎖 ／ 一覧 F2"); },11)]]]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4)[TacticPickerButton()]
          + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,4)[Label([this]
            { const auto& T=Field->Presentation().Target;const int End=T.Find(TEXT("\n"));return End<0 ? T:T.Left(End); },11,Gold)]
          + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,4,0)[Command(ETacticalCommand::QueueTactic,[] { return TEXT("この戦法を予約"); })]
            + SHorizontalBox::Slot().FillWidth(1)[ChooseEnemy]]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4)[SNew(SBox)
            .Visibility_Lambda([this]
            {
                const auto& H=Field->Presentation();
                const bool Lure=H.Tactics.ContainsByPredicate([](const FTacticOptionPresentation& T)
                    { return T.bSelected && T.Id==int(frontline::ChainTactic::Lure); });
                return H.bBusy || H.bTacticUnavailable || Lure ? EVisibility::Visible:EVisibility::Collapsed;
            })
            [Label([this]
              { const auto& H=Field->Presentation();return H.bBusy ? FString(TEXT("行動の再生中。終わると次の戦法を選べる。")):H.TacticAvailability; },11)]]
          + SVerticalBox::Slot().AutoHeight()[SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Orders.IsEmpty() ? EVisibility::Collapsed:EVisibility::Visible; })
            [Label([this] { FString Text;for(const auto& O:Field->Presentation().Orders) { if(!Text.IsEmpty()) Text+=TEXT(" → ");Text+=O.ActorName+TEXT("・")+O.TacticName; }return Text; },11,Gold)]]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)[SNew(SBox)
            .Visibility_Lambda([this] { const auto& H=Field->Presentation();return !H.Orders.IsEmpty() || H.bChainSignalArmed ? EVisibility::Visible:EVisibility::Collapsed; })
            [SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(3).Padding(0,0,4,0)[Button([this]
              { return Field->Presentation().Orders.Num()>1 ? FString(TEXT("連鎖を今発動 / C")):FString(TEXT("単独を今発動 / C")); },
                [this] { Field->FireManualTactics(); },[this] { return Field->CanFireManualTactics(); },true)]
            + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::Clear,[] { return TEXT("取消"); })]]]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)[SNew(SBox).Visibility_Lambda([this]
            { return Field->Presentation().bRealtime ? EVisibility::Visible:EVisibility::Collapsed; })
            .ToolTipText(FText::FromString(TEXT("通常の1周期を進めて行動を回復。行軍・交戦・兵糧も進む。固守を継続している隊は、解除してから進めよう。")))
            [SNew(SHorizontalBox)
              + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,4,0)[SNew(SBox).Visibility_Lambda([this]
                { return Field->Presentation().bGuardOrder ? EVisibility::Visible:EVisibility::Collapsed; })
                [Command(ETacticalCommand::ReleaseGuard,[] { return TEXT("固守解除"); })]]
              + SHorizontalBox::Slot().FillWidth(2)[Command(ETacticalCommand::RealtimeStep,[] { return TEXT("1周期進める / ."); })]]]]];
}
