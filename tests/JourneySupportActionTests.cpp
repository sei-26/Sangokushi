#include "../NewGame/JourneySupportActions.hpp"
#include "../NewGame/JourneySave.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
static void Place(StrategistJourney& j,int army,int x,int y)
{ j.world.armies[army].tile=j.world.armies[army].target=Campaign::At(x,y); }
static std::unique_ptr<JourneySaveData> Scene(int actor=0,bool rear=false)
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=2;s->beforeBattle=j;s->realtimeMode=true;s->selected=actor;
    assert(j.Prepare(1,0)); // Explicit contact/terrain fixture, not a fabricated campaign victory.
    for(int y=1;y<StrategistJourney::Rows-1;++y) for(int x=1;x<StrategistJourney::Columns-1;++x) j.world.tiles[Campaign::At(x,y)].terrain=Terrain::Plain;
    Place(j,1,7,6);Place(j,actor==0 ? 2:0,8,9);Place(j,actor,rear ? 5:6,6);Place(j,3,6,rear ? 6:5);
    j.world.armies[3].troops=3000;j.world.armies[3].morale=actor==2 ? 60:90;j.world.armies[3].stance=battle::Stance::Balanced;j.world.armies[4].troops=0;
    j.chain.Reset(j.world);j.chain.spent[1]=true;j.RefreshIntelligence();return s; // Keep this existing group focused on single-support tactics.
}
static std::vector<JourneySupportAction> Read(const JourneySaveData& s)
{ return ReadJourneySupportActions(s.journey,s.realtime,s.selected); }
static bool Lure(const JourneySupportAction& c) { return c.plan.orders[0].tactic==ChainTactic::Lure; }
static void Same(const std::vector<JourneySupportAction>& a,const std::vector<JourneySupportAction>& b)
{
    assert(a.size()==b.size());for(std::size_t n=0;n<a.size();++n)
    {
        const auto& x=a[n];const auto& y=b[n];assert(x.goal==y.goal && x.guard==y.guard && x.actorTo==y.actorTo && x.enemyTo==y.enemyTo && x.enemyMoraleLoss==y.enemyMoraleLoss && x.threatsBefore==y.threatsBefore && x.threatsAfter==y.threatsAfter && x.clearsThreat==y.clearsThreat && x.supporter==y.supporter && x.guardActs==y.guardActs && x.followup==y.followup);
        assert(x.plan.target==y.plan.target && x.plan.damage==y.plan.damage && x.plan.moraleCost==y.plan.moraleCost && x.plan.friendlyLoss==y.plan.friendlyLoss && x.plan.disordered==y.plan.disordered && x.plan.pressured==y.plan.pressured);
        assert(x.plan.orders.size()==y.plan.orders.size());for(std::size_t k=0;k<x.plan.orders.size();++k) { const auto& p=x.plan.orders[k];const auto& q=y.plan.orders[k];assert(p.army==q.army && p.enemy==q.enemy && p.tactic==q.tactic && p.destination==q.destination); }
    }
}
int main()
{
    auto side=Scene();auto& j=side->journey;const auto options=Read(*side);assert(options.size()==2 && Lure(options[0]) && !Lure(options[1]));
    assert(options[0].threatsBefore==1 && options[0].threatsAfter==1 && !options[0].clearsThreat && options[0].enemyTo==j.world.armies[0].tile);
    const auto before=Bytes(*side);const auto context=JourneySupportActionContext(j,side->realtime,0);
    for(int n=0;n<10;++n) { Same(options,Read(*side));JourneySupportActionContext(j,side->realtime,0); }assert(Bytes(*side)==before);
    auto secret=std::make_unique<JourneySaveData>(*side);secret->journey.enemyPolicy=2;Place(secret->journey,3,13,11);++secret->journey.world.revision;
    assert(JourneySupportActionContext(secret->journey,secret->realtime,0)==context);Same(options,Read(*secret));
    assert(side->realtime.Queue(j,0,JourneyDirective::Hold));Same(options,Read(*side));
    side->realtime.Queue(j,0,JourneyDirective::March,j.goal);assert(Read(*side).empty());side->realtime.Queue(j,0,JourneyDirective::Attack,3);assert(Read(*side).empty());side->realtime.Cancel(0);
    j.assignments[0]={JourneyDuty::Hold,Campaign::At(6,6),false};assert(Read(*side).empty());j.assignments[0]={};
    j.chain.moved[0]=true;const auto moved=Read(*side);assert(!moved.empty() && std::none_of(moved.begin(),moved.end(),Lure));j.chain.moved[0]=false;
    j.chain.spent[0]=true;assert(Read(*side).empty());j.chain.spent[0]=false;j.world.armies[0].morale=14;assert(Read(*side).empty());j.world.armies[0].morale=90;
    j.world.armies[3].stance=battle::Stance::Guard;j.RefreshIntelligence();const auto guarded=Read(*side);assert(!guarded.empty() && std::none_of(guarded.begin(),guarded.end(),Lure));
    auto missing=Scene();missing->journey.world.armies[1].troops=0;assert(Read(*missing).empty());
    auto post=Scene();Place(post->journey,0,7,6);assert(Read(*post).empty());
    auto far=Scene();Place(far->journey,0,3,10);assert(Read(*far).empty());
    auto rear=Scene(0,true);const auto draw=Read(*rear);assert(draw.size()==2 && Lure(draw[0]) && draw[0].clearsThreat && draw[0].threatsBefore==1 && draw[0].threatsAfter==0);
    auto& r=rear->journey;const auto guard=r.world.armies[1];const auto camp=r.world.cities[0];const int troops=r.world.armies[0].troops,morale=r.world.armies[0].morale;
    const auto trust=r.officerTrust;const auto turn=r.turn;const auto result=r.Execute(draw[0].plan.orders);
    assert(result.completed==1 && r.world.armies[0].troops==troops-60 && r.world.armies[0].morale==morale-10 && r.chain.moved[0] && r.chain.spent[0]);
    assert(r.world.armies[0].tile==draw[0].actorTo && r.world.armies[3].tile==draw[0].enemyTo && r.world.armies[1].tile==guard.tile && r.world.armies[1].troops==guard.troops && r.world.armies[1].morale==guard.morale);
    assert(r.turn==turn && r.heldTurns==0 && r.officerTrust==trust && r.world.cities[0].food==camp.food && r.world.cities[0].gold==camp.gold);
    auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Bytes(*rear),*loaded) && loaded->journey.world.armies[0].tile==draw[0].actorTo && loaded->journey.chain.spent[0]);
    auto push=Scene(2);auto attack=Read(*push);assert(!attack.empty() && attack[0].plan.orders[0].tactic==ChainTactic::Breakthrough && attack[0].enemyTo!=push->journey.world.armies[3].tile && attack[0].clearsThreat);
    const auto prediction=attack[0];auto hidden=std::make_unique<JourneySaveData>(*push);hidden->journey.world.armies[4].troops=1000;hidden->journey.world.armies[4].tile=hidden->journey.world.armies[4].target=prediction.enemyTo;
    Same(attack,Read(*hidden));assert(JourneySupportActionContext(push->journey,push->realtime,2)==JourneySupportActionContext(hidden->journey,hidden->realtime,2));
    const int actorMorale=hidden->journey.world.armies[2].morale;assert(hidden->journey.Execute(prediction.plan.orders).completed==0 && !hidden->journey.chain.spent[2] && hidden->journey.world.armies[2].morale==actorMorale);
    const auto guardTile=push->journey.world.armies[1].tile;assert(push->journey.Execute(prediction.plan.orders).completed==1 && push->journey.world.armies[3].tile==prediction.enemyTo && push->journey.world.armies[2].tile==prediction.actorTo && push->journey.world.armies[1].tile==guardTile);
    auto blocked=Scene(2);blocked->journey.world.tiles[prediction.enemyTo].terrain=Terrain::River;const auto alternatives=Read(*blocked);assert(!alternatives.empty());
    for(const auto& c:alternatives) assert(c.plan.orders[0].tactic!=ChainTactic::Breakthrough);
    auto firm=Scene(2);firm->journey.world.armies[3].morale=90;firm->journey.RefreshIntelligence();for(const auto& c:Read(*firm)) assert(c.enemyTo==firm->journey.world.armies[3].tile);
    auto both=Scene();Place(both->journey,0,7,7);Place(both->journey,3,6,7);Place(both->journey,4,8,8);both->journey.world.armies[4].troops=2000;both->journey.RefreshIntelligence();
    const auto comparison=Read(*both);assert(!comparison.empty());for(const auto& c:comparison) assert(c.goal==comparison[0].goal && c.guard==comparison[0].guard && c.plan.target==comparison[0].plan.target);
    std::cout<<"Support action PASS: truthful comparable ordinary tactics, sideways/rear lure, actual and blocked push, public-only immutable reads, roles/orders/budgets, guard safety and save/load\n";
}
