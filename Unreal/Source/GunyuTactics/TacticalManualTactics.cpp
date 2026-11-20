#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalWidgets.h"
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
    return SNew(SBox).Visibility_Lambda([this] { return ShowManualTactics() ? EVisibility::Visible:EVisibility::Collapsed; })
      [SNew(STacticalChrome).Padding(8).Accent(FLinearColor(.92f,.62f,.20f))
        [SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[Label([this]
            { const auto& H=Field->Presentation();return (H.Officers.IsValidIndex(H.Selected) ? H.Officers[H.Selected].Name+TEXT("の"):FString())+TEXT("戦法・連鎖 ／ 一覧 F2"); },13,Gold)]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4)[TacticPickerButton()]
          + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,4)[Label([this]
            { const auto& T=Field->Presentation().Target;const int End=T.Find(TEXT("\n"));return End<0 ? T:T.Left(End); },11,Gold)]
          + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,4,0)[Command(ETacticalCommand::QueueTactic,[] { return TEXT("この戦法を予約"); })]
            + SHorizontalBox::Slot().FillWidth(1)[Button([] { return TEXT("使える敵を選ぶ"); },[this] { FocusNextTacticTarget(); },
                [this] { return !Field->IsPlaying() && Field->Presentation().ReadyTacticTargets>0; },true)]]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Label([this]
            { const auto& H=Field->Presentation();return H.bBusy ? FString(TEXT("行動の再生中。終わると次の戦法を選べる。")):H.TacticAvailability; },11)]
          + SVerticalBox::Slot().AutoHeight()[SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Orders.IsEmpty() ? EVisibility::Collapsed:EVisibility::Visible; })
            [Label([this] { FString Text;for(const auto& O:Field->Presentation().Orders) { if(!Text.IsEmpty()) Text+=TEXT(" → ");Text+=O.ActorName+TEXT("・")+O.TacticName; }return Text; },11,Gold)]]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(3).Padding(0,0,4,0)[Button([this]
              { return Field->Presentation().Orders.Num()>1 ? FString(TEXT("連鎖を今発動 / C")):FString(TEXT("単独を今発動 / C")); },
                [this] { Field->FireManualTactics(); },[this] { return Field->CanFireManualTactics(); },true)]
            + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::Clear,[] { return TEXT("取消"); })]]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)[SNew(SBox).Visibility_Lambda([this]
            { return Field->Presentation().bRealtime ? EVisibility::Visible:EVisibility::Collapsed; })
            .ToolTipText(FText::FromString(TEXT("通常の1周期を進めて行動を回復。行軍・交戦・兵糧も進む。固守を継続している隊は、解除してから進めよう。")))
            [SNew(SHorizontalBox)
              + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,4,0)[SNew(SBox).Visibility_Lambda([this]
                { return Field->Presentation().bGuardOrder ? EVisibility::Visible:EVisibility::Collapsed; })
                [Command(ETacticalCommand::ReleaseGuard,[] { return TEXT("固守を解除"); })]]
              + SHorizontalBox::Slot().FillWidth(2)[Command(ETacticalCommand::RealtimeStep,[] { return TEXT("1周期進める / ."); })]]]]];
}
