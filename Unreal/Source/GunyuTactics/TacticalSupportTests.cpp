#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalContributionQA.h"
using namespace frontline;
bool ATacticalField::SupportChoicesSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_SUPPORT_CHOICES: FAIL %s"),Why);return V; };
    std::vector<std::uint8_t> Original,Before,After;ExportSession(Original);const bool Legacy=bLegacyJourney;
    if(!Check(PrepareCooperationCouncilPreview(this,0),TEXT("actual prior cooperative victory supplies next council"))) return false;
    Issue(ETacticalCommand::PlanExample);Issue(ETacticalCommand::Deploy);SelectOfficer(0);ExportSession(Before);
    if(!Check(State->HUD.SupportChoices.Num()==2 && State->HUD.SupportChoices[0].bEnabled && State->HUD.SupportChoices[1].bEnabled && RealtimePaused(),TEXT("free Liu receives two reachable support cards after normal deployment"))) return false;
    const auto Plan=State->SupportChoices[0];const int Version=State->SupportVersion;
    Refresh();Refresh();ExportSession(After);
    if(!Check(Before==After && Version==State->SupportVersion,TEXT("presentation and cached read spend nothing"))) return false;
    --State->Journey.world.armies[0].morale;
    if(!Check(!UseSupportChoice(0,Version) && State->Realtime.orders[0].kind==JourneyDirective::None,TEXT("stale officer circumstances reject old press"))) return false;
    const int BeforeLoadVersion=State->SupportVersion;ImportSession(Before);
    if(!Check(!UseSupportChoice(0,BeforeLoadVersion) && State->Realtime.orders[0].kind==JourneyDirective::None,TEXT("press from before loading cannot authorize a new card"))) return false;
    State->Orders={{0,3,ChainTactic::Disrupt,-1}};Refresh();
    if(!Check(State->HUD.SupportChoices.IsEmpty() && !UseSupportChoice(0,State->SupportVersion) && State->Orders.size()==1,TEXT("actor's reserved tactic survives without a replacement order"))) return false;
    ImportSession(Before);State->Orders={{2,3,ChainTactic::Disrupt,-1}};Refresh();
    const auto Guard=State->Realtime.orders[1];const int Start=State->Journey.world.armies[0].tile,Turn=State->Journey.turn;
    if(!Check(UseSupportChoice(0,State->SupportVersion) && State->Realtime.orders[0].kind==JourneyDirective::March && State->Realtime.orders[0].arrival==JourneyArrival::Hold && RealtimePaused() && State->HUD.SupportChoices.IsEmpty(),TEXT("explicit choice installs ordinary march/hold and hides spent choice"))) return false;
    if(!Check(State->Journey.world.armies[0].tile==Start && State->Journey.turn==Turn && !State->Journey.chain.moved[0] && SameJourneyOrder(Guard,State->Realtime.orders[1]) && State->Orders.size()==1 && State->Orders[0].army==2,TEXT("queue grants no movement and preserves guard and other reserved tactic"))) return false;
    ExportSession(After);ImportSession(After);
    if(!Check(State->Realtime.orders[0].destination==Plan.destination && State->Realtime.Route(State->Journey,0)==Plan.route,TEXT("normal pending support route survives save/load"))) return false;
    ImportSession(Before);bRealtimeCommand=false;State->bRealtimeInBattle=false;Refresh();
    int Moving=-1,Adjacent=-1;for(int N=0;N<int(State->SupportChoices.size());++N)
        if(State->SupportChoices[N].steps>0) Moving=N;else Adjacent=N;
    if(!Check(Moving>=0 && Adjacent>=0,TEXT("actual deployment offers distant support and adjacent support"))) return false;
    const int ManualStart=State->Journey.world.armies[0].tile;const bool Used=UseSupportChoice(Moving,State->SupportVersion);
    if(!Used || !IsPlaying() || !State->Journey.chain.moved[0] || State->Journey.world.MapDistance(ManualStart,State->Journey.world.armies[0].tile)!=1)
        UE_LOG(LogTemp,Error,TEXT("GUNYU_SUPPORT_DETAIL used=%d playing=%d moved=%d from=%d to=%d order=%d report=%s"),int(Used),int(IsPlaying()),int(State->Journey.chain.moved[0]),ManualStart,State->Journey.world.armies[0].tile,int(State->Realtime.orders[0].kind),*State->Report);
    if(!Check(Used && IsPlaying() && State->Journey.chain.moved[0] && State->Journey.world.MapDistance(ManualStart,State->Journey.world.armies[0].tile)==1 && State->Realtime.orders[0].kind==JourneyDirective::None,TEXT("manual choice performs ordinary single move with playback"))) return false;
    Issue(ETacticalCommand::Skip);
    if(!Check(!UseSupportChoice(Moving,State->SupportVersion),TEXT("manual movement cannot be spent twice"))) return false;
    ImportSession(Before);bRealtimeCommand=false;State->bRealtimeInBattle=false;Refresh();
    if(!Check(UseSupportChoice(Adjacent,State->SupportVersion) && IsPlaying() && State->Journey.guarding[0] && State->Journey.chain.spent[0] && State->Journey.world.armies[0].tile==ManualStart,TEXT("already adjacent manual support uses ordinary hold and its action with playback"))) return false;
    Issue(ETacticalCommand::Skip);
    ImportSession(Before);SelectOfficer(1);
    if(!Check(State->HUD.SupportChoices.IsEmpty(),TEXT("assigned guard receives no distracting support override"))) return false;
    ImportSession(Original);bLegacyJourney=Legacy;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_SUPPORT_CHOICES: PASS (real council/deployment, immutable cards, stale press, reservation/guard safety, exact ordinary queued route/save and manual one-step playback)"));return true;
}
