#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
using namespace frontline;
void ATacticalField::PrepareReliefReadinessPreview(int Variant)
{
    PrepareReliefPreview(Variant==2 ? 1:0);
    if(Variant==1)
    {
        Issue(ETacticalCommand::Cancel);bRealtimeCommand=false;bLegacyJourney=false;bStartWithCouncil=false;FirstMission=2;EnemyPolicy=1;Issue(ETacticalCommand::Restart);
        SelectOfficer(1);Issue(ETacticalCommand::Move);CommandAt(Campaign::At(7,6));Issue(ETacticalCommand::Skip);
        Issue(ETacticalCommand::EndTurn);Issue(ETacticalCommand::Skip);
        SelectOfficer(0);Issue(ETacticalCommand::Move);CommandAt(Campaign::At(8,8));Issue(ETacticalCommand::Skip);
        SelectOfficer(2);BeginTacticPositionComparison();SelectTacticPosition(Campaign::At(9,9));SelectReliefOfficer(0);
    }
    if(Variant==2)
    {
        State->Realtime.Queue(State->Journey,2,JourneyDirective::March,Campaign::At(10,10));
        State->Realtime.AppendWaypoint(State->Journey,2,Campaign::At(11,10));
        State->Realtime.SetArrival(State->Journey,2,JourneyArrival::Hold);
    }
    if(Variant==3) SelectReliefOfficer(1);
    if(Variant==4) State->Orders={{0,3,ChainTactic::Feint,-1}};
    if(Variant==5)
    {
        Issue(ETacticalCommand::Cancel);SelectOfficer(2);Issue(ETacticalCommand::Hold);Issue(ETacticalCommand::Skip);
        BeginTacticPositionComparison();SelectTacticPosition(Campaign::At(9,9));SelectReliefOfficer(0);
    }
    if(Variant==6) State->Journey.world.armies[0].troops=149; // Explicit condition fixture, not earned casualties.
    if(Variant==7)
    {
        // Place a visible opponent to exercise a real tactic without spending movement.
        State->Journey.world.armies[4].tile=State->Journey.world.armies[4].target=Campaign::At(9,8);State->Journey.RefreshIntelligence();
        Issue(ETacticalCommand::Cancel);SelectOfficer(0);SelectTactic(int(ChainTactic::Feint));SelectAt(Campaign::At(9,8));
        Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);Issue(ETacticalCommand::Skip);
        SelectOfficer(2);BeginTacticPositionComparison();SelectTacticPosition(Campaign::At(9,9));SelectReliefOfficer(0);
    }
    Refresh();
}
bool ATacticalField::ReliefReadinessSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_RELIEF_READINESS: FAIL %s"),Why);return V; };
    const int Mission=FirstMission,Policy=EnemyPolicy;const bool Council=bStartWithCouncil,Realtime=bRealtimeCommand,Legacy=bLegacyJourney;
    auto Bytes=[this]() { std::vector<std::uint8_t> B;ExportSession(B);return B; };
    auto Incoming=[this](int Id) { return Presentation().Position.Relief.Officers.FindByPredicate([Id](const auto& R) { return R.Army==Id; }); };
    PrepareReliefReadinessPreview(0);const auto Before=Bytes();
    const auto* In=Incoming(0);
    if(!Check(In && In->bAdjacent && In->bMoveReady && !In->bReserved && Presentation().Position.Relief.bCanIssue &&
        Presentation().Position.Relief.CurrentGuard.Contains(TEXT("条件充足")) && Presentation().Position.Relief.IncomingReadiness.Contains(TEXT("移動未使用")) &&
        Presentation().Position.Relief.Decision.Contains(TEXT("維持見込み")),TEXT("real staged successor is adjacent and ready; observed current guard and post-move forecast stay distinct"))) return false;
    for(int I=0;I<5;++I) { BuildPresentation();RenderEffects();ReadReliefOfficer(0); }
    if(!Check(Bytes()==Before,TEXT("readiness, current guard and drawing do not mutate the saved session"))) return false;
    // Future position selection is independent from the successor's already-known preparation.
    State->ComparedPosition=-1;Refresh();
    if(!Check(Incoming(0)->bMoveReady && Presentation().Position.Relief.Decision.Contains(TEXT("番号を選ぶ")) && !Presentation().Position.Relief.bCanIssue,TEXT("readiness is visible before choosing an outgoing step, without permitting an unspecified commit"))) return false;
    PrepareReliefReadinessPreview(1);In=Incoming(0);
    if(!Check(In && In->bAdjacent && !In->bMoveReady && !Presentation().Position.Relief.bCanIssue &&
        Presentation().Position.Relief.Decision.Contains(TEXT("Space")),TEXT("an actual arriving move still spends the successor's movement this cycle"))) return false;
    const auto Spent=Bytes();if(!Check(!CommitRelief() && Bytes()==Spent,TEXT("the displayed wait never refunds movement or half-commits"))) return false;
    Issue(ETacticalCommand::Cancel);Issue(ETacticalCommand::EndTurn);Issue(ETacticalCommand::Skip);SelectOfficer(2);BeginTacticPositionComparison();SelectReliefOfficer(0);
    int Destination=-1;for(const auto& C:Presentation().Position.Cells) if(C.bMoveAllowed && ReadReliefOfficer(0).Army>=0) { Destination=C.Tile;break; }
    SelectTacticPosition(Destination);In=Incoming(0);
    if(!Check(In && In->bMoveReady && State->Journey.turn==3 && Presentation().Position.Relief.bCanIssue,TEXT("only a real enemy/objective cycle restores the movement and revalidates the new situation"))) return false;
    if(!Check(CommitRelief(),TEXT("the revalidated pair can execute through the ordinary movement path"))) return false;Issue(ETacticalCommand::Skip);
    PrepareReliefReadinessPreview(7);In=Incoming(0);
    if(!Check(In && State->Journey.chain.spent[0] && !State->Journey.chain.moved[0] && In->bMoveReady && Presentation().Position.Relief.bCanIssue &&
        Presentation().Position.Relief.IncomingReadiness.Contains(TEXT("使用済")) && !Presentation().Position.Relief.Decision.Contains(TEXT("Space")),TEXT("a real tactic can spend attack while leaving movement available for a handover"))) return false;
    PrepareReliefReadinessPreview(3);
    if(!Check(!Incoming(1)->bAdjacent && Presentation().Position.Relief.Decision.Contains(TEXT("隣接外")),TEXT("the distant northern guard is not misreported as ready"))) return false;
    PrepareReliefReadinessPreview(4);
    if(!Check(Incoming(0)->bReserved && Presentation().Position.Relief.Decision.Contains(TEXT("予約")),TEXT("existing tactic reservations are explicit and never silently repurposed"))) return false;
    PrepareReliefReadinessPreview(5);
    if(!Check(Presentation().Position.Relief.Decision.Contains(TEXT("守備隊は移動済")),TEXT("the outgoing guard's real used movement is also a blocker"))) return false;
    PrepareReliefReadinessPreview(6);
    if(!Check(Presentation().Position.Relief.IncomingReadiness.Contains(TEXT("兵149")) && Presentation().Position.Relief.Decision.Contains(TEXT("守備条件")),TEXT("arrival permission and actual guard strength remain separate"))) return false;
    PrepareReliefReadinessPreview(2);State->Orders={{1,3,ChainTactic::Feint,-1}};State->ChainSignal.Arm(State->Journey,State->Orders);
    State->Realtime.paused=false;State->Realtime.Advance(.5);Refresh();
    const auto& R=Presentation().Position.Relief;
    if(!Check(R.bCanStopDeparture && R.CurrentGuard.Contains(TEXT("行軍(10,10)")) && R.CurrentGuard.Contains(TEXT("離れる可能性")) &&
        R.IncomingReadiness.Contains(TEXT("行軍(10,8)")) && R.Decision.Contains(TEXT("空白")),TEXT("both actual standing orders and the cost of RT relief are visible"))) return false;
    const auto Snapshot=Bytes();auto Expected=std::make_unique<JourneySaveData>();
    if(!Check(DecodeJourneySave(Snapshot,*Expected),TEXT("the full ongoing session can be independently compared"))) return false;
    Expected->realtime.Cancel(2);Expected->realtime.paused=true;std::vector<std::uint8_t> ExpectedBytes;EncodeJourneySave(*Expected,ExpectedBytes);
    // Changing actual enemies without refreshing contacts must not change any of these reads.
    const auto Guard=R.CurrentGuard,Ready=R.IncomingReadiness,Decision=R.Decision;
    const int Actual=State->Journey.world.armies[3].tile;State->Journey.world.armies[3].tile=Campaign::At(8,10);Refresh();
    if(!Check(Presentation().Position.Relief.CurrentGuard==Guard && Presentation().Position.Relief.IncomingReadiness==Ready && Presentation().Position.Relief.Decision==Decision,
        TEXT("hidden actual positions cannot leak through current guard, readiness or the decision"))) return false;
    State->Journey.world.armies[3].tile=Actual;Refresh();StopReliefDeparture();
    if(!Check(Bytes()==ExpectedBytes && RealtimePaused() && State->ChainSignal.Armed() && !State->HUD.Position.bVisible &&
        !State->Journey.guarding[2],TEXT("explicit departure stop only cancels that guard's standing order and pauses; no free guard, movement, action, resources, signal or other-order changes"))) return false;
    const auto Stopped=Bytes();StopReliefDeparture();
    if(!Check(Bytes()==Stopped && ImportSession(Stopped) && State->Realtime.orders[0].destination==Campaign::At(10,8) && State->Realtime.orders[1].kind==JourneyDirective::Hold,
        TEXT("repeated stop and actual save/load preserve the successor and third officer's orders"))) return false;
    PrepareReliefReadinessPreview(0);const auto Manual=Bytes();StopReliefDeparture();
    if(!Check(!CanStopReliefDeparture() && Bytes()==Manual,TEXT("manual comparison never exposes or executes a standing-order cancellation"))) return false;
    Issue(ETacticalCommand::Cancel);FirstMission=Mission;EnemyPolicy=Policy;bStartWithCouncil=Council;bRealtimeCommand=Realtime;bLegacyJourney=Legacy;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_RELIEF_READINESS: PASS (real arrival wait/recovery, actual tactic versus movement, observed guard, immutable reads, reservations, current orders, exact selective departure stop and save/load)"));return true;
}
