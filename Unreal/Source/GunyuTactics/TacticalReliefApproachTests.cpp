#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
using namespace frontline;
void ATacticalField::PrepareReliefApproachPreview(int Variant)
{
    PrepareReliefPreview(1);
    if(Variant==1 || Variant==3)
    {
        // Visible bridge-control fixture: both crossings remain authored ground.
        auto& J=State->Journey;
        J.world.armies[0].tile=J.world.armies[0].target=Variant==3 ? Campaign::At(6,6):Campaign::At(7,7);
        J.world.armies[1].tile=J.world.armies[1].target=Campaign::At(11,6);
        J.world.armies[3].tile=J.world.armies[3].target=Variant==3 ? Campaign::At(7,7):Campaign::At(11,7);
        if(Variant==3) J.world.armies[4].tile=J.world.armies[4].target=Campaign::At(11,7);
        J.RefreshIntelligence();
    }
    if(Variant==2) bRealtimeCommand=false;
    SelectReliefOfficer(1);Refresh();
}
bool ATacticalField::ReliefApproachSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_RELIEF_APPROACH: FAIL %s"),Why);return V; };
    const int Mission=FirstMission,Policy=EnemyPolicy;const bool Council=bStartWithCouncil,Realtime=bRealtimeCommand,Legacy=bLegacyJourney;
    auto Bytes=[this]() { std::vector<std::uint8_t> B;ExportSession(B);return B; };
    PrepareReliefApproachPreview(0);const auto Read=Presentation().Position.Relief.Approach;const auto Before=Bytes();
    if(!Check(Read.bVisible && !Read.bBlocked && Read.bCanQueue && Read.Route.Num()>1 && Read.Officer==1 &&
        State->Journey.world.MapDistance(Read.Staging,ReliefSource())==1 && Read.Warning.Contains(TEXT("守備地点を空ける")),
        TEXT("a distant northern guard has a real staging route and explicit warning about leaving its own post"))) return false;
    for(int I=0;I<5;++I) { ReadReliefApproach(1);BuildPresentation();RenderEffects(); }
    if(!Check(Bytes()==Before,TEXT("route reads and drawing spend no movement, observation, objective or resources"))) return false;
    const int EnemyTile=State->Journey.world.armies[3].tile;
    State->Journey.world.armies[3].tile=Read.Route[1];Refresh();
    if(!Check(Presentation().Position.Relief.Approach.Route==Read.Route && Presentation().Position.Relief.Approach.Summary==Read.Summary,
        TEXT("unchanged contacts hide a changed actual enemy position from the route"))) return false;
    State->Journey.world.armies[3].tile=EnemyTile;Refresh();
    // A changed reviewed destination cannot silently redirect the explicit command.
    State->Journey.world.armies[0].tile=Read.Staging;const auto Stale=Bytes();
    if(!Check(!CanQueueReliefApproach() && !QueueReliefApproach() && Bytes()==Stale,TEXT("stale staging is rejected instead of silently retargeted"))) return false;
    PrepareReliefApproachPreview(1);const auto Blocked=Presentation().Position.Relief.Approach;
    if(!Check(Blocked.bVisible && Blocked.bBlocked && Blocked.Blocker==0 && Blocked.BlockedTile==Campaign::At(7,7) &&
        !Blocked.bCanQueue && !CanQueueReliefApproach() && Blocked.Summary.Contains(TEXT("劉備")),
        TEXT("a friendly west-bridge blocker is identified while the visible enemy covers the other bridge"))) return false;
    const auto BlockBytes=Bytes();if(!Check(!QueueReliefApproach() && Bytes()==BlockBytes,TEXT("a hypothetical path through a friend is never issued as a legal march"))) return false;
    Issue(ETacticalCommand::Cancel);SelectOfficer(0);Issue(ETacticalCommand::Move);CommandAt(Campaign::At(7,6));
    Issue(ETacticalCommand::RealtimeStep);Issue(ETacticalCommand::Skip);SelectOfficer(2);
    BeginTacticPositionComparison();SelectTacticPosition(Campaign::At(9,9));SelectReliefOfficer(1);
    const auto Cleared=Presentation().Position.Relief.Approach;
    if(!Check(State->Journey.world.armies[0].tile==Campaign::At(7,6) && Cleared.bVisible && !Cleared.bBlocked && Cleared.bCanQueue,
        TEXT("an actual normal order and cycle clear the bridge; the approach is recomputed"))) return false;
    State->Journey.assignments[1]={JourneyDuty::Hold,State->Journey.Definition().goal,false};Refresh();
    if(!Check(Presentation().Position.Relief.Approach.Warning.Contains(TEXT("軍議の守備担当")),TEXT("redirecting an unfinished council guard assignment is explicit before staging"))) return false;
    State->Orders={{0,3,ChainTactic::Feint,-1}};State->ChainSignal.Arm(State->Journey,State->Orders);
    State->Realtime.paused=false;State->Realtime.Advance(.5);Refresh();const auto Pending=Bytes();
    auto Expected=std::make_unique<JourneySaveData>();if(!Check(DecodeJourneySave(Pending,*Expected),TEXT("the whole live session can be compared independently"))) return false;
    Expected->realtime.Queue(Expected->journey,1,JourneyDirective::March,Cleared.Staging);Expected->realtime.SetArrival(Expected->journey,1,JourneyArrival::Hold);Expected->realtime.paused=true;
    std::vector<std::uint8_t> ExpectedBytes;EncodeJourneySave(*Expected,ExpectedBytes);
    if(!Check(QueueReliefApproach() && Bytes()==ExpectedBytes && State->ChainSignal.Armed() && State->Journey.battleReview.relief.empty(),
        TEXT("explicit staging replaces only the successor's standing order and pause; no guard departure, free movement or relief story credit"))) return false;
    const auto Saved=Bytes();if(!Check(ImportSession(Saved) && State->Realtime.orders[1].destination==Cleared.Staging &&
        State->Realtime.orders[1].arrival==JourneyArrival::Hold && !Presentation().Position.bVisible,
        TEXT("actual pending staging survives load without persisting the hypothetical selection"))) return false;
    State->ChainSignal.Cancel();const auto Route=State->Realtime.Route(State->Journey,1);const int Turn=State->Journey.turn,Post=State->Journey.world.armies[2].tile;
    Issue(ETacticalCommand::RealtimeStep);Issue(ETacticalCommand::Skip);
    if(!Check(Route.size()>1 && State->Journey.turn==Turn+1 && State->Journey.world.armies[1].tile==Route[1] && State->Journey.world.armies[2].tile==Post,
        TEXT("only an actual following cycle moves the successor along the normal route, leaving the guard in place"))) return false;
    PrepareReliefApproachPreview(3);
    if(!Check(Presentation().Position.Relief.Approach.bVisible && Presentation().Position.Relief.Approach.Route.IsEmpty() &&
        !CanQueueReliefApproach() && Presentation().Position.Relief.Approach.Summary.Contains(TEXT("仮経路なし")),
        TEXT("two observed blocked crossings give no route instead of blaming an unrelated friendly unit"))) return false;
    PrepareReliefApproachPreview(2);const auto Manual=Bytes();
    if(!Check(Presentation().Position.Relief.Approach.bVisible && !CanQueueReliefApproach() && !QueueReliefApproach() && Bytes()==Manual,
        TEXT("manual mode only previews the approach and never queues automatic multi-tile movement"))) return false;
    PrepareReliefApproachPreview(0);State->Orders={{1,3,ChainTactic::Feint,-1}};Refresh();
    if(!Check(!CanQueueReliefApproach() && Presentation().Position.Relief.Decision.Contains(TEXT("予約")),
        TEXT("a reserved successor's tactic is not silently repurposed for staging"))) return false;
    Issue(ETacticalCommand::Cancel);FirstMission=Mission;EnemyPolicy=Policy;bStartWithCouncil=Council;bRealtimeCommand=Realtime;bLegacyJourney=Legacy;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_RELIEF_APPROACH: PASS (observed legal/blocked approaches, actual bridge clearing, immutable reads, stale/queued guards, exact selective staging, save/load and real cycle)"));return true;
}
