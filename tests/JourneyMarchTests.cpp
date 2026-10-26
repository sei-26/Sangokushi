#include "../NewGame/JourneyBattleWatch.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::unique_ptr<StrategistJourney> MarchMission()
{
    auto j=std::make_unique<StrategistJourney>();j->Reset();j->Meet();j->mission=1;assert(j->Prepare(0,1));
    j->world.armies[3].troops=j->world.armies[4].troops=0;j->world.armies[0].tile=Campaign::At(2,4);
    j->chain.BeginTurn(j->world);j->RefreshIntelligence();return j;
}
int main()
{
    {
        auto j=MarchMission();JourneyRealtime rt;const int revision=j->world.revision,food=j->world.armies[0].food;
        for (int tile : {Campaign::At(3,4),Campaign::At(3,5),Campaign::At(2,5),Campaign::At(2,4)}) assert(rt.AppendWaypoint(*j,0,tile));
        const auto original=rt.orders[0];
        assert(!rt.AppendWaypoint(*j,0,Campaign::At(4,4)) && SameJourneyOrder(original,rt.orders[0]));
        assert(!rt.AppendWaypoint(*j,0,Campaign::At(2,4)) && !rt.AppendWaypoint(*j,0,Campaign::At(2,10)));
        assert(rt.SetArrival(*j,0,JourneyArrival::Hold));assert(!rt.SetArrival(*j,0,JourneyArrival::Ambush));
        assert(rt.UndoWaypoint(0) && rt.orders[0].waypoints.size()==2 && rt.orders[0].arrival==JourneyArrival::Wait);
        assert(j->world.revision==revision && j->world.armies[0].food==food && j->turn==1);
        assert(!rt.AppendWaypoint(*j,-1,Campaign::At(3,4)));
    }
    {
        auto j=MarchMission();JourneyRealtime rt;const std::vector<int> points{Campaign::At(3,4),Campaign::At(3,5),Campaign::At(2,5)};
        for (int tile : points) assert(rt.AppendWaypoint(*j,0,tile));
        const auto plan=rt.MarchPlan(*j,0);assert(plan.complete && plan.points==points && plan.route.front()==Campaign::At(2,4) && plan.route.back()==points.back());
        for (int point : points)
        {
            assert(rt.orders[0].destination==point);
            while (rt.orders[0].kind==JourneyDirective::March && rt.orders[0].destination==point)
            { const int old=j->world.armies[0].tile;assert(rt.Resolve(*j));assert(j->world.MapDistance(old,j->world.armies[0].tile)<=1); }
            assert(j->world.armies[0].tile==point);
        }
        assert(rt.orders[0].kind==JourneyDirective::None && j->turn==4);
    }
    {
        auto j=MarchMission();JourneyRealtime rt;
        assert(rt.AppendWaypoint(*j,0,Campaign::At(3,4)));assert(rt.AppendWaypoint(*j,0,Campaign::At(2,4)));
        const auto plan=rt.MarchPlan(*j,0);assert(plan.complete && plan.route.size()==3 && plan.route.back()==j->world.armies[0].tile);
        j->world.armies[4].tile=Campaign::At(3,4);j->world.armies[4].troops=3000;
        assert(rt.MarchPlan(*j,0).route==plan.route); // Unknown occupancy cannot alter a plan.
    }
    {
        auto j=MarchMission();JourneyRealtime rt;
        assert(rt.AppendWaypoint(*j,0,Campaign::At(3,4)));assert(rt.AppendWaypoint(*j,0,j->world.armies[1].tile));
        const auto read=ReadJourneyOrder(*j,rt,0);
        assert(!read.complete && read.status==JourneyRouteStatus::Moving && read.route.back()==Campaign::At(3,4));
        assert(rt.Resolve(*j) && j->world.armies[0].tile==Campaign::At(3,4)); // A blocked later leg does not cancel the reachable first leg.
    }
    {
        auto j=MarchMission();JourneyRealtime rt;
        assert(rt.AppendWaypoint(*j,0,Campaign::At(3,4)) && rt.SetArrival(*j,0,JourneyArrival::Hold));
        assert(rt.Resolve(*j) && rt.orders[0].kind==JourneyDirective::Hold);
        assert(rt.Queue(*j,0,JourneyDirective::Attack,3)==false);
        rt.Cancel(0);assert(rt.orders[0].waypoints.empty() && rt.orders[0].arrival==JourneyArrival::Wait);
        j->world.armies[1].tile=Campaign::At(6,7);j->RefreshIntelligence();
        assert(rt.AppendWaypoint(*j,1,Campaign::At(6,8)) && rt.SetArrival(*j,1,JourneyArrival::Ambush));
        assert(!rt.AppendWaypoint(*j,1,Campaign::At(3,3))); // An ambush destination must stay in the forest.
        assert(rt.Resolve(*j) && rt.orders[1].kind==JourneyDirective::Ambush && !j->chain.concealed[1]);
        assert(rt.Resolve(*j) && j->chain.concealed[1]); // No free preparation on the movement cycle.
        rt.Reset();assert(rt.orders[1].kind==JourneyDirective::None);
    }
    std::cout<<"Journey march: PASS (ordered checkpoints, limit, non-consuming edits, loop, privacy, partial path, arrival costs, cleanup)\n";
}
