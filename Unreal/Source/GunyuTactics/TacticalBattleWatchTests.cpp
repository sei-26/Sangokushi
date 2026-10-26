#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

void ATacticalField::PrepareBattleWatchPreview()
{
    bRealtimeCommand=true;bStartWithCouncil=false;FirstMission=1;EnemyPolicy=1;Issue(ETacticalCommand::Restart);
    State->Journey.world.armies[3].troops=State->Journey.world.armies[4].troops=0;
    State->Journey.world.armies[0].tile=Campaign::At(4,3);State->Journey.world.armies[1].tile=Campaign::At(5,3);
    State->Journey.chain.BeginTurn(State->Journey.world);State->Journey.RefreshIntelligence();Refresh();
    SelectOfficer(0);CommandAt(Campaign::At(5,4));SelectOfficer(1);CommandAt(Campaign::At(5,4));
    Issue(ETacticalCommand::RealtimeStep);AdvancePresentation(10);
    Issue(ETacticalCommand::RealtimeStep);AdvancePresentation(10);SelectOfficer(0);
}
bool ATacticalField::BattleWatchSmokeTest()
{
    const bool SavedMode=bRealtimeCommand,SavedCouncil=bStartWithCouncil,SavedPlayback=State->bNoPlayback,SavedPause=bPauseOnBattleAlert;
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    auto Check=[](bool Value,const TCHAR* Reason)
    { if (!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_BATTLE_WATCH: FAIL: %s"),Reason);return Value; };
    bPauseOnBattleAlert=true;State->bNoPlayback=true;PrepareBattleWatchPreview();
    if (!Check(Presentation().BattleNotices.Num()==2 && RealtimePaused() && State->Journey.turn==3,
        TEXT("actual simultaneous collision is reported once after two command cycles"))) return false;
    Issue(ETacticalCommand::RealtimeStep);
    if (!Check(Presentation().BattleNotices.Num()==2,TEXT("unchanged obstruction does not produce repeated warnings"))) return false;
    SelectOfficer(1);Issue(ETacticalCommand::RealtimeStop);Issue(ETacticalCommand::RealtimeStep);
    if (!Check(State->Journey.world.armies[0].tile==Campaign::At(5,4),TEXT("changing the real order resolves the conflict"))) return false;

    State->bNoPlayback=false;Issue(ETacticalCommand::Restart);
    State->Journey.world.armies[0].troops=510;Refresh();
    Issue(ETacticalCommand::RealtimePause);AdvanceRealtime(12);
    if (!Check(IsPlaying() && State->bWatchPending && Presentation().BattleNotices.IsEmpty() && !RealtimePaused(),
        TEXT("resolved losses stay private until their playback completes"))) return false;
    AdvancePresentation(10);
    if (!Check(!IsPlaying() && RealtimePaused() && !Presentation().BattleNotices.IsEmpty() && State->Report.Contains(TEXT("伝令")),
        TEXT("critical losses are shown and pause before the next command update"))) return false;
    const int StableTurn=State->Journey.turn;AdvanceRealtime(50);
    if (!Check(State->Journey.turn==StableTurn,TEXT("warning pause freezes the actual battlefield"))) return false;

    Issue(ETacticalCommand::Restart);State->Journey.world.armies[0].troops=510;Refresh();
    Issue(ETacticalCommand::RealtimePause);AdvanceRealtime(12);Issue(ETacticalCommand::RealtimeAlertPause);AdvancePresentation(10);
    if (!Check(!bPauseOnBattleAlert && !RealtimePaused() && !Presentation().BattleNotices.IsEmpty(),
        TEXT("disabling pause during playback preserves reports and continued execution"))) return false;

    FirstMission=0;EnemyPolicy=0;State->bNoPlayback=true;bPauseOnBattleAlert=true;Issue(ETacticalCommand::Restart);
    State->Journey.world.armies[0].troops=510;Refresh();Issue(ETacticalCommand::Sample);Issue(ETacticalCommand::Execute);
    Issue(ETacticalCommand::RealtimePause);AdvanceRealtime(.1f);
    if (!Check(State->Journey.lastChain.completed==3 && RealtimePaused() && !Presentation().BattleNotices.IsEmpty(),
        TEXT("the lure's real sacrifice also triggers a report through normal chain execution"))) return false;
    Issue(ETacticalCommand::RealtimeMode);
    if (!Check(Presentation().BattleNotices.IsEmpty(),TEXT("switching mode clears stale realtime reports"))) return false;
    bRealtimeCommand=SavedMode;bStartWithCouncil=SavedCouncil;bPauseOnBattleAlert=SavedPause;FirstMission=SavedMission;EnemyPolicy=SavedPolicy;
    State->bNoPlayback=SavedPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_BATTLE_WATCH: PASS (real stalls and recovery, deferred observations, damage pause, optional stop, chain losses, reset)"));
    return true;
}
