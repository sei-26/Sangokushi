#include "JourneyAutoBattle.hpp"
#include <memory>
namespace frontline
{
    void CoordinateCoalitionReturn(const StrategistJourney&,JourneyAutoPlan&);
    void CoordinateJourneyAutoMovement(const StrategistJourney& j,JourneyAutoPlan& plan)
    {
        const auto& w=j.world;
        CoordinateCoalitionReturn(j,plan);
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
                const auto& order=plan.movement.orders[a];
                const auto* contact=j.intelligence[0].Find(order.target);
                int flank=-1,best=100000;
                // Avoid steps already allocated to other officers. Issue only
                // the resulting next step so resolution cannot choose the
                // original contested shortest path again.
                auto detour=std::make_unique<StrategistJourney>(j);
                for(int tile:arrivals) detour->world.tiles[tile].terrain=Terrain::River;
                if(order.kind==JourneyDirective::Attack && contact && contact->current)
                    for(int tile:w.MapNeighbors(contact->observed.tile))
                    {
                        // Leave the escort road and assigned objective positions
                        // available for their own units.
                        if(j.HasConvoy() && std::find(j.ConvoyPath().begin(),j.ConvoyPath().end(),tile)!=j.ConvoyPath().end()) continue;
                        bool reserved=false;
                        for(int other=0;other<3;++other)
                            if(other!=a && plan.movement.orders[other].kind==JourneyDirective::March &&
                               plan.movement.orders[other].arrival==JourneyArrival::Hold &&
                               plan.movement.orders[other].destination==tile) reserved=true;
                        if(reserved) continue;
                        auto movement=plan.movement;
                        if(!movement.Queue(j,a,JourneyDirective::March,tile)) continue;
                        const auto alternative=movement.Route(*detour,a);
                        if(alternative.size()<2 || alternative.size()>route.size()+2 ||
                           std::find(arrivals.begin(),arrivals.end(),alternative[1])!=arrivals.end()) continue;
                        int score=int(alternative.size())*20;
                        for(int other=0;other<3;++other)
                            if(other!=a && TacticChain::Living(w,other) && w.MapDistance(w.armies[other].tile,contact->observed.tile)==1 &&
                               w.MapDistance(w.armies[other].tile,tile)>=2) score-=8;
                        if(score<best) { best=score;flank=alternative[1]; }
                    }
                if(flank>=0)
                {
                    plan.movement.Queue(j,a,JourneyDirective::March,flank);
                    arrivals.push_back(plan.movement.Route(j,a)[1]);
                    plan.reasons[a]=U"味方とは別の側から回り込む。挟んで戦法を合わせる！";
                    continue;
                }
                // Both would otherwise stop every pulse at the same empty cell.
                // Yield movement only; grant no guard, action or free passage.
                plan.movement.orders[a]={};
                plan.reasons[a]=U"味方を先に通す。道が空いてから続く！";
            }
        }
    }
}
