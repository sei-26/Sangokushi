#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalContributionQA.h"
#include "JourneySave.hpp"
#include <memory>
using namespace frontline;
bool ATacticalField::SupportActionsSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_SUPPORT_ACTIONS: FAIL %s"),Why);return V; };
    std::vector<std::uint8_t> Original,Initial,Before,After,Natural;Issue(ETacticalCommand::Skip);
    const bool Legacy=bLegacyJourney,Playback=State->bNoPlayback;const float Speed=PlaybackSpeed,Volume=EffectsVolume;
    if(!Check(ExportSession(Original) && PrepareCooperationCouncilPreview(this,0),TEXT("actual cooperative victory reaches the next unpaid council"))) return false;
    State->bNoPlayback=false;PlaybackSpeed=1;EffectsVolume=0;
    Issue(ETacticalCommand::PlanExample);Issue(ETacticalCommand::PlanAmbush);Issue(ETacticalCommand::Deploy);SelectOfficer(0);
    auto Save=std::make_unique<JourneySaveData>();
    if(!Check(ExportSession(Before) && DecodeJourneySave(Before,*Save),TEXT("normal adopted paid deployment exports"))) return false;
    // Explicit tactical fixture, not a claimed campaign victory: a rear support position and a visible enemy at the northern fire.
    auto& J=Save->journey;const int Goal=J.goal,Start=Campaign::At(9,6),Enemy=Campaign::At(8,6);
    for(int Y=4;Y<=8;++Y) for(int X=6;X<=11;++X) J.world.tiles[Campaign::At(X,Y)].terrain=Terrain::Plain;
    J.world.armies[0].tile=J.world.armies[0].target=Start;J.world.armies[1].tile=J.world.armies[1].target=Goal;
    J.world.armies[3].tile=J.world.armies[3].target=Enemy;J.world.armies[3].troops=1600;J.world.armies[3].stance=battle::Stance::Balanced;
    J.assignments[0]={};J.assignments[1]={JourneyDuty::Hold,Goal,true};J.chain.BeginTurn(J.world);J.guarding={};
    for(int A=0;A<3;++A) J.chain.concealed[A]=false;
    Save->selected=0;Save->target=3;Save->orders.clear();Save->signal.Cancel();Save->watch.Reset();Save->realtimeMode=true;
    Save->realtime.Queue(J,0,JourneyDirective::Hold);Save->realtime.Queue(J,1,JourneyDirective::Hold);Save->realtime.paused=true;J.RefreshIntelligence();
    if(!Check(EncodeJourneySave(*Save,Initial) && ImportSession(Initial),TEXT("valid explicit rear support fixture imports"))) return false;
    auto Lure=[&]() { for(int I=0;I<int(State->SupportActions.size());++I) if(State->SupportActions[I].plan.orders.front().tactic==ChainTactic::Lure) return I;return -1; };
    if(!Check(State->bSupportBattleChoices && Lure()>=0 && State->HUD.BattleChoices.Num()<=2,TEXT("arrived Hold support offers legal single-tactic decisions"))) return false;
    const int Version=State->BattleChoiceVersion;ExportSession(Before);Refresh();Refresh();ExportSession(After);
    if(!Check(Before==After && Version==State->BattleChoiceVersion,TEXT("reading offers spends no resources, trust, command or action"))) return false;
    --State->Journey.world.armies[0].morale;
    if(!Check(!UseBattleChoice(Lure(),Version) && State->Orders.empty(),TEXT("a changed public circumstance rejects the old press"))) return false;
    const int BeforeLoad=State->BattleChoiceVersion;ImportSession(Initial);
    if(!Check(!UseBattleChoice(Lure(),BeforeLoad) && State->Orders.empty(),TEXT("loading invalidates an earlier button press"))) return false;
    State->bRealtimeChainRequested=true;Refresh();
    if(!Check(State->HUD.BattleChoices.IsEmpty() && !UseBattleChoice(0,State->BattleChoiceVersion),TEXT("pending realtime dispatch cannot fall back into a different clickable plan"))) return false;
    State->bRealtimeChainRequested=false;Refresh();
    State->Orders={{0,3,ChainTactic::Feint,-1}};Refresh();
    if(!Check(State->HUD.BattleChoices.IsEmpty() && !UseBattleChoice(0,State->BattleChoiceVersion) && State->Orders.size()==1,TEXT("reserved tactics cannot be replaced by an assistance decision"))) return false;
    ImportSession(Initial);State->Realtime.Queue(State->Journey,0,JourneyDirective::March,Campaign::At(10,6));Refresh();
    if(!Check(!State->bSupportBattleChoices && State->Realtime.orders[0].kind==JourneyDirective::March,TEXT("existing marching orders are not silently repurposed"))) return false;
    ImportSession(Initial);State->Journey.chain.moved[0]=true;Refresh();
    if(!Check(State->bSupportBattleChoices && Lure()<0,TEXT("movement already used removes only the unavailable lure"))) return false;
    for(const auto& Card:State->HUD.BattleChoices) if(!Check(!Card.Title.Contains(TEXT("押し返")),TEXT("ordinary Liu tactics do not claim an unavailable push"))) return false;
    ImportSession(Initial);const int Index=Lure();const auto Plan=State->SupportActions[Index];const auto Orders=State->Realtime.orders;
    const int Troops=State->Journey.world.armies[0].troops,Morale=State->Journey.world.armies[0].morale,Turn=State->Journey.turn;
    if(!Check(UseBattleChoice(Index,State->BattleChoiceVersion) && IsPlaying() && ReadMissionResponse().Line.IsEmpty() && CurrentBeat()->SupportGuard==1 && CurrentBeat()->SupportGoal==Goal,TEXT("a real immediate tactic plays before publishing its local result"))) return false;
    if(!Check(State->Journey.lastChain.completed==1 && State->Journey.world.armies[0].troops==Troops-Plan.plan.friendlyLoss && State->Journey.world.armies[0].morale==Morale-Plan.plan.moraleCost && State->Journey.chain.moved[0] && State->Journey.chain.spent[0],TEXT("lure pays its exact normal morale, bait losses and movement/action budgets"))) return false;
    if(!Check(State->Journey.world.armies[1].tile==Goal && !State->Journey.chain.spent[1] && !State->Journey.chain.moved[1] && State->Journey.turn==Turn && !UseBattleChoice(Index,Version),TEXT("guard stays ready and playback cannot spend a second command"))) return false;
    for(int A=0;A<3;++A) if(!Check(SameJourneyOrder(Orders[A],State->Realtime.orders[A]),TEXT("all standing orders are preserved by the explicit single tactic"))) return false;
    AdvancePresentation(20);
    if(!Check(!IsPlaying() && RealtimePaused() && ReadMissionResponse().Actor==0 && ReadMissionResponse().ReplyActor==1 && !ReadMissionResponse().Line.IsEmpty() && ExportSession(Natural),TEXT("natural completion acknowledges actual relief with the acting officer and guard"))) return false;
    ReadMissionResponse();ReadMissionResponse();AdvancePresentation(1);ExportSession(After);
    if(!Check(Natural==After,TEXT("reading the response cannot award resources, trust or objective progress"))) return false;
    AdvancePresentation(4);
    if(!Check(ReadMissionResponse().Line.IsEmpty(),TEXT("the response expires without an extra input"))) return false;
    if(!Check(ImportSession(Natural) && ReadMissionResponse().Line.IsEmpty(),TEXT("loading the settled action does not replay a transient response"))) return false;
    ImportSession(Initial);
    if(!Check(UseBattleChoice(Lure(),State->BattleChoiceVersion),TEXT("same initial public decision can be repeated for skip comparison"))) return false;
    Issue(ETacticalCommand::Skip);ExportSession(After);
    if(!Check(ReadMissionResponse().Line.IsEmpty() && Natural==After,TEXT("skipping preserves the exact authoritative outcome and suppresses the response"))) return false;
    ImportSession(Initial);bRealtimeCommand=false;State->bRealtimeInBattle=false;Refresh();
    if(!Check(UseBattleChoice(Lure(),State->BattleChoiceVersion) && IsPlaying() && State->Journey.turn==Turn && State->Journey.chain.spent[0],TEXT("manual support uses the same ordinary immediate tactic and playback"))) return false;
    Issue(ETacticalCommand::Skip);bLegacyJourney=Legacy;ImportSession(Original);State->bNoPlayback=Playback;PlaybackSpeed=Speed;EffectsVolume=Volume;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_SUPPORT_ACTIONS: PASS (paid deployment plus explicit contact fixture, immutable/stale/load guards, ordinary tactic budgets, stationary guard/orders, natural response, skip equality and manual play)"));return true;
}
