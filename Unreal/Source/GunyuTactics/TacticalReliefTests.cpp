#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
using namespace frontline;
void ATacticalField::PrepareReliefPreview(int Variant)
{
    Issue(ETacticalCommand::Skip); // End an earlier capture cue before restarting this bounded fixture.
    bLegacyJourney=false;bStartWithCouncil=false;bRealtimeCommand=false;FirstMission=2;EnemyPolicy=1;Issue(ETacticalCommand::Restart);
    // Public moves stage Guan at the north beacon and Liu next to the south one.
    SelectOfficer(1);Issue(ETacticalCommand::Move);CommandAt(Campaign::At(7,6));Issue(ETacticalCommand::Skip);
    SelectOfficer(0);Issue(ETacticalCommand::Move);CommandAt(Campaign::At(8,8));Issue(ETacticalCommand::Skip);
    Issue(ETacticalCommand::EndTurn);Issue(ETacticalCommand::Skip);SelectOfficer(2);
    if(Variant==2)
    { State->Journey.world.armies[3].tile=State->Journey.world.armies[3].target=Campaign::At(9,10);State->Journey.RefreshIntelligence(); }
    if(Variant==1)
    {
        bRealtimeCommand=true;Refresh();
        State->Realtime.Queue(State->Journey,2,JourneyDirective::Hold);
        State->Realtime.Queue(State->Journey,0,JourneyDirective::March,Campaign::At(10,8));
        State->Realtime.AppendWaypoint(State->Journey,0,Campaign::At(11,8));
        State->Realtime.SetArrival(State->Journey,0,JourneyArrival::Hold);
        State->Realtime.Queue(State->Journey,1,JourneyDirective::Hold);
    }
    BeginTacticPositionComparison();SelectTacticPosition(Campaign::At(9,9));SelectReliefOfficer(0);Refresh();
}
bool ATacticalField::ReliefSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_RELIEF: FAIL %s"),Why);return V; };
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedRealtime=bRealtimeCommand,SavedLegacy=bLegacyJourney,SavedNoPlayback=State->bNoPlayback;
    State->bNoPlayback=false;
    auto Bytes=[this]() { std::vector<std::uint8_t> B;ExportSession(B);return B; };
    const int Source=Campaign::At(8,9),Destination=Campaign::At(9,9);
    PrepareReliefPreview(0);Issue(ETacticalCommand::Cancel);SelectOfficer(0);SelectTactic(int(ChainTactic::Oath));
    if(!Check(!CanCompareTacticPositions(),TEXT("support away from a guard point still does not advertise enemy positioning"))) return false;
    SelectOfficer(2);BeginTacticPositionComparison();SelectTacticPosition(Destination);SelectReliefOfficer(0);
    const auto Before=Bytes();const auto Read=ReadReliefOfficer(0);
    if(!Check(State->Journey.turn==2 && State->Journey.heldTurns==1 && Read.bCanIssue && Read.bSecured && Presentation().Position.Relief.Source==Source,TEXT("public staging yields an adjacent unspent successor and a genuine guarded battle"))) return false;
    SelectReliefOfficer(1);SelectReliefOfficer(0);BuildPresentation();RenderEffects();
    if(!Check(Bytes()==Before,TEXT("reading, comparing successor, drawing and predicted control spend nothing"))) return false;
    auto Expected=std::make_unique<JourneySaveData>();DecodeJourneySave(Before,*Expected);
    Expected->journey.RecordReliefOrder(2,0,Source,Destination,false);
    if(!Check(Expected->journey.Move(2,Destination) && Expected->journey.Move(0,Source),TEXT("reference two normal moves succeed"))) return false;
    std::vector<std::uint8_t> ExpectedBytes;EncodeJourneySave(*Expected,ExpectedBytes);
    if(!Check(CommitRelief() && IsPlaying() && State->Beats.Num()==2 && CurrentBeat()->Actor==2 && !CommitRelief(),TEXT("one explicit handover creates two chronological movement beats and blocks repeated execution"))) return false;
    if(!Check(VisualUnits()[2].Position.Equals(SurfaceLocation(Source)) && VisualUnits()[0].Position.Equals(SurfaceLocation(Campaign::At(8,8))),TEXT("windup does not expose final positions"))) return false;
    AdvancePresentation(1.85f/FMath::Clamp(PlaybackSpeed,.5f,2.f));
    if(!Check(CurrentBeat() && CurrentBeat()->Actor==0 && VisualUnits()[2].Position.Equals(SurfaceLocation(Destination)),TEXT("outgoing move precedes successor arrival in playback"))) return false;
    Issue(ETacticalCommand::Skip);
    if(!Check(Bytes()==ExpectedBytes && State->Journey.heldTurns==1 && State->Journey.chain.moved[0] && State->Journey.chain.moved[2] && !State->Journey.chain.spent[0] && !State->Journey.chain.spent[2],TEXT("actual handover equals two ordinary moves byte for byte without turn, tactic or story rewards"))) return false;
    SelectOfficer(0);SelectTactic(int(ChainTactic::Oath));
    if(!Check(CanCompareTacticPositions() && BeginTacticPositionComparison() && Presentation().Position.Relief.bVisible && !Presentation().Position.Title.Contains(TEXT(" → ")),TEXT("support on a guard point can compare relief without falsely naming an enemy target"))) return false;
    Issue(ETacticalCommand::EndTurn);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.heldTurns==2,TEXT("a real subsequent command preserves progress with the actual successor"))) return false;
    PrepareReliefPreview(2);
    if(!Check(ReadReliefOfficer(0).bCanIssue && !ReadReliefOfficer(0).bSecured && ReadReliefOfficer(0).Detail.Contains(TEXT("固守できない")),TEXT("an arriving unit cannot promise guard against an adjacent enemy"))) return false;
    CommitRelief();Issue(ETacticalCommand::Skip);Issue(ETacticalCommand::EndTurn);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.heldTurns==0,TEXT("actual exposed handover loses continuous beacon progress"))) return false;
    PrepareReliefPreview(0);State->Journey.world.armies[0].troops=149;Refresh();
    if(!Check(ReadReliefOfficer(0).bCanIssue && !ReadReliefOfficer(0).bSecured && ReadReliefOfficer(0).Detail.Contains(TEXT("兵150")),TEXT("arrival remains legal but insufficient troops never count as secure"))) return false;
    PrepareReliefPreview(0);State->Journey.chain.moved[0]=true;Refresh();const auto Used=Bytes();
    if(!Check(!ReadReliefOfficer(0).bCanIssue && !CommitRelief() && Bytes()==Used,TEXT("spent successor movement cannot be refunded or half-commit outgoing"))) return false;
    PrepareReliefPreview(0);State->Orders={{0,-1,ChainTactic::Oath,-1}};Refresh();
    if(!Check(!ReadReliefOfficer(0).bCanIssue && ReadReliefOfficer(0).Detail.Contains(TEXT("予約")),TEXT("an incoming actor's existing tactic cannot be silently repurposed"))) return false;
    PrepareReliefPreview(0);SelectTacticPosition(Campaign::At(8,8));
    if(!Check(!ReadReliefOfficer(0).bCanIssue && !CommitRelief(),TEXT("no teleport swap into the successor's occupied tile"))) return false;
    // Moving a hidden reserve changes neither the preview nor the guard warning.
    PrepareReliefPreview(0);const auto Observed=ReadReliefOfficer(0);
    State->Journey.world.armies[3].tile=Campaign::At(1,1);State->Journey.world.armies[3].troops=0;
    State->Journey.world.armies[4].tile=State->Journey.world.armies[4].target=Destination;Refresh();
    if(!Check(ReadReliefOfficer(0).Detail==Observed.Detail && ReadReliefOfficer(0).bSecured==Observed.bSecured,TEXT("unchanged observations isolate the entire handover read from actual hidden forces"))) return false;
    const int IncomingFrom=State->Journey.world.armies[0].tile;
    if(!Check(!CommitRelief() && !State->Journey.chain.moved[2] && !State->Journey.chain.moved[0] && State->Journey.world.armies[0].tile==IncomingFrom,TEXT("actual first-step obstruction stops the whole sequence without spending the successor"))) return false;
    PrepareReliefPreview(0);State->Journey.intelligence[0].visible[Source]=false;
    if(!Check(!CommitRelief(),TEXT("successor destination is revalidated before committing"))) return false;
    // The same observed enemy read used by the map warns about a real competing move.
    PrepareReliefPreview(1);const auto Conflict=ReadReliefOfficer(0).Detail;
    if(!Check(Conflict.Contains(TEXT("敵の推定進路")),TEXT("observed competing destination produces a conditional collision warning"))) return false;
    State->Journey.world.armies[3].tile=Campaign::At(1,1);Refresh();
    if(!Check(ReadReliefOfficer(0).Detail==Conflict,TEXT("collision warning never reads the actual unseen enemy order"))) return false;
    PrepareReliefPreview(1);CommitRelief();Issue(ETacticalCommand::RealtimeStep);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.world.armies[2].tile==Source && State->Journey.world.armies[0].tile==IncomingFrom,TEXT("real enemy competition can stop both handover moves instead of granting automatic passage"))) return false;
    // Future RT orders preserve everything except the selected two orders and pause.
    PrepareReliefPreview(1);State->Orders={{1,3,ChainTactic::Feint,-1}};
    State->ChainSignal.Arm(State->Journey,State->Orders);Refresh();const auto RealtimeBefore=Bytes();
    if(!Check(Presentation().Position.Relief.bCanIssue && !Presentation().Position.Relief.bSecured && Presentation().Position.Relief.Detail.Contains(TEXT("最短2周期")),TEXT("RT explicitly predicts an occupied-tile gap rather than a seamless swap"))) return false;
    Expected=std::make_unique<JourneySaveData>();DecodeJourneySave(RealtimeBefore,*Expected);
    Expected->journey.RecordReliefOrder(2,0,Source,Destination,true);
    Expected->realtime.Queue(Expected->journey,2,JourneyDirective::March,Destination);
    Expected->realtime.Queue(Expected->journey,0,JourneyDirective::March,Source);Expected->realtime.SetArrival(Expected->journey,0,JourneyArrival::Hold);Expected->realtime.paused=true;
    ExpectedBytes.clear();EncodeJourneySave(*Expected,ExpectedBytes);
    if(!Check(CommitRelief() && Bytes()==ExpectedBytes && !State->Journey.chain.moved[2] && State->ChainSignal.Armed(),TEXT("RT commit only replaces two standing orders, retaining all resources, time, reservations, signal and third guard"))) return false;
    const auto Saved=Bytes();
    if(!Check(ImportSession(Saved) && !Presentation().Position.bVisible && State->Realtime.orders[0].arrival==JourneyArrival::Hold,TEXT("actual two orders survive load; the hypothetical selection does not"))) return false;
    // Separate the minimum staging delay from enemy interference. The real
    // objective AI can otherwise collide with the outgoing unit and delay it.
    State->Journey.world.armies[3].tile=State->Journey.world.armies[3].target=Campaign::At(13,12);
    State->Journey.world.armies[4].tile=State->Journey.world.armies[4].target=Campaign::At(13,2);
    State->Journey.RefreshIntelligence();Refresh();
    Issue(ETacticalCommand::RealtimeStep);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.world.armies[2].tile==Destination && State->Journey.world.armies[0].tile==IncomingFrom && State->Journey.heldTurns==0,TEXT("real first RT cycle leaves the beacon empty and resets progress"))) return false;
    Issue(ETacticalCommand::RealtimeStep);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.world.armies[0].tile==Source && State->Journey.heldTurns==1 && !State->Journey.guarding[0] && State->Realtime.orders[0].kind==JourneyDirective::Hold,TEXT("real second cycle brings successor in; arrival guard still waits for its normal action timing"))) return false;
    // If the outgoing move already completes the actual mission, do not issue a second move after battle.
    bRealtimeCommand=false;FirstMission=1;bStartWithCouncil=false;Issue(ETacticalCommand::Restart);
    auto& J=State->Journey;J.world.armies[2].tile=J.world.armies[2].target=Campaign::At(11,11);
    J.world.armies[0].tile=J.world.armies[0].target=Campaign::At(10,11);J.world.armies[3].troops=0;
    J.assignments[2]={JourneyDuty::Hold,Campaign::At(11,11),true};J.RefreshIntelligence();SelectOfficer(2);
    BeginTacticPositionComparison();SelectTacticPosition(J.goal);SelectReliefOfficer(0);
    if(!Check(ReadReliefOfficer(0).bCanIssue,TEXT("capture fixture has a valid departure and successor"))) return false;
    if(!Check(!CommitRelief() && J.stage==JourneyStage::Debrief && J.won && J.world.armies[0].tile==Campaign::At(10,11) && !J.chain.moved[0] && State->Beats.Num()==1,TEXT("real victory after departure stops before unnecessary successor movement"))) return false;
    Issue(ETacticalCommand::Skip);
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedRealtime;bLegacyJourney=SavedLegacy;State->bNoPlayback=SavedNoPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_RELIEF: PASS (public staging, immutable guard forecast, exact normal moves, chronological playback, actual preserved/lost progress, budgets, fog, atomic two-order RT edit, saved staging gap and mission-end interruption)"));return true;
}
