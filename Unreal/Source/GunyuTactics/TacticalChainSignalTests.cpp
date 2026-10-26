#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

bool ATacticalField::ChainSignalSmokeTest()
{
    const bool SavedMode=bRealtimeCommand,SavedCouncil=bStartWithCouncil,SavedPlayback=State->bNoPlayback;
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    auto Check=[](bool Value,const TCHAR* Reason)
    { if (!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_CHAIN_SIGNAL: FAIL: %s"),Reason);return Value; };
    bRealtimeCommand=true;bStartWithCouncil=false;FirstMission=0;EnemyPolicy=1;State->bNoPlayback=true;
    auto WaitingFormation=[this]
    {
        Issue(ETacticalCommand::Restart);
        const int Positions[]={Campaign::At(9,9),Campaign::At(10,8),Campaign::At(7,9),Campaign::At(9,8)};
        for (int Id=0;Id<4;++Id) State->Journey.world.armies[Id].tile=State->Journey.world.armies[Id].target=Positions[Id];
        State->Journey.world.armies[4].troops=0;
        State->Journey.chain.BeginTurn(State->Journey.world);State->Journey.RefreshIntelligence();Refresh();
        SelectOfficer(0);SelectTactic(static_cast<int>(ChainTactic::Feint));Issue(ETacticalCommand::QueueTactic);
        SelectOfficer(1);SelectTactic(static_cast<int>(ChainTactic::Disrupt));Issue(ETacticalCommand::QueueTactic);
        SelectOfficer(2);SelectTactic(static_cast<int>(ChainTactic::Breakthrough));Issue(ETacticalCommand::QueueTactic);
    };
    WaitingFormation();
    CommandAt(Campaign::At(8,9));
    const int Enemy=State->Journey.world.armies[3].troops,Revision=State->Journey.world.revision;
    Action(EKeys::B);AdvanceRealtime(30);
    if (!Check(State->ChainSignal.Armed() && State->SignalRead.completed==2 && RealtimePaused() && State->Journey.world.revision==Revision,
        TEXT("arming an incomplete sequence is non-consuming and starts paused"))) return false;
    Action(EKeys::SpaceBar);AdvanceRealtime(1);
    if (!Check(State->Journey.world.armies[3].troops==Enemy && State->Journey.lastChain.completed==0,TEXT("a feasible prefix must never fire automatically"))) return false;
    AdvanceRealtime(11);
    if (!Check(State->Journey.turn==2 && State->Journey.world.armies[2].tile==Campaign::At(8,9) && State->SignalRead.state==ChainSignalState::Ready,
        TEXT("real persistent march completes the setup while the battlefield advances"))) return false;
    AdvanceRealtime(.1f);
    if (!Check(State->Journey.lastChain.completed==3 && !State->ChainSignal.Armed() && RealtimePaused() && State->Orders.empty(),
        TEXT("the whole sequence fires once when ready and returns control paused"))) return false;
    const int After=State->Journey.world.armies[3].troops;AdvanceRealtime(50);
    if (!Check(State->Journey.world.armies[3].troops==After && State->Realtime.orders[2].kind==JourneyDirective::None,
        TEXT("no repeated firing or stale pursuit after the operation"))) return false;

    WaitingFormation();Action(EKeys::B);EditOrder(1,-1);
    if (!Check(!State->ChainSignal.Armed() && State->Orders.size()==3 && RealtimePaused(),TEXT("changing sequence cancels authorization without discarding reservations"))) return false;
    WaitingFormation();Action(EKeys::B);
    for (auto& Contact : State->Journey.intelligence[0].contacts) if (Contact.id==3) Contact.current=false;
    Refresh();
    if (!Check(!State->ChainSignal.Armed() && RealtimePaused() && State->Orders.size()==3 && State->Report.Contains(TEXT("見失った")),
        TEXT("lost target cancels and explains the pause without hidden tracking"))) return false;

    EnemyPolicy=0;Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);Action(EKeys::B);
    State->bNoPlayback=false;Action(EKeys::SpaceBar);AdvanceRealtime(.1f);
    if (!Check(IsPlaying() && !State->ChainSignal.Armed() && CueTitle().Contains(TEXT("合図成立")),TEXT("automatic operation uses real per-step chain playback"))) return false;
    AdvanceRealtime(50);
    if (!Check(IsPlaying(),TEXT("no new command pulse can interrupt the chain presentation"))) return false;
    AdvancePresentation(30);
    if (!Check(!IsPlaying() && RealtimePaused(),TEXT("finished signal playback pauses before the next world update"))) return false;

    State->bNoPlayback=true;Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);Action(EKeys::B);Action(EKeys::C);
    if (!Check(!State->ChainSignal.Armed() && State->bRealtimeChainRequested,TEXT("manual fire explicitly overrides the signal"))) return false;
    Action(EKeys::SpaceBar);AdvanceRealtime(.1f);
    if (!Check(State->Journey.lastChain.completed==3,TEXT("manual override still resolves the original chain"))) return false;
    Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);Action(EKeys::B);Issue(ETacticalCommand::RealtimeMode);
    if (!Check(!bRealtimeCommand && !State->ChainSignal.Armed() && State->Orders.size()==3,TEXT("manual mode clears signal but retains tactics"))) return false;
    bRealtimeCommand=SavedMode;bStartWithCouncil=SavedCouncil;FirstMission=SavedMission;EnemyPolicy=SavedPolicy;
    State->bNoPlayback=SavedPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_CHAIN_SIGNAL: PASS (observed full-chain trigger, real approach, one shot, cancellation, pause, playback, manual override)"));
    return true;
}
