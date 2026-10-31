#include "../NewGame/JourneySave.hpp"
#include <cassert>
#include <algorithm>
#include <iostream>
#include <memory>
using namespace frontline;
using S=JourneyApproachState;
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
static std::unique_ptr<JourneySaveData> Corridor(bool blocker)
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=2;s->beforeBattle=j;
    assert(j.Prepare(1,1));
    // Authored test corridor isolates occupancy; this is not a claimed campaign playthrough.
    for(auto& tile:j.world.tiles) tile.terrain=Terrain::Sea;
    for(int x=4;x<=8;++x) j.world.tiles[Campaign::At(x,9)].terrain=Terrain::Plain;
    j.world.armies[0].tile=j.world.armies[0].target=Campaign::At(4,9);
    j.world.armies[1].tile=j.world.armies[1].target=Campaign::At(6,9);if(!blocker) j.world.armies[1].troops=0;
    j.world.armies[2].tile=j.world.armies[2].target=Campaign::At(8,9);
    j.world.armies[3].troops=j.world.armies[4].troops=0;j.RefreshIntelligence();return s;
}
static void Same(const JourneyReliefApproach& a,const JourneyReliefApproach& b)
{ assert(a.state==b.state && a.staging==b.staging && a.blocker==b.blocker && a.blockedTile==b.blockedTile && a.route==b.route); }
int main()
{
    const int source=Campaign::At(8,9);
    auto clear=Corridor(false);auto& j=clear->journey;auto& clock=clear->realtime;
    const auto before=Bytes(*clear);const auto plan=clock.ReliefApproach(j,0,source);
    assert(plan.state==S::Clear && plan.staging==Campaign::At(7,9) && plan.route.size()==4 && plan.blocker==-1);
    assert(std::find(plan.route.begin(),plan.route.end(),source)==plan.route.end());
    for(int i=0;i<20;++i) Same(plan,clock.ReliefApproach(j,0,source));assert(Bytes(*clear)==before);
    assert(clock.Queue(j,0,JourneyDirective::March,plan.staging));assert(clock.Route(j,0)==plan.route);
    auto restored=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Bytes(*clear),*restored));Same(plan,restored->realtime.ReliefApproach(restored->journey,0,source));
    for(int i=0;i<3;++i) assert(clock.Resolve(j));
    assert(j.world.armies[0].tile==plan.staging && j.world.armies[2].tile==source);
    const auto adjacent=clock.ReliefApproach(j,0,source);assert(adjacent.state==S::Adjacent && adjacent.route==std::vector<int>{plan.staging});
    auto blocked=Corridor(true);auto& b=blocked->journey;const auto saved=Bytes(*blocked);
    const auto wait=blocked->realtime.ReliefApproach(b,0,source);
    assert(wait.state==S::AllyBlocked && wait.blocker==1 && wait.blockedTile==Campaign::At(6,9) && wait.route.size()==4);
    assert(Bytes(*blocked)==saved && b.world.armies[1].troops>0);
    blocked->realtime.Queue(b,0,JourneyDirective::March,wait.staging);assert(blocked->realtime.Route(b,0).empty());
    assert(blocked->realtime.Resolve(b) && b.world.armies[0].tile==Campaign::At(4,9));
    b.world.tiles[Campaign::At(6,8)].terrain=Terrain::Plain;b.RefreshIntelligence();
    assert(b.Move(1,Campaign::At(6,8)));
    const auto opened=blocked->realtime.ReliefApproach(b,0,source);assert(opened.state==S::Clear && blocked->realtime.Route(b,0)==opened.route);
    assert(blocked->realtime.Resolve(b) && b.world.armies[0].tile==opened.route[1]);
    // A usable detour is a legal route, not a false report that an ally must be moved.
    auto detour=Corridor(true);
    for(int x=4;x<=7;++x) detour->journey.world.tiles[Campaign::At(x,8)].terrain=Terrain::Plain;
    detour->journey.RefreshIntelligence();const auto around=detour->realtime.ReliefApproach(detour->journey,0,source);
    assert(around.state==S::Clear && around.blocker==-1 && std::find(around.route.begin(),around.route.end(),Campaign::At(6,9))==around.route.end());
    detour->realtime.Queue(detour->journey,0,JourneyDirective::March,around.staging);assert(detour->realtime.Route(detour->journey,0)==around.route);
    // Only current contacts matter. A hidden actual obstruction may still stop the real move.
    auto fog=Corridor(false);const auto known=fog->realtime.ReliefApproach(fog->journey,0,source);
    fog->journey.world.armies[3].tile=fog->journey.world.armies[3].target=Campaign::At(5,9);fog->journey.world.armies[3].troops=1200;
    const auto unseen=Bytes(*fog);Same(known,fog->realtime.ReliefApproach(fog->journey,0,source));assert(Bytes(*fog)==unseen);
    assert(!fog->journey.Move(0,Campaign::At(5,9)));Same(known,fog->realtime.ReliefApproach(fog->journey,0,source));
    fog->journey.RefreshIntelligence();const auto contact=fog->realtime.ReliefApproach(fog->journey,0,source);
    assert(contact.state==S::NoPath && contact.route.empty() && contact.blocker==-1);
    auto terrain=Corridor(false);terrain->journey.world.tiles[Campaign::At(5,9)].terrain=Terrain::River;
    assert(terrain->realtime.ReliefApproach(terrain->journey,0,source).state==S::NoPath);
    assert(terrain->realtime.ReliefApproach(terrain->journey,-1,source).state==S::Unavailable);
    assert(terrain->realtime.ReliefApproach(terrain->journey,2,source).state==S::Unavailable);
    assert(terrain->realtime.ReliefApproach(terrain->journey,0,-1).state==S::Unavailable);
    terrain->journey.Withdraw();assert(terrain->realtime.ReliefApproach(terrain->journey,0,source).state==S::Unavailable);
    std::cout<<"Relief approach PASS: shared routing, legal detours, exact friendly obstruction, actual clearing/movement, immutable observation, real hidden collision and saved pending movement\n";
}
