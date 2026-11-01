#include "JourneyAutoBattle.hpp"
namespace frontline
{
    void CoordinateJourneyAutoMovement(const StrategistJourney& j,JourneyAutoPlan& plan)
    {
        const auto& w=j.world;
        for(int a=0;a<3;++a)
        {
            if(!TacticChain::Living(w,a) || j.chain.moved[a]) continue;
            const auto& order=plan.movement.orders[a];
            const auto* contact=j.intelligence[0].Find(order.target);
            if(order.kind!=JourneyDirective::Attack || !contact || !contact->current ||
               w.tiles[w.armies[a].tile].terrain!=Terrain::Bridge ||
               w.MapDistance(w.armies[a].tile,contact->observed.tile)!=1) continue;
            // Keep fighting beside the bridge, so following allies can cross.
            // Route validates terrain and observed occupancy; no hidden unit is read.
            int best=-1,score=100000;
            for(int tile:w.MapNeighbors(w.armies[a].tile))
            {
                if(w.tiles[tile].terrain==Terrain::Bridge || w.MapDistance(tile,contact->observed.tile)!=1) continue;
                auto movement=plan.movement;
                if(!movement.Queue(j,a,JourneyDirective::March,tile)) continue;
                const auto route=movement.Route(j,a);
                if(route.size()!=2) continue;
                const int value=w.MapDistance(tile,j.goal);
                if(value<score) { best=tile;score=value; }
            }
            if(best>=0)
            {
                plan.movement.Queue(j,a,JourneyDirective::March,best);
                plan.reasons[a]=U"橋の出口で戦う。後続が渡る道を空ける！";
            }
        }
        std::vector<int> arrivals;
        for(int a=0;a<3;++a)
        {
            if(j.chain.moved[a]) continue;
            const auto route=plan.movement.Route(j,a);
            if(route.size()<2) continue;
            if(std::find(arrivals.begin(),arrivals.end(),route[1])==arrivals.end()) arrivals.push_back(route[1]);
            else
            {
                // Both would otherwise stop every pulse at the same empty cell.
                // Yield movement only; grant no guard, action or free passage.
                plan.movement.orders[a]={};
                plan.reasons[a]=U"味方を先に通す。道が空いてから続く！";
            }
        }
    }
}
