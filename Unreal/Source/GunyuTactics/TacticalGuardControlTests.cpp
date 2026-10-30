#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
using namespace frontline;

void ATacticalField::PrepareGuardControlPreview(bool bSpent)
{
    bStartWithCouncil=true;FirstMission=2;EnemyPolicy=1;bRealtimeCommand=true;Issue(ETacticalCommand::Restart);
    for(int I=0;I<3;++I) Issue(ETacticalCommand::Dialogue);
    Issue(ETacticalCommand::PlanSupplies);Issue(ETacticalCommand::Deploy);SelectOfficer(2);
    State->Realtime.Queue(State->Journey,2,JourneyDirective::Hold);
    if(bSpent) State->Journey.Hold(2);
    Refresh();
}
bool ATacticalField::GuardControlSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_GUARD_CONTROL: FAIL %s"),Why);return V; };
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedRealtime=bRealtimeCommand,SavedLegacy=bLegacyJourney;
    auto Start=[this](bool Realtime)
    {
        bLegacyJourney=false;bStartWithCouncil=true;FirstMission=0;EnemyPolicy=1;bRealtimeCommand=Realtime;
        Issue(ETacticalCommand::Restart);for(int I=0;I<3;++I) Issue(ETacticalCommand::Dialogue);
        Issue(ETacticalCommand::PromiseRescue);Issue(ETacticalCommand::PlanSupplies);Issue(ETacticalCommand::Deploy);SelectOfficer(0);
    };
    auto Bytes=[this]() { std::vector<std::uint8_t> B;ExportSession(B);return B; };
    // A release changes only the selected standing order and the pause state.
    Start(true);SelectTactic(int(ChainTactic::Feint));
    State->Realtime.Queue(State->Journey,0,JourneyDirective::Hold);
    State->Realtime.Queue(State->Journey,1,JourneyDirective::Ambush);
    State->Orders={{0,3,ChainTactic::Feint,-1}};
    if(!Check(State->ChainSignal.Arm(State->Journey,State->Orders),TEXT("a valid already reserved tactic can wait for its signal"))) return false;
    State->Realtime.paused=false;State->Realtime.Advance(2);Refresh();
    const auto Before=Bytes();auto Expected=std::make_unique<JourneySaveData>();
    if(!Check(DecodeJourneySave(Before,*Expected) && CanIssue(ETacticalCommand::ReleaseGuard) && Presentation().bGuardOrder,TEXT("guard order exposes explicit release"))) return false;
    Expected->realtime.Cancel(0);Expected->realtime.paused=true;
    std::vector<std::uint8_t> ExpectedBytes;EncodeJourneySave(*Expected,ExpectedBytes);
    Issue(ETacticalCommand::ReleaseGuard);
    if(!Check(Bytes()==ExpectedBytes && RealtimePaused() && !Presentation().bGuardOrder && State->ChainSignal.Armed(),TEXT("release is a pure order/pause change: world, finance, actions, reservations, signal and time match exactly"))) return false;
    const auto Stable=Bytes();Refresh();
    if(!Check(Bytes()==Stable && !CanIssue(ETacticalCommand::ReleaseGuard),TEXT("reading and repeated release cannot change the settled state"))) return false;
    if(!Check(ImportSession(Stable) && State->Realtime.orders[0].kind==JourneyDirective::None && State->Realtime.orders[1].kind==JourneyDirective::Ambush,TEXT("load preserves the release without clearing other officers' orders"))) return false;
    // Arrival guard is also explicit: choosing this release abandons the whole march.
    Start(true);
    State->Realtime.Queue(State->Journey,0,JourneyDirective::March,Campaign::At(10,12));
    State->Realtime.AppendWaypoint(State->Journey,0,Campaign::At(12,12));
    State->Realtime.SetArrival(State->Journey,0,JourneyArrival::Hold);Refresh();
    if(!Check(Presentation().GuardButtonLabel.Contains(TEXT("行軍")) && Presentation().GuardContext.Contains(TEXT("行軍も停止")),TEXT("arrival-guard release discloses the march cancellation"))) return false;
    const int MarchTurn=State->Journey.turn,MarchTile=State->Journey.world.armies[0].tile;
    Issue(ETacticalCommand::ReleaseGuard);
    if(!Check(State->Realtime.orders[0].kind==JourneyDirective::None && State->Realtime.orders[0].waypoints.empty() && State->Journey.turn==MarchTurn && State->Journey.world.armies[0].tile==MarchTile,TEXT("march release stops rather than teleporting or advancing"))) return false;
    // A real paid guard cannot be turned into a free attack in the same command.
    Start(true);State->Journey.Hold(0);State->Journey.chain.signatureUsed[0]=true;
    State->Realtime.Queue(State->Journey,0,JourneyDirective::Hold);SelectTactic(int(ChainTactic::Feint));Refresh();
    if(!Check(Presentation().GuardContext.Contains(TEXT("行動済み")) && !CanIssue(ETacticalCommand::QueueTactic),TEXT("actual guard explains the spent action"))) return false;
    Issue(ETacticalCommand::ReleaseGuard);
    if(!Check(State->Journey.guarding[0] && State->Journey.chain.spent[0] && State->Journey.chain.moved[0] && State->Journey.chain.signatureUsed[0] && !CanIssue(ETacticalCommand::QueueTactic),TEXT("release never refunds guard, movement or a signature"))) return false;
    if(!Check(!Presentation().TacticAvailability.Contains(TEXT("継続を解除")) && Presentation().TacticAvailability.Contains(TEXT("1周期")),TEXT("an already released guard points to recovery rather than asking for another release"))) return false;
    const auto Released=Bytes();
    if(!Check(ImportSession(Released) && State->Journey.guarding[0] && State->Journey.chain.spent[0] && State->Realtime.orders[0].kind==JourneyDirective::None,TEXT("saving a spent guard cannot launder its action"))) return false;
    const int GuardTurn=State->Journey.turn;
    State->Realtime.Queue(State->Journey,0,JourneyDirective::Hold);Refresh();
    Issue(ETacticalCommand::RealtimeStep);
    if(!Check(IsPlaying() && !CanIssue(ETacticalCommand::ReleaseGuard),TEXT("release is blocked while a command is being replayed"))) return false;
    Issue(ETacticalCommand::ReleaseGuard);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.turn==GuardTurn+1 && State->Realtime.orders[0].kind==JourneyDirective::Hold && !State->Journey.chain.spent[0] && State->Journey.chain.signatureUsed[0],TEXT("only a real cycle restores the ordinary action; queued guard and used signature survive"))) return false;
    Issue(ETacticalCommand::ReleaseGuard);SelectTactic(int(ChainTactic::Oath));
    if(!Check(!CanIssue(ETacticalCommand::QueueTactic) && Presentation().TacticAvailability.Contains(TEXT("使用済み")),TEXT("a released guard does not restore a signature"))) return false;
    SelectTactic(int(ChainTactic::Feint));
    if(!Check(CanIssue(ETacticalCommand::QueueTactic),TEXT("ordinary tactic is available after the real cycle and explicit guard release"))) return false;
    const int Attempts=State->Journey.battleReview.tacticAttempts;
    Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);Issue(ETacticalCommand::RealtimePause);AdvanceRealtime(.01f);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.battleReview.tacticAttempts==Attempts+1 && State->Journey.lastChain.completed==1,TEXT("the next actual solo tactic succeeds through the normal command path"))) return false;
    // Manual commands receive a recovery explanation, not an invented cancel action.
    Start(false);State->Journey.Hold(0);SelectTactic(int(ChainTactic::Feint));Refresh();
    if(!Check(!CanIssue(ETacticalCommand::ReleaseGuard) && !CanIssue(ETacticalCommand::QueueTactic) && Presentation().GuardContext.Contains(TEXT("Space")),TEXT("manual guard directs the player to the next command"))) return false;
    Issue(ETacticalCommand::EndTurn);Issue(ETacticalCommand::Skip);
    if(!Check(CanIssue(ETacticalCommand::QueueTactic),TEXT("manual next command restores a normal tactic"))) return false;
    PrepareGuardControlPreview(false);
    if(!Check(Presentation().GuardContext.Contains(TEXT("烽火")) && Presentation().GuardHint.Contains(TEXT("維持")),TEXT("an actual beacon guard warns about the objective before release"))) return false;
    // Guidance reads friendly/public data, not the unobserved reserve.
    const auto Context=Presentation().GuardContext,Hint=Presentation().GuardHint;
    State->Journey.world.armies[4].tile=Campaign::At(1,1);State->Journey.world.armies[4].troops=100;Refresh();
    if(!Check(Presentation().GuardContext==Context && Presentation().GuardHint==Hint,TEXT("hidden enemy changes do not leak through guard advice"))) return false;
    State->Journey.world.armies[2].troops=0;Refresh();
    if(!Check(!CanIssue(ETacticalCommand::ReleaseGuard),TEXT("a destroyed army has no releasable order"))) return false;
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedRealtime;bLegacyJourney=SavedLegacy;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_GUARD_CONTROL: PASS (explicit release, exact state conservation, spent guard and real recovery, signatures, reservations, arrival march, saved orders, manual input, replay lock and public objective warning)"));return true;
}
