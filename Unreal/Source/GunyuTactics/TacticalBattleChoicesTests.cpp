#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalSettledTarget.h"
bool ATacticalField::BattleChoicesSmokeTest()
{
    const bool Mode=bRealtimeCommand,Council=bStartWithCouncil,Legacy=bLegacyJourney,Playback=State->bNoPlayback;
    const bool Grand=bGrandHistoricalBattles,Alerts=bPauseOnBattleAlert;
    const int Mission=FirstMission,Policy=EnemyPolicy;const float Speed=PlaybackSpeed,Volume=EffectsVolume;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_BATTLE_CHOICES: FAIL %s"),Why);return V; };
    bRealtimeCommand=false;bStartWithCouncil=false;bLegacyJourney=false;FirstMission=0;EnemyPolicy=0;PlaybackSpeed=1;EffectsVolume=0;State->bNoPlayback=false;
    Issue(ETacticalCommand::Restart);
    if(!Check(State->HUD.BattleChoices.Num()==2 && State->BattleChoices[0].orders.size()==1 && State->BattleChoices[1].orders.size()>1,TEXT("simple battle offers real solo and team tradeoffs"))) return false;
    for(int I=0;I<State->HUD.BattleChoices.Num();++I)
        for(const auto& Order:State->BattleChoices[I].orders)
            if(!Check(State->HUD.BattleChoices[I].Title.Contains(JourneyText(frontline::TacticChain::Name(Order.tactic))),
                TEXT("each visible proposal names the tactics it will actually execute"))) return false;
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
    if(!Check(PrepareGrandBattlePreview(frontline::YellowTurbanMission,3,1),TEXT("the actual Guangzong relief supplies a settled northern objective"))) return false;
    std::vector<std::uint8_t> Relief;
    if(!ExportSession(Relief)) return false;
    auto& J=State->Journey;
    const int Relieved=J.battleReview.grand.relievedOn;
    State->Selected=1;State->Target=8;State->SelectedTactics[1]=frontline::ChainTactic::Feint;
    J.chain.spent[1]=J.chain.disordered[1]=false;J.world.armies[1].morale=90;
    int Adjacent=-1;
    for(int Tile:J.world.MapNeighbors(J.world.armies[1].tile))
        if(frontline::StrategistJourney::OnMap(Tile) && J.world.tiles[Tile].terrain!=frontline::Terrain::River &&
            J.world.tiles[Tile].terrain!=frontline::Terrain::Mountain && frontline::TacticChain::Free(J.world,Tile)) { Adjacent=Tile;break; }
    if(!Check(Adjacent>=0,TEXT("a legal adjacent tile supplies the explicit manual-pursuit fixture"))) return false;
    J.world.armies[8].tile=Adjacent;J.world.armies[8].troops=1000;J.world.armies[8].morale=25;
    J.battleReview.grand.relievedOn=0;J.RefreshIntelligence();Refresh();
    if(!Check(!State->HUD.BattleChoices.IsEmpty(),TEXT("before relief, the same public enemy has a real usable tactic card"))) return false;
    const int EarlierCard=State->BattleChoiceVersion;
    J.battleReview.grand.relievedOn=Relieved;Refresh();
    if(!Check(State->HUD.BattleChoices.IsEmpty() && State->HUD.BattleChoiceTarget.IsEmpty() &&
        State->HUD.BattleChoicePrompt.Contains(TEXT("完了")),TEXT("settled relief replaces automatic pursuit with its public completed status"))) return false;
    if(!ExportSession(Before)) return false;
    Refresh();
    if(!Check(!UseBattleChoice(0,EarlierCard) && ExportSession(After) && Before==After,
        TEXT("a pre-relief pressed card cannot fire after completion, and reading consumes no action or resources"))) return false;
    if(!Check(CanIssue(ETacticalCommand::QueueTactic),TEXT("a completed objective never disables the player's legal manual pursuit"))) return false;
    Issue(ETacticalCommand::QueueTactic);
    if(!Check(CanFireManualTactics() && FireManualTactics() && J.chain.spent[1] && J.lastChain.completed==1,
        TEXT("explicit manual pursuit still fires through the real tactic pipeline and pays its action"))) return false;
    Issue(ETacticalCommand::Skip);
    if(!ImportSession(Relief)) return false;
    auto Public=std::make_unique<frontline::StrategistJourney>(State->Journey);
    for(auto& C:Public->intelligence[0].contacts) if(C.id==8) C.current=false;
    if(!Check(gunyucommand::SettledTargetReason(*Public,8).IsEmpty(),TEXT("a stale contact never supplies completed-target intelligence"))) return false;
    Public->mission=frontline::YangrenMission;Public->battleReview.grand.yangren.active=true;
    Public->battleReview.grand.yangren.repelledOn=Public->turn;
    for(auto& C:Public->intelligence[0].contacts) if(C.id==8) { C.current=true;C.observed.troops=1000;C.observed.morale=25; }
    if(!Check(!gunyucommand::SettledTargetReason(*Public,8).IsEmpty(),TEXT("the Yangren presentation fixture recognises a currently observed repelled cavalry objective"))) return false;
    for(auto& C:Public->intelligence[0].contacts) if(C.id==8) C.observed.morale=26;
    if(!Check(gunyucommand::SettledTargetReason(*Public,8).IsEmpty(),TEXT("recovered cavalry remains eligible for normal tactical suggestions"))) return false;
    bGrandHistoricalBattles=Grand;bPauseOnBattleAlert=Alerts;
    bRealtimeCommand=Mode;bStartWithCouncil=Council;bLegacyJourney=Legacy;FirstMission=Mission;EnemyPolicy=Policy;PlaybackSpeed=Speed;EffectsVolume=Volume;State->bNoPlayback=Playback;
    Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_BATTLE_CHOICES: PASS (solo/team tradeoffs, exact real costs, stale/double-click and reservation guards, real relief, settled public targets, preserved manual pursuit, paused execution, delegation)"));return true;
}
