#include "TacticalTrial.h"
#include "TacticalState.h"
bool ATacticalField::BattleChoicesSmokeTest()
{
    const bool Mode=bRealtimeCommand,Council=bStartWithCouncil,Legacy=bLegacyJourney,Playback=State->bNoPlayback;
    const int Mission=FirstMission,Policy=EnemyPolicy;const float Speed=PlaybackSpeed,Volume=EffectsVolume;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_BATTLE_CHOICES: FAIL %s"),Why);return V; };
    bRealtimeCommand=false;bStartWithCouncil=false;bLegacyJourney=false;FirstMission=0;EnemyPolicy=0;PlaybackSpeed=1;EffectsVolume=0;State->bNoPlayback=false;
    Issue(ETacticalCommand::Restart);
    if(!Check(State->HUD.BattleChoices.Num()==2 && State->BattleChoices[0].orders.size()==1 && State->BattleChoices[1].orders.size()>1,TEXT("simple battle offers real solo and team tradeoffs"))) return false;
    std::vector<std::uint8_t> Before,After;
    if(!Check(ExportSession(Before),TEXT("initial decision state exports"))) return false;
    const int Version=State->BattleChoiceVersion;const auto Plan=State->BattleChoices[0];
    Refresh();Refresh();
    if(!Check(ExportSession(After) && Before==After && State->BattleChoiceVersion==Version,TEXT("reading and cached refresh grant no troops, food, trust or action"))) return false;
    const int Troops=State->Journey.world.armies[3].troops;
    if(!Check(UseBattleChoice(0,Version) && IsPlaying() && State->Journey.lastChain.completed==1 && Troops-State->Journey.world.armies[3].troops==Plan.damage,TEXT("one click executes the exact real preview and starts playback"))) return false;
    for(int A=0;A<3;++A) if(A!=Plan.orders[0].army)
        if(!Check(!State->Journey.chain.spent[A],TEXT("solo decision preserves the other officers' attack budgets"))) return false;
    if(!Check(!UseBattleChoice(1,Version),TEXT("double click during playback cannot spend again"))) return false;
    Issue(ETacticalCommand::Skip);Issue(ETacticalCommand::Restart);
    const int Stale=State->BattleChoiceVersion;const int EnemyTroops=State->Journey.world.armies[3].troops;
    --State->Journey.world.armies[0].morale;
    if(!Check(!UseBattleChoice(0,Stale) && State->Journey.world.armies[3].troops==EnemyTroops && State->Orders.empty(),TEXT("changed observed circumstances reject a stale card without an action"))) return false;
    const auto Reserved=State->BattleChoices[0].orders;State->Orders=Reserved;Refresh();
    if(!Check(State->HUD.BattleChoices.IsEmpty() && !UseBattleChoice(0,State->BattleChoiceVersion) && State->Orders.size()==Reserved.size(),TEXT("existing manual reservations are never discarded by a quick decision"))) return false;
    State->Orders.clear();bRealtimeCommand=true;Issue(ETacticalCommand::Restart);
    const int Turn=State->Journey.turn;const auto Team=State->BattleChoices[1];
    if(!Check(State->Realtime.paused && UseBattleChoice(1,State->BattleChoiceVersion) && IsPlaying() && State->bRealtimeStepping,TEXT("paused planning takes a direct decision without an extra resume command"))) return false;
    AdvancePresentation(20);
    if(!Check(!IsPlaying() && State->Realtime.paused && !State->bRealtimeStepping && State->Journey.turn==Turn && State->Journey.lastChain.completed==int(Team.orders.size()),TEXT("natural completion returns paused without a free enemy turn or repeated effect"))) return false;
    Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::AutoBattle);
    if(!Check(State->HUD.BattleChoices.IsEmpty() && !UseBattleChoice(0,State->BattleChoiceVersion),TEXT("delegation owns its commands until explicit manual takeover"))) return false;
    StopAutoBattle();
    bRealtimeCommand=Mode;bStartWithCouncil=Council;bLegacyJourney=Legacy;FirstMission=Mission;EnemyPolicy=Policy;PlaybackSpeed=Speed;EffectsVolume=Volume;State->bNoPlayback=Playback;
    Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_BATTLE_CHOICES: PASS (solo/team tradeoffs, exact real costs, cached immutable reads, stale/double-click and reservation guards, paused immediate execution, delegation)"));return true;
}
