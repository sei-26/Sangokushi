#include "JourneyRealtime.hpp"
#include "JourneyMarchBlocker.hpp"
#include <memory>
#include <queue>

namespace frontline
{
    namespace
    {
        bool Walkable(const Campaign& world,int tile)
        {
            return StrategistJourney::OnMap(tile) && tile%Width>0 && tile/Width>0 &&
                tile%Width<StrategistJourney::Columns-1 && tile/Width<StrategistJourney::Rows-1 &&
                world.tiles[tile].terrain!=Terrain::River && world.tiles[tile].terrain!=Terrain::Sea && world.CityAt(tile)<0;
        }
        std::unique_ptr<Campaign> PlayerView(const StrategistJourney& journey)
        {
            auto seen=std::make_unique<Campaign>(journey.world);
            for (auto& unit : seen->armies) if (unit.faction!=0) unit.troops=0;
            for (const auto& contact : journey.intelligence[0].contacts)
                if (contact.current && contact.id>=0 && contact.id<static_cast<int>(seen->armies.size()))
                    seen->armies[contact.id]=contact.observed;
            return seen;
        }
        std::vector<int> Path(const Campaign& seen,int start,int goal)
        {
            std::array<int,TileCount> previous;previous.fill(-1);
            std::queue<int> frontier;frontier.push(start);previous[start]=start;
            while (!frontier.empty() && previous[goal]<0)
            {
                const int from=frontier.front();frontier.pop();
                for (int next : seen.MapNeighbors(from))
                    if (Walkable(seen,next) && previous[next]<0 && TacticChain::Free(seen,next))
                    { previous[next]=from;frontier.push(next); }
            }
            if (previous[goal]<0) return {};
            std::vector<int> result;
            for (int tile=goal;;tile=previous[tile]) { result.push_back(tile);if (tile==start) break; }
            std::reverse(result.begin(),result.end());return result;
        }
    }
    bool JourneyRealtime::Ground(const Campaign& world,int tile)
    { return Walkable(world,tile); }
    JourneyMarchBlocker ReadJourneyMarchBlocker(const StrategistJourney& journey,const JourneyRealtime& clock,int army)
    {
        if(journey.stage!=JourneyStage::Battle || army<0 || army>=3 || !TacticChain::Living(journey.world,army)) return {};
        const auto& order=clock.orders[army];
        if(order.kind!=JourneyDirective::March || !Walkable(journey.world,order.destination)) return {};
        auto view=PlayerView(journey);
        const int start=view->armies[army].tile;
        if(!Path(*view,start,order.destination).empty()) return {};
        JourneyMarchBlocker best;
        std::size_t shortest=TileCount+1;
        for(int id=0;id<int(view->armies.size());++id)
        {
            auto& unit=view->armies[id];
            if(id==army || unit.troops<=0) continue;
            const int troops=unit.troops;
            unit.troops=0;
            const auto route=Path(*view,start,order.destination);
            unit.troops=troops;
            if(!route.empty() && route.size()<shortest) { shortest=route.size();best={id,unit.tile}; }
        }
        return best;
    }
    std::vector<int> JourneyRealtime::Route(const StrategistJourney& journey,int army) const
    {
        if (army<0 || army>=3 || !TacticChain::Living(journey.world,army)) return {};
        const auto& order=orders[army];
        if (order.kind!=JourneyDirective::March && order.kind!=JourneyDirective::Attack) return {};
        auto seen=PlayerView(journey);
        const int start=seen->armies[army].tile;
        if (order.kind==JourneyDirective::March) return Ground(*seen,order.destination) ? Path(*seen,start,order.destination) : std::vector<int>{};
        const auto* target=journey.intelligence[0].Find(order.target);
        if (!target || !target->current || target->observed.troops<=0) return {};
        if (seen->CanStrike(army,target->observed.tile)) return {start};
        std::vector<int> best;
        for (int goal : seen->MapNeighbors(target->observed.tile))
        {
            if (!Ground(*seen,goal)) continue;
            auto path=Path(*seen,start,goal);
            if (!path.empty() && (best.empty() || path.size()<best.size())) best=std::move(path);
        }
        return best;
    }
    JourneyMarchPlan JourneyRealtime::MarchPlan(const StrategistJourney& journey,int army) const
    {
        JourneyMarchPlan plan;
        if (army<0 || army>=3 || !TacticChain::Living(journey.world,army) || orders[army].kind!=JourneyDirective::March) return plan;
        const auto& order=orders[army];plan.points={order.destination};
        plan.points.insert(plan.points.end(),order.waypoints.begin(),order.waypoints.end());
        auto seen=PlayerView(journey);int from=seen->armies[army].tile;plan.route={from};
        for (int point : plan.points)
        {
            if (!Ground(*seen,point)) { plan.complete=false;break; }
            const auto leg=Path(*seen,from,point);
            if (leg.empty()) { plan.complete=false;break; }
            plan.route.insert(plan.route.end(),leg.begin()+1,leg.end());
            seen->armies[army].tile=point;from=point;
        }
        return plan;
    }
    JourneyReliefApproach JourneyRealtime::ReliefApproach(const StrategistJourney& journey,int army,int source) const
    {
        JourneyReliefApproach result;
        if(journey.stage!=JourneyStage::Battle || army<0 || army>=3 || !TacticChain::Living(journey.world,army) ||
            !Ground(journey.world,source)) return result;
        auto seen=PlayerView(journey);const int start=seen->armies[army].tile;
        if(!Ground(*seen,start) || start==source) return result;
        if(seen->MapDistance(start,source)==1)
        { result.state=JourneyApproachState::Adjacent;result.staging=start;result.route={start};return result; }
        // Never assume the guard vacates its own post while the successor approaches.
        seen->tiles[source].terrain=Terrain::River;
        auto Choose=[&](const Campaign& view)
        {
            std::vector<int> best;
            for(int goal:view.MapNeighbors(source))
            {
                if(!Ground(view,goal)) continue;
                auto path=Path(view,start,goal);
                if(!path.empty() && (best.empty() || path.size()<best.size())) best=std::move(path);
            }
            return best;
        };
        result.route=Choose(*seen);
        if(!result.route.empty())
        { result.state=JourneyApproachState::Clear;result.staging=result.route.back();return result; }
        auto open=std::make_unique<Campaign>(*seen);
        for(int i=0;i<int(open->armies.size());++i)
            if(i!=army && !open->Hostile(open->player,open->armies[i].faction)) open->armies[i].troops=0;
        const auto hypothetical=Choose(*open);
        if(!hypothetical.empty())
        {
            for(std::size_t p=1;p<hypothetical.size();++p) for(int i=0;i<int(seen->armies.size());++i)
                if(i!=army && TacticChain::Living(*seen,i) && !seen->Hostile(seen->player,seen->armies[i].faction) && seen->armies[i].tile==hypothetical[p])
                {
                    result.state=JourneyApproachState::AllyBlocked;result.staging=hypothetical.back();result.route=hypothetical;
                    result.blocker=i;result.blockedTile=hypothetical[p];return result;
                }
        }
        result.state=JourneyApproachState::NoPath;return result;
    }

}
