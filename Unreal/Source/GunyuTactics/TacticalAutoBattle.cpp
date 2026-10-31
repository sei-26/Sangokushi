#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyAutoBattle.hpp"
using namespace frontline;
bool ATacticalField::IsAutoBattle() const { return State->bAutoBattle; }
void ATacticalField::StopAutoBattle()
{
    if(!State->bAutoBattle) return;
    State->bAutoBattle=State->bAutoCyclePending=false;
    State->Realtime.paused=true;State->bRealtimeStepping=false;
    for(int A=0;A<3;++A) State->Realtime.Cancel(A);
    State->Orders.clear();State->bRealtimeChainRequested=false;
    State->Report=TEXT("ここからは俺が指揮する！ 戦場を止めた。使った行動や兵力はそのまま、次の一手を選ぼう。");
    Refresh();
}
bool ATacticalField::BeginAutoCycle()
{
    if(!State->bAutoBattle || IsPlaying() || State->Journey.stage!=JourneyStage::Battle) return false;
    // A manually reserved combo always owns its attack budgets.
    if(!State->Orders.empty() || State->ChainSignal.Armed()) { StopAutoBattle();return false; }
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
    bRealtimeCommand=Mode;bStartWithCouncil=Council;bLegacyJourney=Legacy;FirstMission=Mission;bPauseOnBattleAlert=Alerts;State->bNoPlayback=Playback;
    Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_AUTO_BATTLE: PASS (observed plans, actual combo costs and cinematics, immediate takeover, reserved-plan protection, paused manual resume)"));return true;
}
