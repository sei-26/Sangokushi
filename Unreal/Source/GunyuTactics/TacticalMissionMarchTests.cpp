#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
using namespace frontline;

bool ATacticalField::MissionMarchSmokeTest()
{
    auto Check=[](bool Value,const TCHAR* Why)
    { if(!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_MISSION_MARCH: FAIL %s"),Why);return Value; };
    const bool Mode=bRealtimeCommand,Grand=bGrandHistoricalBattles,Council=bStartWithCouncil,Legacy=bLegacyJourney;
    const bool Playback=State->bNoPlayback,Alerts=bPauseOnBattleAlert;const int Mission=FirstMission;
    std::vector<std::uint8_t> Before,After,ExpectedBytes;
    if(!Check(PrepareGrandBattlePreview(YellowTurbanMission,0,1),TEXT("actual relief deployment"))) return false;
    SelectOfficer(0);State->Realtime.paused=false;Refresh();
    const auto Proposal=Presentation().MissionMarch;
    if(!Check(Proposal.bCanIssue && Proposal.Route.Num()>1,TEXT("the normal relief battlefield offers a public route for the selected officer"))) return false;
    if(!ExportSession(Before)) return false;
    Refresh();Refresh();
    if(!Check(ExportSession(After) && Before==After,TEXT("reading a task route never advances time, reveals enemies or changes resources"))) return false;
    auto Expected=std::make_unique<JourneySaveData>();
    if(!DecodeJourneySave(Before,*Expected)) return false;
    Expected->realtime.Queue(Expected->journey,Proposal.Army,JourneyDirective::March,Proposal.Destination);
    Expected->realtime.paused=true;
    if(!Check(EncodeJourneySave(*Expected,ExpectedBytes) && IssueMissionMarch(Proposal) && ExportSession(After) && After==ExpectedBytes,
        TEXT("issuing is exactly one normal march plus pause; all actions, officers, resources, intel and elapsed time stay unchanged"))) return false;
    const auto Ordered=After;
    if(!Check(!IssueMissionMarch(Proposal) && ExportSession(After) && Ordered==After,TEXT("a repeated click cannot replace a standing march"))) return false;
    if(!Check(ImportSession(Ordered) && State->Realtime.orders[Proposal.Army].destination==Proposal.Destination,
        TEXT("the real order survives save and reload"))) return false;
    const int Start=State->Journey.world.armies[Proposal.Army].tile,Turn=State->Journey.turn;
    Issue(ETacticalCommand::RealtimeStep);AdvancePresentation(60);
    if(!Check(State->Journey.turn==Turn+1 && State->Journey.world.armies[Proposal.Army].tile!=Start,
        TEXT("movement requires a real simultaneous cycle with the other armies"))) return false;

    if(!PrepareGrandBattlePreview(YellowTurbanMission,0,1)) return false;
    SelectOfficer(0);const auto Stale=Presentation().MissionMarch;
    SelectOfficer(2);if(!ExportSession(Before)) return false;
    if(!Check(!IssueMissionMarch(Stale) && ExportSession(After) && Before==After,TEXT("a pressed proposal cannot redirect another selected officer"))) return false;
    SelectOfficer(1);
    State->Journey.chain.concealed[1]=true;Refresh(); // Explicit ambush-condition fixture.
    if(!Check(State->Journey.chain.concealed[1] && !Presentation().MissionMarch.bCanIssue,
        TEXT("a mission shortcut preserves the ambush instead of silently exposing its officer"))) return false;
    SelectOfficer(0);State->Orders={{0,-1,ChainTactic::Oath,-1}};Refresh();
    if(!ExportSession(Before)) return false;
    if(!Check(!Presentation().MissionMarch.bCanIssue && !IssueMissionMarch(Stale) && ExportSession(After) && Before==After,
        TEXT("manual tactic reservations cannot be discarded by the shortcut"))) return false;
    State->Orders.clear();Refresh();
    const auto Public=Presentation().MissionMarch;
    auto Original=std::make_unique<StrategistJourney>(State->Journey);
    for(auto& Contact:State->Journey.intelligence[0].contacts) if(Contact.id==9) Contact.current=false;
    Refresh();const auto FogBefore=Presentation().MissionMarch;
    if(FogBefore.Route.Num()<2) return false;
    State->Journey.world.armies[9].tile=FogBefore.Route[1];Refresh();
    if(!Check(Presentation().MissionMarch.Destination==FogBefore.Destination && Presentation().MissionMarch.Route==FogBefore.Route,
        TEXT("an unobserved private occupant never changes the proposed route"))) return false;
    State->Journey=*Original;Refresh();
    if(!Check(Public.Destination==Presentation().MissionMarch.Destination,TEXT("restoring the actual observation restores the same proposal"))) return false;

    for(int Battle:{YellowTurbanMission,CoalitionMission,YangrenMission})
    {
        if(!Check(PrepareGrandBattlePreview(Battle,2,1),TEXT("actual completed campaign battle"))) return false;
        const auto& P=Presentation().Journey;
        if(!Check(P.bDebrief && P.BattleResponse.Num()>=2 && P.BattleResponse.Last().bProtagonist &&
            P.BattleResponse[0].Speaker>=0 && !P.BattleResponse[0].Name.IsEmpty(),
            TEXT("each actual battle shows its officers and protagonist responding with the matching portrait"))) return false;
        for(const auto& Line:P.BattleResponse)
        {
            bool Found=false;for(const auto& Source:P.Episode.Lines) if(Source.Speaker==Line.Speaker && Source.Text==Line.Text) Found=true;
            if(!Check(Found,TEXT("the result response comes from the actual outcome scene"))) return false;
        }
        if(!ExportSession(Before)) return false;Refresh();
        if(!Check(ExportSession(After) && Before==After,TEXT("displaying the aftermath never awards additional trust or supplies"))) return false;
    }
    if(!PrepareGrandBattlePreview(YellowTurbanMission,1,2)) return false;
    bool FlagFound=false;
    for(int Actor=0;Actor<3;++Actor) if(State->Journey.world.armies[Actor].tile==State->Journey.goal)
    {
        FlagFound=true;
        SelectOfficer(Actor);
        if(!Check(!Presentation().MissionMarch.bCanIssue,TEXT("the officer holding the captured flag is not sent away by the mission shortcut"))) return false;
    }
    if(!Check(FlagFound,TEXT("the actual breakthrough has an officer holding its flag"))) return false;
    if(!PrepareGrandBattlePreview(YellowTurbanMission,0,1)) return false;
    State->Journey.world.armies[5].troops=0;
    if(!State->Journey.EndTurn()) return false;Refresh();
    if(!Check(!State->Journey.won && Presentation().Journey.bDebrief && !Presentation().Journey.BattleResponse.IsEmpty() &&
        Presentation().Journey.BattleResponse.Last().bProtagonist,TEXT("a lost main army produces the failure conversation, never a victory response"))) return false;
    if(!PrepareEpicPreludePreview(1)) return false;
    if(!Check(Presentation().Journey.bDebrief && Presentation().Journey.BattleResponse.Num()>=2 &&
        Presentation().Journey.BattleResponse.Last().bProtagonist,TEXT("the first battle also introduces the result through characters and protagonist"))) return false;
    for(int Battle:{YellowTurbanMission,CoalitionMission,YangrenMission})
    {
        if(!PrepareGrandBattlePreview(Battle,0,1)) return false;
        if(Battle==YellowTurbanMission && !Check(gunyudecision::Capture(State->Journey).Moment.Kind==gunyudecision::EKind::None,
            TEXT("the modern Guangzong opening never advertises the obsolete civilian-road decision"))) return false;
        bPauseOnBattleAlert=true;Issue(ETacticalCommand::AutoBattle);bool Found=false;
        for(int Cycle=0;Cycle<36 && State->Journey.stage==JourneyStage::Battle;++Cycle)
        {
            AdvanceRealtime(12.1f);
            if(State->DecisionPending.Kind!=gunyudecision::EKind::None && IsPlaying())
                if(!Check(!Presentation().bDecisionPaused,TEXT("a turning point is never exposed before its action playback finishes"))) return false;
            AdvancePresentation(60);
            if(Presentation().bDecisionPaused)
            {
                const auto Kind=State->DecisionPaused.Kind;
                Found=Battle==YangrenMission ? Kind==gunyudecision::EKind::GrandCounter:
                    Kind==gunyudecision::EKind::GrandRelief || Kind==gunyudecision::EKind::GrandOpening;
                if(Found) break;
            }
            if(RealtimePaused() && !IsPlaying()) Issue(ETacticalCommand::RealtimePause);
        }
        if(!Check(Found && IsAutoBattle() && RealtimePaused(),TEXT("real delegation stops for the current battle's rescue, breakthrough or cavalry response"))) return false;
        const int StopTurn=State->Journey.turn;AdvanceRealtime(100);
        const auto Snapshot=gunyudecision::Capture(State->Journey);
        if(!Check(State->Journey.turn==StopTurn && gunyudecision::Compare(Snapshot,Snapshot).Kind==gunyudecision::EKind::None,
            TEXT("a decision waits without spending time and an unchanged phase does not retrigger"))) return false;
        Issue(ETacticalCommand::RealtimePause);
        if(!Check(IsAutoBattle() && !RealtimePaused() && !Presentation().bDecisionPaused,TEXT("resume keeps delegation; intervention remains optional"))) return false;
    }
    bRealtimeCommand=Mode;bGrandHistoricalBattles=Grand;bStartWithCouncil=Council;bLegacyJourney=Legacy;
    State->bNoPlayback=Playback;bPauseOnBattleAlert=Alerts;FirstMission=Mission;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_MISSION_MARCH: PASS (actual public march, no free actions, save/reload, real cycle, stale input, reservations, ambush and flag protection, fog, three battle outcomes, failure response and modern decision pauses)"));
    return true;
}
