#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalContributionQA.h"
#include "JourneySave.hpp"
#include <array>
#include <memory>
#include <utility>
using namespace frontline;
bool ATacticalField::SupportCombinationSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_SUPPORT_COMBINATION: FAIL %s"),Why);return V; };
    Issue(ETacticalCommand::Skip);std::vector<std::uint8_t> Original,Initial,Before,After,Natural,Prepared;
    const bool Legacy=bLegacyJourney,Playback=State->bNoPlayback;const float Speed=PlaybackSpeed,Volume=EffectsVolume;
    if(!Check(ExportSession(Original) && PrepareCooperationCouncilPreview(this,0),TEXT("actual prior victory supplies a normal unpaid council"))) return false;
    State->bNoPlayback=false;PlaybackSpeed=1;EffectsVolume=0;
    Issue(ETacticalCommand::PlanExample);Issue(ETacticalCommand::PlanAmbush);Issue(ETacticalCommand::Deploy);
    auto Save=std::make_unique<JourneySaveData>();
    if(!Check(ExportSession(Before) && DecodeJourneySave(Before,*Save),TEXT("adopted paid deployment exports"))) return false;
    // Explicit local contact fixture, never a claimed campaign victory: both officers can reach the same northern enemy.
    auto& J=Save->journey;const int Goal=J.goal,South=J.Definition().secondGoal;
    for(int Y=4;Y<=10;++Y) for(int X=4;X<=11;++X) J.world.tiles[Campaign::At(X,Y)].terrain=Terrain::Plain;
    for(const auto& P:std::array<std::pair<int,int>,4>{{{0,Campaign::At(6,6)},{1,Goal},{2,South},{3,Campaign::At(6,5)}}}) J.world.armies[P.first].tile=J.world.armies[P.first].target=P.second;
    J.world.armies[3].troops=3000;J.world.armies[3].morale=90;J.world.armies[3].stance=battle::Stance::Balanced;J.world.armies[4].troops=0;
    J.assignments[0]={};J.assignments[1]={JourneyDuty::Hold,Goal,true};J.assignments[2]={JourneyDuty::Hold,South,true};
    J.chain.BeginTurn(J.world);J.guarding={};J.lastChain={};for(int A=0;A<3;++A) J.chain.concealed[A]=false;
    Save->selected=0;Save->target=3;Save->orders.clear();Save->signal.Cancel();Save->watch.Reset();Save->realtimeMode=true;Save->realtime.Reset();
    for(int A=0;A<3;++A) Save->realtime.Queue(J,A,JourneyDirective::Hold);J.RefreshIntelligence();
    if(!Check(EncodeJourneySave(*Save,Initial) && ImportSession(Initial),TEXT("valid explicit two-officer tactical fixture imports"))) return false;
    auto Combo=[&]() { for(int I=0;I<int(State->SupportActions.size());++I) { const auto& C=State->SupportActions[I];if(C.guardActs && !C.followup && C.plan.orders.size()==2) return I; }return -1; };
    auto Follow=[&]() { for(int I=0;I<int(State->SupportActions.size());++I) if(State->SupportActions[I].followup) return I;return -1; };
    if(!Check(State->bSupportBattleChoices && Combo()>=0 && State->HUD.BattleChoices.Num()<=2,TEXT("one officer and a real two-officer decision share the existing two-card limit"))) return false;
    const auto& Card=State->HUD.BattleChoices[Combo()];const FString GuardName=JourneyText(State->Journey.world.generals[State->Journey.world.armies[1].general].name);
    if(!Check(Card.Cost.Contains(GuardName) && Card.Cost.Contains(TEXT("固守")),TEXT("guard participation and lost immediate hold are visible on the card"))) return false;
    const int Version=State->BattleChoiceVersion;ExportSession(Before);Refresh();Refresh();ExportSession(After);
    if(!Check(Before==After && Version==State->BattleChoiceVersion,TEXT("proposal reads spend no command, action, supplies or trust"))) return false;
    --State->Journey.world.armies[1].morale;
    if(!Check(!UseBattleChoice(Combo(),Version) && !State->Journey.chain.spent[0] && !State->Journey.chain.spent[1],TEXT("changed guard circumstances reject a stale combined decision"))) return false;
    const int BeforeLoad=State->BattleChoiceVersion;ImportSession(Initial);
    if(!Check(!UseBattleChoice(Combo(),BeforeLoad),TEXT("loading rejects a press from the prior presentation"))) return false;
    const int Index=Combo();const auto Plan=State->SupportActions[Index];const auto Orders=State->Realtime.orders;
    const int Turn=State->Journey.turn;std::array<int,3> Morale{},Cost{};for(int A=0;A<3;++A) Morale[A]=State->Journey.world.armies[A].morale;
    for(const auto& O:Plan.plan.orders) Cost[O.army]+=TacticChain::Definition(O.tactic).moraleCost;
    if(!Check(UseBattleChoice(Index,State->BattleChoiceVersion) && IsPlaying() && State->Beats.Num()==2 && ReadMissionResponse().Line.IsEmpty(),TEXT("combined choice resolves normally and begins before publishing any success"))) return false;
    const auto& Events=State->Journey.lastChain.events;
    if(!Check(State->Journey.lastChain.completed==2 && Events.size()==2 && Events[0].combo==1 && Events[0].comboPercent==100 && Events[1].combo==2 && Events[1].comboPercent==125,TEXT("same execution gives its second real tactic the normal two-combo multiplier"))) return false;
    for(int A=0;A<3;++A) if(!Check(State->Journey.world.armies[A].morale==Morale[A]-Cost[A] && SameJourneyOrder(Orders[A],State->Realtime.orders[A]),TEXT("exact participant costs preserve every standing order"))) return false;
    if(!Check(State->Journey.world.armies[1].tile==Goal && !State->Journey.chain.moved[1] && State->Journey.chain.spent[0] && State->Journey.chain.spent[1] && !State->Journey.Hold(1) && !State->Journey.chain.spent[2] && State->Journey.world.armies[2].tile==South && State->Journey.turn==Turn,TEXT("guard stays on its post but cannot hold now; the other wing and command clock remain untouched"))) return false;
    AdvancePresentation(State->Beats[0].Duration+.1f);
    if(!Check(IsPlaying() && State->BeatIndex==1 && ReadMissionResponse().Line.IsEmpty(),TEXT("the first successful tactic cannot announce the unplayed follow-through"))) return false;
    AdvancePresentation(20);
    if(!Check(!IsPlaying() && RealtimePaused() && ReadMissionResponse().Actor==1 && ReadMissionResponse().ReplyActor==0 && !ReadMissionResponse().Line.IsEmpty() && ExportSession(Natural),TEXT("only whole-chain natural completion lets the guard and supporter acknowledge their actual attack"))) return false;
    ReadMissionResponse();AdvancePresentation(1);ExportSession(After);
    if(!Check(After==Natural,TEXT("reading the response cannot award trust, resources or objective progress"))) return false;
    if(!Check(ImportSession(Natural) && ReadMissionResponse().Line.IsEmpty(),TEXT("settled save restores the battle without replaying transient dialogue"))) return false;
    ImportSession(Initial);if(!Check(UseBattleChoice(Combo(),State->BattleChoiceVersion),TEXT("same combined command can be checked with skip"))) return false;
    Issue(ETacticalCommand::Skip);ExportSession(After);
    if(!Check(ReadMissionResponse().Line.IsEmpty() && Natural==After,TEXT("skip suppresses dialogue while preserving the exact authoritative combined outcome"))) return false;
    ImportSession(Initial);Issue(ETacticalCommand::RealtimeMode);State->Orders={{0,3,ChainTactic::Feint,-1}};Issue(ETacticalCommand::Execute);AdvancePresentation(20);
    if(!Check(!IsPlaying() && State->Journey.lastChain.completed==1 && State->Journey.chain.pressured[3] && Follow()>=0 && ExportSession(Prepared),TEXT("an ordinary separate feint records the real opening that the guard can follow"))) return false;
    State->Journey.lastChain={};Refresh();
    if(!Check(Follow()<0,TEXT("a pressure flag without the acting officer's real record cannot invent a follow-up handoff"))) return false;
    ImportSession(Prepared);const int FollowIndex=Follow();
    if(!Check(FollowIndex>=0 && State->SupportActions[FollowIndex].plan.orders.size()==1 && State->SupportActions[FollowIndex].plan.orders[0].army==1 && UseBattleChoice(FollowIndex,State->BattleChoiceVersion),TEXT("a recorded opening authorizes only an ordinary guard single tactic"))) return false;
    const auto& Followed=State->Journey.lastChain;
    if(!Check(Followed.completed==1 && Followed.events.size()==1 && Followed.events[0].combo==1 && Followed.events[0].comboPercent==100 && Followed.events[0].setupPercent>100 && State->Journey.world.armies[1].tile==Goal,TEXT("separate follow-up uses the actual opening without claiming a two-combo multiplier or moving the guard"))) return false;
    AdvancePresentation(20);
    if(!Check(!IsPlaying() && ReadMissionResponse().Line.IsEmpty(),TEXT("separate single follow-up uses ordinary cinema without fabricating a joint-chain response"))) return false;
    ImportSession(Initial);State->Journey.world.armies[3].stance=battle::Stance::Guard;State->Journey.RefreshIntelligence();Refresh();
    const int Interrupted=Combo();State->Journey.world.armies[3].troops=1; // Observed/actual discrepancy isolates a later failure, not a claimed campaign outcome.
    if(!Check(Interrupted>=0 && UseBattleChoice(Interrupted,State->BattleChoiceVersion) && State->Journey.lastChain.completed==1 && State->Beats.Num()==2 && State->Beats.Last().Kind==EBattleCue::Stop && !State->Journey.chain.spent[1] && State->Orders.size()==1 && State->Orders[0].army==1,TEXT("later failure preserves the guard's unspent action and its ordinary pending reservation"))) return false;
    AdvancePresentation(20);
    if(!Check(!IsPlaying() && RealtimePaused() && ReadMissionResponse().Line.IsEmpty() && State->Orders.size()==1,TEXT("partial chain finishes paused without inventing a joint success"))) return false;
    Issue(ETacticalCommand::Clear);
    if(!Check(State->Orders.empty() && !State->Journey.chain.spent[1],TEXT("cancelling the pending follower does not consume the guard's action"))) return false;
    Issue(ETacticalCommand::Skip);bLegacyJourney=Legacy;ImportSession(Original);State->bNoPlayback=Playback;PlaybackSpeed=Speed;EffectsVolume=Volume;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_SUPPORT_COMBINATION: PASS (real paid deployment plus local fixture, solo/combo comparison, guard costs and stationary posts, true 125 percent chain, deferred response, skip/load equality, recorded separate 100 percent follow-up)"));return true;
}
