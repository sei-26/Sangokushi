#include "JourneyAutoBattle.hpp"
#include <memory>
namespace frontline
{
    void CoordinateCoalitionReturn(const StrategistJourney& j,JourneyAutoPlan& plan)
    {
        if(!j.HasCoalitionBattle() || j.coalitionPlan!=CoalitionPlan::Protect || !j.battleReview.grand.coalition.rescuedOn) return;
        auto seen=std::make_unique<Campaign>(j.world);
        for(auto& u:seen->armies) if(u.faction!=0) u.troops=0;
        for(const auto& c:j.intelligence[0].contacts) if(c.current) seen->armies[c.id]=c.observed;
        auto clear=std::make_unique<Campaign>(*seen);
        for(int a=0;a<3;++a) if(clear->armies[a].tile!=j.goal) clear->armies[a].troops=0;
        std::vector<int> passage;
        for(const auto& p:j.PlanCoalitionAllies()) if(p.army==5 || p.army==6)
        {
            const int next=JourneyAdvanceStep(*clear,p.from,p.target,p.occupyTarget);
            if(next!=p.from) passage.push_back(next);
        }
        for(int a=0;a<3;++a)
        {
            if(!TacticChain::Living(j.world,a) || j.chain.moved[a] || j.world.armies[a].tile==j.goal) continue;
            const int from=j.world.armies[a].tile;
            const bool blocking=std::find(passage.begin(),passage.end(),from)!=passage.end();
            if(!blocking)
            {
                const auto route=plan.movement.Route(j,a);
                if(route.size()>1 && std::find(passage.begin(),passage.end(),route[1])!=passage.end())
                { plan.movement.orders[a]={};plan.reasons[a]=U"帰る味方を先に通す。橋へ入り直さず、一歩待つ！"; }
                continue;
            }
            bool attacking=false;for(const auto& o:plan.tactics) attacking|=o.army==a;
            if(attacking) continue;
            int best=-1,score=100000;
            for(int tile:seen->MapNeighbors(from))
            {
                if(tile==j.goal || seen->tiles[tile].terrain==Terrain::Bridge ||
                    std::find(passage.begin(),passage.end(),tile)!=passage.end()) continue;
                auto move=plan.movement;
                if(!move.Queue(j,a,JourneyDirective::March,tile) || move.Route(j,a).size()!=2) continue;
                const int value=seen->MapDistance(tile,j.goal)+(tile%Width>7 ? 8:0);
                if(value<score) { best=tile;score=value; }
            }
            if(best>=0)
            { plan.movement.Queue(j,a,JourneyDirective::March,best);plan.reasons[a]=U"帰る旗の進路を空ける。橋の脇へ一歩退く！"; }
        }
    }
}
