#include "../NewGame/JourneyDefense.hpp"
#include "../NewGame/JourneySave.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> out;assert(EncodeJourneySave(s,out));return out; }
static void Place(StrategistJourney& j,int a,int x,int y)
{ j.world.armies[a].tile=j.world.armies[a].target=Campaign::At(x,y); }
static std::unique_ptr<JourneySaveData> Scene()
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=2;s->beforeBattle=j;s->realtimeMode=true;
    assert(j.Prepare(1,0)); // Authored local contact/terrain boundary after an ordinary paid deployment.
    for(int y=1;y<StrategistJourney::Rows-1;++y) for(int x=1;x<StrategistJourney::Columns-1;++x) j.world.tiles[Campaign::At(x,y)].terrain=Terrain::Plain;
    Place(j,0,6,6);Place(j,1,7,6);Place(j,2,8,9);Place(j,3,6,5);j.world.armies[3].troops=3000;j.world.armies[3].morale=90;j.world.armies[4].troops=0;
    j.chain.Reset(j.world);j.RefreshIntelligence();return s;
}
int main()
{
    auto s=Scene();auto& j=s->journey;const auto bytes=Bytes(*s);const auto first=ReadJourneyDefense(j,j.goal);
    assert(first.guard==1 && first.goal==j.goal && first.turn==j.turn && first.threats==1 && first.canGuard && !first.moved && first.reason==JourneyDefenseReason::NeedsGuard);
    assert(ReadJourneyDefense(j,j.Definition().secondGoal).secured && ReadJourneyDefenseCheck(j,j.goal).reason==JourneyDefenseReason::Unchecked);
    for(int n=0;n<10;++n) { ReadJourneyDefense(j,j.goal);ReadJourneyDefenseCheck(j,j.goal); }assert(Bytes(*s)==bytes);
    auto hidden=std::make_unique<JourneySaveData>(*s);hidden->journey.world.armies[4].troops=9000;Place(hidden->journey,4,7,5);
    assert(ReadJourneyDefense(hidden->journey,hidden->journey.goal).threats==1); // No new observation: actual hidden occupancy cannot enter a preview.
    for(auto& contact:hidden->journey.intelligence[0].contacts) contact.current=false;
    assert(ReadJourneyDefense(hidden->journey,hidden->journey.goal).threats==0 && ReadJourneyDefense(hidden->journey,hidden->journey.goal).secured);
    auto limits=Scene();auto& l=limits->journey;auto& guard=l.world.armies[1];
    guard.troops=149;assert(ReadJourneyDefense(l,l.goal).reason==JourneyDefenseReason::Depleted);
    guard.troops=150;guard.morale=24;assert(ReadJourneyDefense(l,l.goal).reason==JourneyDefenseReason::Depleted);
    guard.morale=25;assert(ReadJourneyDefense(l,l.goal).reason==JourneyDefenseReason::GuardWeak);
    guard.troops=499;guard.morale=40;l.guarding[1]=true;assert(!ReadJourneyDefense(l,l.goal).secured);
    guard.troops=500;guard.morale=39;assert(!ReadJourneyDefense(l,l.goal).secured);
    guard.morale=40;assert(ReadJourneyDefense(l,l.goal).secured);
    l.guarding[1]=false;l.chain.spent[1]=true;assert(ReadJourneyDefense(l,l.goal).reason==JourneyDefenseReason::GuardSpent && !ReadJourneyDefense(l,l.goal).canGuard);
    l.chain.spent[1]=false;l.chain.moved[1]=true;assert(ReadJourneyDefense(l,l.goal).canGuard && ReadJourneyDefense(l,l.goal).moved);
    l.world.armies[4].troops=900;Place(l,4,7,5);l.RefreshIntelligence();l.guarding[1]=true;assert(ReadJourneyDefense(l,l.goal).reason==JourneyDefenseReason::Overwhelmed);
    guard.troops=0;assert(ReadJourneyDefense(l,l.goal).reason==JourneyDefenseReason::Vacant);
    assert(ReadJourneyDefense(j,-1).reason==JourneyDefenseReason::Unchecked && ReadJourneyDefense(j,Campaign::At(1,1)).goal<0);
    auto spent=Scene();auto& a=spent->journey;assert(a.Execute({{0,3,ChainTactic::Feint,-1},{1,3,ChainTactic::Disrupt,-1}}).completed==2 && a.chain.disordered[3]);
    assert(spent->realtime.Queue(a,1,JourneyDirective::Hold));assert(spent->realtime.Queue(a,2,JourneyDirective::Hold));
    assert(spent->realtime.Resolve(a) && a.stage==JourneyStage::Battle && a.turn==2 && !a.chain.spent[1] && !a.guarding[1]);
    const auto lost=ReadJourneyDefenseCheck(a,a.goal),south=ReadJourneyDefenseCheck(a,a.Definition().secondGoal);
    assert(lost.turn==1 && lost.guard==1 && lost.attackSpent && !lost.guarded && lost.reason==JourneyDefenseReason::GuardSpent && !lost.secured);
    assert(south.secured && a.heldTurns==0 && ReadJourneyDefense(a,a.goal).reason==JourneyDefenseReason::NeedsGuard);
    const auto saved=Bytes(*spent);auto erased=std::make_unique<JourneySaveData>(*spent);erased->journey.defenseChecks={};assert(Bytes(*erased)==saved);
    auto loaded=Scene();assert(DecodeJourneySave(saved,*loaded) && ReadJourneyDefenseCheck(loaded->journey,loaded->journey.goal).goal<0);
    ++a.turn;assert(ReadJourneyDefenseCheck(a,a.goal).goal<0);--a.turn;
    auto quiet=Scene();quiet->journey.world.armies[3].troops=0;quiet->journey.RefreshIntelligence();assert(quiet->realtime.Resolve(quiet->journey));
    assert(ReadJourneyDefenseCheck(quiet->journey,quiet->journey.goal).secured && ReadJourneyDefenseCheck(quiet->journey,quiet->journey.Definition().secondGoal).secured && quiet->journey.heldTurns==1);
    quiet->journey.stage=JourneyStage::Preparation;assert(ReadJourneyDefenseCheck(quiet->journey,quiet->journey.goal).goal<0 && ReadJourneyDefense(quiet->journey,quiet->journey.goal).goal<0);
    quiet->journey.world.cities[0].troops=6000;quiet->journey.world.cities[0].food=18000;quiet->journey.world.cities[0].gold=900;
    assert(quiet->journey.Prepare(1,0) && quiet->journey.defenseChecks[0].goal<0 && quiet->journey.defenseChecks[1].goal<0);
    a.Reset();assert(a.defenseChecks[0].goal<0 && a.defenseChecks[1].goal<0);
    std::cout<<"Defense PASS: immutable observed conditions, hidden/stale contacts, occupation/brace thresholds, spent/moved distinction, real local checks before recovery, independent wings, unchanged save bytes and no stale/load replay\n";
}
