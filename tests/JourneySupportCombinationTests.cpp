#include "../NewGame/JourneySupportActions.hpp"
#include "../NewGame/JourneySave.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
static void Place(StrategistJourney& j,int a,int x,int y)
{ j.world.armies[a].tile=j.world.armies[a].target=Campaign::At(x,y); }
static std::unique_ptr<JourneySaveData> Scene()
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=2;s->beforeBattle=j;s->realtimeMode=true;
    assert(j.Prepare(1,0)); // Isolated contact/terrain fixture, not a campaign victory.
    for(int y=1;y<StrategistJourney::Rows-1;++y) for(int x=1;x<StrategistJourney::Columns-1;++x) j.world.tiles[Campaign::At(x,y)].terrain=Terrain::Plain;
    Place(j,0,6,6);Place(j,1,7,6);Place(j,2,8,9);Place(j,3,6,5);j.world.armies[3].troops=3000;j.world.armies[3].morale=90;j.world.armies[3].stance=battle::Stance::Balanced;j.world.armies[4].troops=0;
    j.assignments[1]={JourneyDuty::Hold,j.goal,true};j.assignments[2]={JourneyDuty::Hold,j.Definition().secondGoal,true};j.chain.Reset(j.world);j.RefreshIntelligence();
    for(int a=0;a<3;++a) s->realtime.Queue(j,a,JourneyDirective::Hold);return s;
}
static std::vector<JourneySupportAction> Read(const JourneySaveData& s)
{ return ReadJourneySupportActions(s.journey,s.realtime,0); }
static const JourneySupportAction& Pair(const std::vector<JourneySupportAction>& c)
{ const auto p=std::find_if(c.begin(),c.end(),[](const auto& x) { return x.plan.orders.size()==2; });assert(p!=c.end());return *p; }
static bool HasGuard(const std::vector<JourneySupportAction>& c)
{ return std::any_of(c.begin(),c.end(),[](const auto& x) { return x.guardActs; }); }
int main()
{
    auto s=Scene();auto& j=s->journey;const auto shown=Read(*s);assert(shown.size()==2 && shown[0].plan.orders.size()==1 && !shown[0].guardActs);
    const auto plan=Pair(shown);assert(plan.supporter==0 && plan.guard==1 && plan.guardActs && !plan.followup && plan.goal==shown[0].goal && plan.plan.target==shown[0].plan.target);
    const auto saved=Bytes(*s);for(int n=0;n<10;++n) Read(*s);assert(Bytes(*s)==saved);
    const auto proof=j.Preview(plan.plan.orders);assert(proof.completed==2 && proof.events[1].combo==2 && proof.events[1].comboPercent==125);
    const auto orders=s->realtime.orders;const auto camp=j.world.cities[0];const auto trust=j.officerTrust;const int m0=j.world.armies[0].morale,m1=j.world.armies[1].morale,turn=j.turn;
    const auto actual=j.Execute(plan.plan.orders);assert(actual.completed==2 && actual.events[1].comboPercent==125 && j.chain.spent[0] && j.chain.spent[1] && !j.chain.spent[2]);
    assert(j.world.armies[1].tile==j.goal && j.world.armies[2].tile==j.Definition().secondGoal && j.world.armies[0].tile==plan.actorTo && j.world.armies[3].tile==plan.enemyTo);
    assert(j.world.armies[0].morale==m0-TacticChain::Definition(plan.plan.orders[0].tactic).moraleCost && j.world.armies[1].morale==m1-TacticChain::Definition(plan.plan.orders[1].tactic).moraleCost);
    assert(!j.Hold(1) && j.turn==turn && j.heldTurns==0 && j.world.cities[0].food==camp.food && j.officerTrust==trust);
    for(int a=0;a<3;++a) assert(SameJourneyOrder(orders[a],s->realtime.orders[a]));
    auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Bytes(*s),*loaded) && loaded->journey.chain.spent[1] && loaded->journey.world.armies[1].tile==j.goal);
    auto depleted=Scene();depleted->journey.world.armies[1].morale=34;assert(!HasGuard(Read(*depleted)));depleted->journey.world.armies[1].morale=35;assert(HasGuard(Read(*depleted)));
    auto enough=Scene();enough->journey.world.armies[3].troops=180;enough->journey.RefreshIntelligence();assert(!HasGuard(Read(*enough)));
    for(auto duty:{JourneyDirective::March,JourneyDirective::Attack}) { auto busy=Scene();assert(busy->realtime.Queue(busy->journey,1,duty,duty==JourneyDirective::March ? Campaign::At(7,5):3));assert(!HasGuard(Read(*busy))); }
    auto otherDuty=Scene();otherDuty->journey.assignments[1]={JourneyDuty::Hold,Campaign::At(8,9),false};assert(!HasGuard(Read(*otherDuty)));
    auto braced=Scene();assert(braced->journey.Hold(1) && !HasGuard(Read(*braced)));
    auto mobile=Scene();std::swap(mobile->journey.world.armies[1].general,mobile->journey.world.armies[2].general);mobile->journey.world.armies[3].morale=50;mobile->journey.RefreshIntelligence();
    for(const auto& c:Read(*mobile)) for(const auto& o:c.plan.orders) assert(o.army!=1 || o.tactic!=ChainTactic::Breakthrough);
    auto wing=Scene();Place(wing->journey,0,8,8);Place(wing->journey,3,7,7);wing->journey.RefreshIntelligence();const auto safeWing=Read(*wing);assert(!safeWing.empty());for(const auto& c:safeWing) assert(c.plan.orders[0].tactic!=ChainTactic::Lure);
    auto partial=Scene();partial->journey.world.armies[3].stance=battle::Stance::Guard;partial->journey.RefreshIntelligence();const auto queued=Pair(Read(*partial));
    const auto key=JourneySupportActionContext(partial->journey,partial->realtime,0);partial->journey.world.armies[3].troops=1;
    assert(JourneySupportActionContext(partial->journey,partial->realtime,0)==key && Pair(Read(*partial)).plan.damage==queued.plan.damage);
    const int guardMorale=partial->journey.world.armies[1].morale;const auto stopped=partial->journey.Execute(queued.plan.orders);
    assert(stopped.completed==1 && stopped.events.size()==2 && !stopped.events[1].success && partial->journey.chain.spent[0] && !partial->journey.chain.spent[1] && partial->journey.world.armies[1].morale==guardMorale);
    auto follow=Scene();auto& f=follow->journey;assert(f.Execute({{0,3,ChainTactic::Feint,-1}}).completed==1 && f.chain.pressured[3]);
    const auto afterSetup=Bytes(*follow);const auto followKey=JourneySupportActionContext(f,follow->realtime,0);const auto next=Read(*follow);
    assert(!next.empty() && next[0].followup && next[0].guardActs && next[0].supporter==0 && next[0].plan.orders.size()==1 && next[0].plan.orders[0].army==1 && next[0].actorTo==f.goal);
    const auto prediction=f.Preview(next[0].plan.orders);assert(prediction.completed==1 && prediction.events[0].combo==1 && prediction.events[0].comboPercent==100 && prediction.events[0].setupPercent==125);
    assert(Bytes(*follow)==afterSetup && DecodeJourneySave(afterSetup,*loaded) && !Read(*loaded).empty());
    auto unknown=std::make_unique<JourneySaveData>(*follow);unknown->journey.lastChain={};assert(Read(*unknown).empty() && JourneySupportActionContext(unknown->journey,unknown->realtime,0)!=followKey);
    auto wrong=std::make_unique<JourneySaveData>(*follow);wrong->journey.lastChain.events[0].army=2;assert(Read(*wrong).empty());
    auto carried=std::make_unique<JourneySaveData>(*follow);carried->journey.lastChain.events[0].tactic=ChainTactic::Breakthrough;assert(Read(*carried).empty());
    const auto end=f.Execute(next[0].plan.orders);assert(end.completed==1 && end.events[0].comboPercent==100 && f.world.armies[1].tile==f.goal && f.chain.spent[1] && Read(*follow).empty());
    std::cout<<"Support combination PASS: comparable solo/pair, real 125 percent chain vs proven 100 percent follow-up, paid guard budget, stationary posts, morale/order/other-wing protection, private partial stop and saved provenance\n";
}
