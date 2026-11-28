#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyAutoBattle.hpp"
using namespace frontline;
bool ATacticalField::IsAutoBattle() const { return State->bAutoBattle; }
void ATacticalField::StopAutoBattle()
{
    if(!State->bAutoBattle) return;
    State->bAutoBattle=State->bAutoCyclePending=false;
    State->DecisionPending={};State->DecisionPaused={};
    State->Realtime.paused=true;State->bRealtimeStepping=false;
    for(int A=0;A<3;++A) State->Realtime.Cancel(A);
    State->Orders.clear();State->bRealtimeChainRequested=false;
    if(!TacticChain::Living(State->Journey.world,State->Selected) || State->Journey.chain.spent[State->Selected])
        for(int A=0;A<3;++A) if(TacticChain::Living(State->Journey.world,A) && !State->Journey.chain.spent[A])
        { State->Selected=A;break; }
    State->Report=TEXT("ここからは俺が指揮する！ 戦場を止めた。使った行動や兵力はそのまま、次の一手を選ぼう。");
    Refresh();
}
void ATacticalField::PublishDecisionMoment()
{
    const auto Moment=State->DecisionPending;
    State->DecisionPending={};
    if(Moment.Kind==gunyudecision::EKind::None || !State->bAutoBattle || !bPauseOnBattleAlert ||
        State->Journey.stage!=JourneyStage::Battle) return;
    State->Realtime.paused=true;
    State->bAutoCyclePending=false;
    State->Orders.clear();
    State->DecisionPaused=Moment;
    State->Report=Moment.Message;
}
bool ATacticalField::BeginAutoCycle()
{
    if(!State->bAutoBattle || IsPlaying() || State->Journey.stage!=JourneyStage::Battle) return false;
    // A manually reserved combo always owns its attack budgets.
    if(!State->Orders.empty() || State->ChainSignal.Armed()) { StopAutoBattle();return false; }
    if(State->Journey.CanChooseGrandApproach()) State->Journey.ChooseGrandApproach(GrandApproach::CentralAssault);
    if(State->Journey.CanChooseYangrenApproach()) State->Journey.ChooseYangrenApproach(YangrenApproach::Forest);
    State->Journey.RefreshIntelligence();
    const auto Plan=PlanJourneyAutoBattle(State->Journey);
    State->Realtime.orders=Plan.movement.orders;
    State->Orders=Plan.tactics;State->bAutoCyclePending=true;
    State->Report=TEXT("武将たちが動く！ ")+JourneyText(Plan.reasons[State->Selected]);
    if(State->Orders.empty()) return false;
    DispatchQueuedChain();Refresh();return true;
}
bool ATacticalField::AutoBattleSmokeTest()
{
    const bool Mode=bRealtimeCommand,Council=bStartWithCouncil,Legacy=bLegacyJourney,Playback=State->bNoPlayback;
    const int Mission=FirstMission;const bool Alerts=bPauseOnBattleAlert;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_AUTO_BATTLE: FAIL %s"),Why);return V; };
    bRealtimeCommand=false;bStartWithCouncil=false;bLegacyJourney=true;FirstMission=0;State->bNoPlayback=false;bPauseOnBattleAlert=false;
    Issue(ETacticalCommand::Restart);const int Initial=State->Journey.world.armies[3].troops,Turn=State->Journey.turn;
    Issue(ETacticalCommand::AutoBattle);
    if(!Check(IsAutoBattle() && bRealtimeCommand && !RealtimePaused() && State->Journey.world.armies[3].troops==Initial,TEXT("delegation starts a real clock without a free action"))) return false;
    AdvanceRealtime(12.1f);
    if(!Check(State->Journey.lastChain.completed>0 && IsPlaying() && State->Journey.world.armies[3].troops<Initial,TEXT("observed AI tactics consume real budgets and enter cinematic playback"))) return false;
    Issue(ETacticalCommand::AutoBattle);const int Revision=State->Journey.world.revision;const float Beat=State->BeatTime;
    AdvanceRealtime(30);AdvancePresentation(1);
    if(!Check(!IsAutoBattle() && RealtimePaused() && State->BeatTime==Beat && State->Journey.world.revision==Revision && State->Journey.turn==Turn,TEXT("takeover during playback freezes action and never grants an extra turn"))) return false;
    // Resume finishes the existing action, rather than duplicating the combo.
    Issue(ETacticalCommand::RealtimePause);AdvancePresentation(30);Issue(ETacticalCommand::RealtimePause);
    State->bNoPlayback=true;Issue(ETacticalCommand::Restart);
    State->Orders={{0,3,ChainTactic::Feint,-1}};
    if(!Check(!CanIssue(ETacticalCommand::AutoBattle),TEXT("delegation cannot destroy the player's reserved plan"))) return false;
    State->Orders.clear();State->ChainSignal.Cancel();Issue(ETacticalCommand::AutoBattle);
    std::vector<std::uint8_t> Bytes;
    if(!Check(ExportSession(Bytes) && ImportSession(Bytes) && !IsAutoBattle() && RealtimePaused(),TEXT("loaded sessions always return paused under manual control"))) return false;
    if(!Check(PrepareDecisionPausePreview(true),TEXT("the actual march starts the Guangzong turning point without displaying it early"))) return false;
    AdvancePresentation(10);
    if(!Check(RealtimePaused() && IsAutoBattle() && Presentation().bDecisionPaused && Presentation().MissionFocusTile==GuangzongRoad &&
        Presentation().NextLabel.Contains(TEXT("武将に任せて再開")),TEXT("delegation stops after playback with the choice and observed location"))) return false;
    const int DecisionTurn=State->Journey.turn;AdvanceRealtime(100);
    if(!Check(State->Journey.turn==DecisionTurn,TEXT("no additional update while waiting for a decision"))) return false;
    Issue(ETacticalCommand::RealtimePause);
    if(!Check(!RealtimePaused() && IsAutoBattle() && !Presentation().bDecisionPaused,TEXT("Space keeps delegation and dismisses the stop"))) return false;
    if(!Check(PrepareDecisionPausePreview(true),TEXT("the route decision can be reached again on a new deployment"))) return false;
    AdvancePresentation(10);Issue(ETacticalCommand::AutoBattle);
    if(!Check(RealtimePaused() && !IsAutoBattle() && !Presentation().bDecisionPaused &&
        State->Realtime.orders[0].kind==JourneyDirective::None,TEXT("manual takeover cancels the delegated movement but preserves the battle"))) return false;
    if(!Check(PrepareDecisionPausePreview(false),TEXT("turning points also occur with optional pauses disabled"))) return false;
    AdvancePresentation(10);
    if(!Check(!RealtimePaused() && IsAutoBattle() && !Presentation().bDecisionPaused,TEXT("disabled alert pauses let delegation continue"))) return false;
    // Exercise the actual auto clock and dispatch, including a support opening
    // at full morale instead of choosing the largest solo hit first.
    Issue(ETacticalCommand::Skip);
    bRealtimeCommand=false;bLegacyJourney=true;FirstMission=0;bPauseOnBattleAlert=false;
    Issue(ETacticalCommand::Restart);
    {
        auto& FlankJourney=State->Journey;auto& FlankWorld=FlankJourney.world;
        for(int Y=0;Y<StrategistJourney::Rows;++Y)
            for(int X=0;X<StrategistJourney::Columns;++X) FlankWorld.tiles[Campaign::At(X,Y)].terrain=Terrain::Plain;
        FlankWorld.armies[0].tile=FlankWorld.armies[0].target=Campaign::At(7,6);
        FlankWorld.armies[1].tile=FlankWorld.armies[1].target=Campaign::At(7,5);
        FlankWorld.armies[2].troops=FlankWorld.armies[4].troops=0;
        FlankWorld.armies[3].tile=FlankWorld.armies[3].target=Campaign::At(9,7);
        FlankWorld.armies[3].troops=2000;
        FlankJourney.chain.Reset(FlankWorld);FlankJourney.chain.disordered[3]=true;
        FlankJourney.RefreshIntelligence();Refresh();
        Issue(ETacticalCommand::AutoBattle);AdvanceRealtime(12.1f);
        if(!Check(IsPlaying() && FlankWorld.armies[0].tile==Campaign::At(8,6) &&
            FlankWorld.armies[1].tile==Campaign::At(8,5),
            TEXT("the auto clock advances both officers on distinct approaches instead of cancelling one"))) return false;
    }
    Issue(ETacticalCommand::Skip);bRealtimeCommand=false;Issue(ETacticalCommand::Restart);
    auto& J=State->Journey;auto& W=J.world;
    for(int A=3;A<int(W.armies.size());++A) W.armies[A].troops=0;
    const int Target=Campaign::At(7,7);
    const auto Neighbours=W.MapNeighbors(Target);
    for(int A=0;A<3;++A)
    {
        W.armies[A].tile=W.armies[A].target=Neighbours[A];
        W.armies[A].troops=1400;W.armies[A].morale=100;
        W.tiles[Neighbours[A]].terrain=Terrain::Plain;
    }
    int Support=-1;
    for(int Tile:W.MapNeighbors(W.armies[1].tile))
        if(W.MapDistance(Tile,Target)==2 && W.MapDistance(Tile,W.armies[2].tile)<=2 && TacticChain::Free(W,Tile))
        { Support=Tile;break; }
    if(!Check(Support>=0,TEXT("the support officer has a legal position behind the attackers"))) return false;
    W.armies[0].tile=W.armies[0].target=Support;
    W.armies[3].tile=W.armies[3].target=Target;W.armies[3].troops=10000;W.armies[3].morale=100;
    W.armies[3].stance=battle::Stance::Guard;W.tiles[Target].terrain=Terrain::Plain;
    J.chain.Reset(W);J.RefreshIntelligence();Refresh();
    Issue(ETacticalCommand::AutoBattle);AdvanceRealtime(12.1f);
    if(!Check(IsPlaying() && J.lastChain.completed==3 && J.lastChain.events.front().tactic==ChainTactic::Oath &&
        J.lastChain.events.back().comboPercent==150 && J.chain.signatureUsed[0] &&
        J.chain.spent[0] && J.chain.spent[1] && J.chain.spent[2],
        TEXT("delegation dispatches a real three-officer combo, rally first and 150 percent finisher"))) return false;
    Issue(ETacticalCommand::Skip); // Finish the tested combo before restoring the next test's battle.
    bRealtimeCommand=Mode;bStartWithCouncil=Council;bLegacyJourney=Legacy;FirstMission=Mission;bPauseOnBattleAlert=Alerts;State->bNoPlayback=Playback;
    Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_AUTO_BATTLE: PASS (coordinated approaches, three-officer combo, rally, actual costs, decision pause, takeover, reservations and load)"));return true;
}

bool ATacticalField::PrepareDecisionPausePreview(bool bAlerts)
{
    Issue(ETacticalCommand::Skip);
    bRealtimeCommand=true;bStartWithCouncil=false;bLegacyJourney=false;FirstMission=0;State->bNoPlayback=false;
    bPauseOnBattleAlert=bAlerts;Issue(ETacticalCommand::Restart);
    auto& J=State->Journey;
    J.ResetCampaign();J.Meet();J.mission=YellowTurbanMission;
    for(int I=0;I<YellowTurbanMission;++I) J.history[I].completed=J.history[I].won=true;
    State->BeforeBattle=std::make_unique<StrategistJourney>(J);
    if(!J.Prepare(1,1)) return false;
    Refresh();
    Issue(ETacticalCommand::AutoBattle);
    if(!State->Realtime.Queue(J,0,JourneyDirective::March,Campaign::At(10,8))) return false;
    RunRealtimeCycle(false);
    return IsPlaying() && !Presentation().bDecisionPaused && J.GuangzongCommandsLeft()>0;
}
