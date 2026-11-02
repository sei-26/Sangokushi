#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

bool ATacticalField::RealtimeSmokeTest()
{
    const bool SavedMode=bRealtimeCommand,SavedCouncil=bStartWithCouncil,SavedPlayback=State->bNoPlayback;
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const float SavedSpeed=PlaybackSpeed;
    auto Check=[](bool Value,const TCHAR* Reason)
    {
        if (!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_REALTIME: FAIL: %s"),Reason);
        return Value;
    };
    bRealtimeCommand=true;bStartWithCouncil=false;FirstMission=1;EnemyPolicy=1;State->bNoPlayback=true;
    Issue(ETacticalCommand::Restart);
    if (!Check(RealtimePaused() && State->Journey.turn==1,TEXT("deploy starts paused"))) return false;
    SelectOfficer(0);CommandAt(Campaign::At(9,12));
    const int InitialTile=State->Journey.world.armies[0].tile;
    const int InitialRevision=State->Journey.world.revision;
    AdvanceRealtime(40);
    if (!Check(State->Journey.turn==1 && State->Journey.world.revision==InitialRevision,TEXT("planning and paused time cannot advance simulation"))) return false;
    Issue(ETacticalCommand::RealtimePause);AdvanceRealtime(11.9f);
    if (!Check(State->Journey.turn==1,TEXT("no update before the command deadline"))) return false;
    AdvanceRealtime(.2f);
    if (!Check(State->Journey.turn==2 && State->Journey.world.armies[0].tile!=InitialTile,TEXT("clock advances standing march and enemy/combat without EndTurn input"))) return false;
    Issue(ETacticalCommand::RealtimePause);AdvanceRealtime(40);
    if (!Check(State->Journey.turn==2,TEXT("pause remains stable after a live cycle"))) return false;
    Issue(ETacticalCommand::RealtimeStep);
    if (!Check(State->Journey.turn==3 && RealtimePaused() && State->Journey.world.armies[0].tile==Campaign::At(9,12),TEXT("single cycle reaches destination and stays paused"))) return false;
    const int StableTile=State->Journey.world.armies[0].tile;
    Issue(ETacticalCommand::RealtimeMode);
    if (!Check(!bRealtimeCommand && State->Journey.turn==3 && State->Journey.world.armies[0].tile==StableTile,TEXT("switching to manual preserves the actual battle"))) return false;
    Issue(ETacticalCommand::RealtimeMode);State->bNoPlayback=false;
    Issue(ETacticalCommand::RealtimeStep);AdvancePresentation(.5f);
    if (!Check(IsPlaying(),TEXT("single-cycle update has real presentation"))) return false;
    Issue(ETacticalCommand::RealtimePause);
    const float Beat=State->BeatTime;const FVector Position=State->Units[0].Position;
    AdvancePresentation(1);AdvanceRealtime(1);
    if (!Check(State->BeatTime==Beat && State->Units[0].Position.Equals(Position),TEXT("pause freezes moving formations and effects"))) return false;
    const int Revision=State->Journey.world.revision;
    Issue(ETacticalCommand::Hold);
    if (!Check(State->Realtime.orders[0].kind==JourneyDirective::Hold && State->Journey.world.revision==Revision,TEXT("orders can be changed during paused playback without touching resolved combat"))) return false;
    Issue(ETacticalCommand::RealtimePause);AdvancePresentation(10);
    if (!Check(!IsPlaying(),TEXT("resuming completes the same movement"))) return false;
    FirstMission=0;EnemyPolicy=0;State->bNoPlayback=true;Issue(ETacticalCommand::Restart);
    Issue(ETacticalCommand::Sample);
    const int Enemy=State->Journey.world.armies[3].troops;
    Issue(ETacticalCommand::Execute);AdvanceRealtime(40);
    if (!Check(State->Journey.world.armies[3].troops==Enemy,TEXT("queued chain cannot resolve while time is paused"))) return false;
    Issue(ETacticalCommand::RealtimePause);AdvanceRealtime(.1f);
    if (!Check(State->Journey.lastChain.completed==3 && State->Journey.turn==1,TEXT("chosen chain fires on resume, keeping authentic tactic conditions and action costs"))) return false;
    State->Journey.Withdraw();const int Turn=State->Journey.turn;AdvanceRealtime(100);
    if (!Check(State->Journey.turn==Turn && RealtimePaused(),TEXT("story/debrief cannot accumulate battle time"))) return false;
    bRealtimeCommand=false;FirstMission=0;EnemyPolicy=0;State->bNoPlayback=false;PlaybackSpeed=1;Issue(ETacticalCommand::Restart);
    Issue(ETacticalCommand::RealtimeSpeed);Issue(ETacticalCommand::RealtimeSpeed);
    if(!Check(!bRealtimeCommand && State->Realtime.speed==3 && State->HUD.SpeedLabel==TEXT("3.0×"),TEXT("manual commands can select three-times speed"))) return false;
    const int InitialEnemy=State->Journey.world.armies[3].troops;
    if(!Check(UseBattleChoice(0,State->HUD.BattleChoiceVersion),TEXT("manual solo starts at selected speed"))) return false;
    const int Damage=InitialEnemy-State->Journey.world.armies[3].troops;
    AdvancePresentation(.2f);
    if(!Check(IsPlaying() && FMath::IsNearlyEqual(State->BeatTime,.6f),TEXT("manual playback advances three times without a second speed multiplier"))) return false;
    const float BeforeSpeed=State->BeatTime;
    Issue(ETacticalCommand::RealtimeSpeed);
    if(!Check(State->Realtime.speed==.5 && State->BeatTime==BeforeSpeed,TEXT("changing speed during playback preserves progress"))) return false;
    Issue(ETacticalCommand::RealtimeSpeed);Issue(ETacticalCommand::RealtimeSpeed);Issue(ETacticalCommand::RealtimeSpeed);
    AdvancePresentation(20);
    if(!Check(!IsPlaying() && InitialEnemy-State->Journey.world.armies[3].troops==Damage,TEXT("fast playback neither duplicates damage nor changes its result"))) return false;
    Issue(ETacticalCommand::AutoBattle);AdvanceRealtime(1);
    if(!Check(IsAutoBattle() && State->Realtime.speed==3 && FMath::IsNearlyEqual(State->Realtime.elapsed,3.),TEXT("delegation retains three-times speed and advances the actual clock"))) return false;
    Issue(ETacticalCommand::RealtimePause);const double Paused=State->Realtime.elapsed;AdvanceRealtime(20);
    if(!Check(State->Realtime.elapsed==Paused,TEXT("three-times clock still respects pause"))) return false;
    std::vector<std::uint8_t> Saved;
    if(!Check(ExportSession(Saved) && ImportSession(Saved) && State->Realtime.speed==3 && RealtimePaused() && !IsAutoBattle(),TEXT("three-times save reloads paused with its selected speed"))) return false;
    bRealtimeCommand=SavedMode;bStartWithCouncil=SavedCouncil;FirstMission=SavedMission;EnemyPolicy=SavedPolicy;
    State->bNoPlayback=SavedPlayback;PlaybackSpeed=SavedSpeed;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_REALTIME: PASS (paused planning, autonomous pulse, persistent march, single-step, mode switching, frozen playback, live orders, timed chain, debrief, manual/auto 3x speed and paused reload)"));
    return true;
}
