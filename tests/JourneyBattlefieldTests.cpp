#include "../NewGame/JourneyBattlefield.hpp"
#include "../NewGame/JourneyRealtime.hpp"
#include "../NewGame/JourneySave.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <queue>
using namespace frontline;

static std::unique_ptr<StrategistJourney> Battle(int mission, int policy=0)
{
    auto j=std::make_unique<StrategistJourney>();
    j->Reset();j->Meet();j->mission=mission;assert(j->Prepare(0,policy));return j;
}
static bool Connected(const Campaign& world,int from,int target)
{
    auto empty=std::make_unique<Campaign>(world);empty->armies.clear();
    std::array<bool,TileCount> seen{};std::queue<int> open;open.push(from);seen[from]=true;
    while(!open.empty())
    {
        const int at=open.front();open.pop();if(at==target) return true;
        for(int next:world.MapNeighbors(at))
            if(StrategistJourney::OnMap(next) && !seen[next] && TacticChain::Free(*empty,next))
            { seen[next]=true;open.push(next); }
    }
    return false;
}
int main()
{
    for(int mission=0;mission<3;++mission)
    {
        for(auto row:BattlefieldDefinition(mission).rows) assert(row.size()==14);
        for(int policy=0;policy<3;++policy)
        {
            auto j=Battle(mission,policy);
            for(const auto& a:j->world.armies)
                assert(j->world.tiles[a.tile].terrain!=Terrain::River && j->world.tiles[a.tile].terrain!=Terrain::Sea);
            for(const auto& city:j->world.cities)
                assert(j->world.tiles[city.tile].terrain!=Terrain::River && j->world.tiles[city.tile].terrain!=Terrain::Sea);
            assert(Connected(j->world,j->world.armies[0].tile,j->goal));
            if(j->Definition().secondGoal>=0) assert(Connected(j->world,j->goal,j->Definition().secondGoal));
            if(j->HasConvoy())
                for(std::size_t i=0;i<j->ConvoyPath().size();++i)
                {
                    const int p=j->ConvoyPath()[i];
                    assert(j->world.tiles[p].terrain!=Terrain::River);
                    if(i) assert(j->world.MapDistance(j->ConvoyPath()[i-1],p)==1);
                }
            auto save=std::make_unique<JourneySaveData>();save->journey=*j;
            auto council=std::make_unique<StrategistJourney>();council->Reset();council->Meet();council->mission=mission;
            save->beforeBattle=*council;
            std::vector<std::uint8_t> bytes;assert(EncodeJourneySave(*save,bytes));
            auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(bytes,*loaded));
            for(int y=1;y<=12;++y) for(int x=1;x<=14;++x)
                assert(loaded->journey.world.tiles[Campaign::At(x,y)].terrain==j->world.tiles[Campaign::At(x,y)].terrain);
            assert(loaded->journey.world.armies[3].tile==j->world.armies[3].tile && loaded->journey.goal==j->goal);
        }
    }
    for(int a=0;a<3;++a) for(int b=a+1;b<3;++b)
    {
        int different=0;
        for(int y=1;y<=12;++y) for(int x=1;x<=14;++x)
            different+=BattlefieldTerrain(a,x,y)!=BattlefieldTerrain(b,x,y);
        assert(different>=40); // Distinct boards, rather than a color swap.
    }
    // Both crossings must be real alternatives. Removing both cuts off the
    // opposite bank; the map boundary must not permit walking around the river.
    for(int mission:{1,2})
    {
        auto j=Battle(mission);
        const int first=mission==1 ? Campaign::At(11,8):Campaign::At(7,7);
        const int second=mission==1 ? Campaign::At(11,11):Campaign::At(11,7);
        const int from=mission==1 ? Campaign::At(9,9):j->goal;
        const int to=mission==1 ? Campaign::At(12,9):j->Definition().secondGoal;
        assert(Connected(j->world,from,to));
        j->world.tiles[first].terrain=Terrain::River;assert(Connected(j->world,from,to));
        j->world.tiles[first].terrain=Terrain::Bridge;j->world.tiles[second].terrain=Terrain::River;
        assert(Connected(j->world,from,to));
        j->world.tiles[first].terrain=Terrain::River;assert(!Connected(j->world,from,to));
    }
    {
        auto j=Battle(1);const int from=Campaign::At(10,10);
        j->world.armies[0].tile=j->world.armies[0].target=from;j->RefreshIntelligence();
        assert(j->world.MapDistance(from,Campaign::At(11,10))==1);
        assert(!j->Move(0,Campaign::At(11,10)) && j->world.armies[0].tile==from && !j->chain.moved[0]);
        // Long orders route through a real bridge too, without mutating state.
        j->world.armies[3].troops=j->world.armies[4].troops=0;j->RefreshIntelligence();
        JourneyRealtime rt;assert(rt.Queue(*j,0,JourneyDirective::March,Campaign::At(12,10)));
        const auto route=rt.Route(*j,0);assert(!route.empty() && route.back()==Campaign::At(12,10));
        bool crossed=false;
        for(int tile:route)
        { assert(j->world.tiles[tile].terrain!=Terrain::River);crossed|=j->world.tiles[tile].terrain==Terrain::Bridge; }
        assert(crossed && j->world.armies[0].tile==from && j->turn==1);
    }
    {
        auto j=Battle(1);
        assert(j->world.armies[3].tile==j->goal); // Bridgehead holds the delivery site.
        for(int friendly=0;friendly<3;++friendly)
            assert(j->world.MapDistance(j->world.armies[friendly].tile,j->world.armies[3].tile)>1);
        const auto plans=j->ReadEnemyPlans();
        for(const auto& plan:plans) assert(j->world.tiles[plan.next].terrain!=Terrain::River);
    }
    {
        // A fully observed planner fixture: the enemy must also use a crossing,
        // instead of taking a straight step through an impassable river.
        auto j=Battle(1);
        for(int id:{1,2,3,5}) j->world.armies[id].troops=0;
        j->world.armies[0].tile=Campaign::At(10,8);
        bool crossed=false;
        for(int step=0;step<4;++step)
        {
            const auto plans=PlanJourneyEnemies(j->world,j->chain,{JourneyObjective::Escort,j->goal,-1,0,false});
            assert(plans.size()==1 && plans[0].army==4);
            const auto& plan=plans[0];
            assert(j->world.MapDistance(plan.from,plan.next)<=1 && j->world.tiles[plan.next].terrain!=Terrain::River);
            crossed|=j->world.tiles[plan.next].terrain==Terrain::Bridge;
            j->world.armies[4].tile=plan.next;
        }
        assert(crossed);
    }
    std::cout<<"Three authored battlefields: distinct terrain, valid deployments, two genuine crossings, convoy continuity and real order routing passed\n";
}
