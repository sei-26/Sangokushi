#include "JourneyContributionPlayer.hpp"
#include "../NewGame/JourneySupport.hpp"
#include <cassert>
#include <iostream>
using namespace frontline;
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
static std::unique_ptr<JourneySaveData> Corridor(bool blocker)
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=2;assert(j.Prepare(1,0));
    // Authored isolated fixture; not a claim that campaign enemies have been defeated.
    for(auto& tile:j.world.tiles) tile.terrain=Terrain::Sea;
    for(int x=4;x<=8;++x) j.world.tiles[Campaign::At(x,8)].terrain=Terrain::Plain;
    j.goal=Campaign::At(8,8);j.world.armies[0].tile=j.world.armies[0].target=Campaign::At(4,8);
    j.world.armies[1].tile=j.world.armies[1].target=j.goal;
    j.world.armies[2].tile=j.world.armies[2].target=Campaign::At(6,8);if(!blocker) j.world.armies[2].troops=0;
    j.world.armies[3].troops=j.world.armies[4].troops=0;j.RefreshIntelligence();return s;
}
static JourneySupportChoice North(const JourneySaveData& s)
{ const auto list=ReadJourneySupport(s.journey,s.realtime,0);for(const auto& c:list) if(c.goal==s.journey.goal) return c;assert(false);return {}; }
int main()
{
    auto actual=contributionqa::Play(0);assert(actual && actual->journey.Continue());auto& a=actual->journey;
    assert(a.ApplyPlanExample());actual->beforeBattle=a;actual->realtimeMode=true;
    assert(a.Prepare(0,0));actual->realtime.ApplyAssignments(a);
    const auto before=Bytes(*actual);const auto choices=ReadJourneySupport(a,actual->realtime,0);assert(choices.size()==2);
    for(const auto& c:choices) { assert(c.cover && c.guard!=0 && c.available);assert(std::find(c.route.begin(),c.route.end(),c.goal)==c.route.end()); }
    for(int n=0;n<12;++n) ReadJourneySupport(a,actual->realtime,0);assert(Bytes(*actual)==before);
    auto privateChange=std::make_unique<JourneySaveData>(*actual);privateChange->journey.enemyPolicy=2;
    privateChange->journey.world.armies[3].tile=Campaign::At(15,12);++privateChange->journey.world.revision;
    assert(JourneySupportContext(a,actual->realtime,0)==JourneySupportContext(privateChange->journey,privateChange->realtime,0));
    const auto hidden=ReadJourneySupport(privateChange->journey,privateChange->realtime,0);assert(hidden[0].route==choices[0].route && hidden[0].enemies==choices[0].enemies);
    // Explicit public-contact fixture: the northern visible threat changes the recommendation and invalidates its old estimate.
    auto publicChange=std::make_unique<JourneySaveData>(*actual);auto& intel=publicChange->journey.intelligence[0];intel.contacts.clear();
    ArmyContact seen;seen.id=3;seen.current=true;seen.observed=a.world.armies[3];seen.observed.troops=1200;seen.observed.tile=Campaign::At(7,5);intel.contacts.push_back(seen);
    const auto threatened=ReadJourneySupport(publicChange->journey,publicChange->realtime,0);
    assert(threatened.front().goal==a.goal && threatened.front().enemies==1 && threatened.front().adjacentEnemies==1);
    const auto priorNorth=std::find_if(choices.begin(),choices.end(),[&](const auto& c) { return c.goal==a.goal; });
    assert(priorNorth!=choices.end() && !QueueJourneySupport(publicChange->journey,publicChange->realtime,*priorNorth));
    const auto guardOrder=actual->realtime.orders[1];const auto camp=a.world.cities[0];const int start=a.world.armies[0].tile,turn=a.turn;
    assert(QueueJourneySupport(a,actual->realtime,choices[0]) && actual->realtime.paused && SameJourneyOrder(guardOrder,actual->realtime.orders[1]));
    assert(a.world.armies[0].tile==start && a.turn==turn && a.world.cities[0].food==camp.food && !a.chain.moved[0]);
    assert(actual->realtime.Route(a,0)==choices[0].route && actual->realtime.orders[0].arrival==JourneyArrival::Hold);
    auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Bytes(*actual),*loaded) && SameJourneyOrder(loaded->realtime.orders[0],actual->realtime.orders[0]));
    assert(!QueueJourneySupport(a,actual->realtime,choices[0]) && ReadJourneySupport(a,actual->realtime,1).empty());
    assert(actual->realtime.Resolve(a) && a.turn==turn+1 && a.world.MapDistance(start,a.world.armies[0].tile)<=1);
    auto s=Corridor(false);auto& j=s->journey;auto& clock=s->realtime;auto c=North(*s);
    assert(c.available && c.cover && c.guard==1 && c.steps==3 && c.arrival==3 && c.brace==4);
    j.chain.moved[0]=true;const auto delayed=North(*s);assert(delayed.arrival==4 && delayed.brace==5 && !QueueJourneySupport(j,clock,c));
    assert(QueueJourneySupport(j,clock,delayed));const int from=j.world.armies[0].tile;assert(clock.Resolve(j) && j.world.armies[0].tile==from);
    auto holds=[&]() { return std::count_if(j.reports.begin(),j.reports.end(),[](const auto& r) { return r.text.find(U"劉備隊が固守")!=std::u32string::npos; }); };
    for(int n=0;n<3;++n) assert(clock.Resolve(j));assert(j.world.armies[0].tile==delayed.destination && holds()==0);
    assert(clock.Resolve(j) && holds()==1 && j.world.armies[1].tile==j.goal); // Guard flags renew after the resolved cycle; the actual report records the action.
    auto blocked=Corridor(true);auto b=North(*blocked);assert(!b.available && b.blocker==2 && !QueueJourneySupport(blocked->journey,blocked->realtime,b));
    auto fog=Corridor(false);const auto key=JourneySupportContext(fog->journey,fog->realtime,0);const auto old=North(*fog);
    fog->journey.world.armies[3].tile=fog->journey.world.armies[3].target=Campaign::At(5,8);fog->journey.world.armies[3].troops=1200;
    assert(JourneySupportContext(fog->journey,fog->realtime,0)==key && North(*fog).route==old.route);
    assert(QueueJourneySupport(fog->journey,fog->realtime,old));assert(fog->realtime.Resolve(fog->journey) && fog->journey.world.armies[0].tile==Campaign::At(4,8));
    fog->realtime.Cancel(0);fog->journey.RefreshIntelligence();assert(!North(*fog).available && JourneySupportContext(fog->journey,fog->realtime,0)!=key);
    auto empty=Corridor(false);empty->journey.world.armies[1].troops=0;const auto take=North(*empty);assert(take.available && !take.cover && take.destination==empty->journey.goal);
    empty->journey.chain.concealed[0]=true;empty->journey.intelligence[0].visible.fill(false);
    const auto uncertain=North(*empty);assert(uncertain.uncertain && uncertain.breaksConcealment);
    empty->journey.assignments[0].duty=JourneyDuty::Hold;assert(ReadJourneySupport(empty->journey,empty->realtime,0).empty());
    std::cout<<"Support PASS: actual cooperation council, truthful public routes, immutable reads, ordinary paid movement/hold budgets, save/load, occupied-post safety, blockers and hidden collision\n";
}
