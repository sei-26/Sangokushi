#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

bool ATacticalField::SignalHasPendingMarch() const
{
    if (!State->ChainSignal.Armed()) return false;
    for (const auto& Order : State->Orders)
        if (Order.army>=0 && Order.army<3 && State->Realtime.orders[Order.army].kind==JourneyDirective::March) return true;
    return false;
}

void ATacticalField::SyncChainSignal()
{
    // Do not announce the resolved destination, loss or contact before playback.
    if (IsPlaying()) return;
    if (!bRealtimeCommand || State->Journey.stage!=JourneyStage::Battle)
    { State->ChainSignal.Cancel();State->SignalRead={};return; }
    State->SignalRead=State->ChainSignal.Inspect(State->Journey,State->Orders);
    if (State->SignalRead.state!=ChainSignalState::Invalid) return;
    State->Report=JourneyText(State->SignalRead.reason);
    State->ChainSignal.Cancel();State->Realtime.paused=true;
    State->bRealtimeChainRequested=false;
}
void ATacticalField::IssueChainSignal()
{
    if (State->ChainSignal.Armed())
    {
        State->ChainSignal.Cancel();State->SignalRead={};
        State->Report=TEXT("合図待ちを解除。戦法の予約と行軍指示は保持。Cで手動発動できる。");
    }
    else if (State->ChainSignal.Arm(State->Journey,State->Orders))
    {
        State->Realtime.paused=true;State->bRealtimeChainRequested=false;
        State->Report=TEXT("Spaceで再開。全手成立で一度だけ発動。担当隊の追加攻撃は温存し、行軍・交戦・護送は進む。見失いや予約変更で解除・停止。");
    }
    else State->Report=TEXT("合図を設定できません。各隊１手ずつ、生存部隊と捕捉中の敵を指定し、使用済みの固有戦法を外してください。");
    Refresh();
}
bool ATacticalField::DispatchReadySignal()
{
    if (IsPlaying() || SignalHasPendingMarch() || State->SignalRead.state!=ChainSignalState::Ready ||
        !State->ChainSignal.Release(State->Journey,State->Orders,!RealtimePaused())) return false;
    const auto Fired=State->Orders;
    State->SignalRead={};
    // A completed operation returns its participants to the commander rather
    // than immediately resuming a stale pursuit against the displaced target.
    for (const auto& Order : Fired) State->Realtime.Cancel(Order.army);
    DispatchQueuedChain();
    State->Report=TEXT("合図成立 / 全手が成立する見込みを確認して発動。 ")+State->Report;
    if (IsPlaying())
    {
        State->Beats[0].Title=TEXT("合図成立 / ")+State->Beats[0].Title;
        State->bRealtimePauseAfterPlayback=true;
    }
    else State->Realtime.paused=true;
    Refresh();return true;
}
