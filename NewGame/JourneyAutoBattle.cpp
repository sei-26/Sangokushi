#include "JourneyAutoBattle.hpp"
#include <limits>
#include <memory>
namespace frontline
{
    JourneyAutoPlan PlanJourneyAutoBattle(const StrategistJourney& j)
    {
        JourneyAutoPlan plan;
        if(j.stage!=JourneyStage::Battle) return plan;
        const auto& w=j.world;
        std::array<int,3> duties{-1,-1,-1};
        auto Assign=[&](int tile)
        {
            int best=-1,score=100000;
            for(int a=0;a<3;++a) if(TacticChain::Living(w,a) && duties[a]<0)
            {
                const int value=w.MapDistance(w.armies[a].tile,tile)*10+
                    (w.armies[a].troops<500 ? 20:0);
                if(value<score) { score=value;best=a; }
            }
            if(best>=0) duties[best]=tile;
        };
        if(j.Definition().kind==JourneyObjective::HoldPositions)
        { Assign(j.goal);Assign(j.Definition().secondGoal); }
        else if(j.Definition().kind==JourneyObjective::CaptureCamp) Assign(j.goal);
        std::array<bool,3> defending{};
        for(int a=0;a<3;++a)
        {
            if(!TacticChain::Living(w,a)) continue;
            const int at=w.armies[a].tile;
            int target=-1,dist=100000;
            for(const auto& c:j.intelligence[0].contacts)
            {
                if(!c.current || c.observed.troops<=0 || !w.Hostile(w.player,c.observed.faction)) continue;
                int value=w.MapDistance(at,c.observed.tile);
                if(j.HasConvoy())
                {
                    const auto& path=j.ConvoyPath();
                    const int next=path[std::min(j.convoyStep+1,int(path.size())-1)];
                    value+=w.MapDistance(c.observed.tile,next)*2;
                }
                if(value<dist) { dist=value;target=c.id; }
            }
            if(duties[a]>=0)
            {
                if(at==duties[a])
                {
                    plan.movement.Queue(j,a,JourneyDirective::Hold);
                    defending[a]=true;plan.reasons[a]=U"旗を守る。仲間が動ける時間をつくる！";
                }
                else
                {
                    plan.movement.Queue(j,a,JourneyDirective::March,duties[a]);
                    plan.movement.orders[a].arrival=JourneyArrival::Hold;
                    if(plan.movement.Route(j,a).empty() && target>=0) plan.movement.Queue(j,a,JourneyDirective::Attack,target);
                    plan.reasons[a]=U"目標へ進む。道を塞ぐ敵は押し返す！";
                }
            }
            else if(target>=0)
            {
                plan.movement.Queue(j,a,JourneyDirective::Attack,target);
                plan.reasons[a]=j.HasConvoy() ? U"車の前を開ける。追い過ぎず、護衛を助ける！":U"敵を押さえる。仲間の一撃につなぐぞ！";
            }
            else
            {
                int goal=j.goal;
                if(j.HasConvoy())
                {
                    const auto& path=j.ConvoyPath();
                    goal=path[std::min(j.convoyStep+2,int(path.size())-1)];
                }
                // Find a reachable flank instead of waiting on a friendly-occupied objective/cart road.
                std::vector<int> candidates{goal};
                const auto neighbours=w.MapNeighbors(goal);candidates.insert(candidates.end(),neighbours.begin(),neighbours.end());
                bool queued=false;
                for(int cell:candidates)
                {
                    if(j.HasConvoy() && std::find(j.ConvoyPath().begin(),j.ConvoyPath().end(),cell)!=j.ConvoyPath().end()) continue;
                    if(plan.movement.Queue(j,a,JourneyDirective::March,cell) && !plan.movement.Route(j,a).empty()) { queued=true;break; }
                }
                if(!queued) plan.movement.Queue(j,a,JourneyDirective::Hold);
                plan.reasons[a]=j.HasConvoy() ? U"車道を空け、前へ。見えていない敵にも備える！":U"目標へ進み、次の敵を探る！";
            }
        }
        // Greedily extend a fully successful observed-state combo. Never spend a beacon guard's action.
        std::array<bool,3> used{};
        for(int step=0;step<3;++step)
        {
            int bestScore=0;ChainOrder best;
            for(int a=0;a<3;++a) if(!used[a] && !defending[a] && TacticChain::Living(w,a) && !j.chain.spent[a])
                for(auto tactic:TacticChain::Loadout(w,a))
                {
                    // Automatic retreat selection could abandon the promised objective; leave lure to manual command.
                    if(tactic==ChainTactic::Lure) continue;
                    const bool enemy=TacticChain::Definition(tactic).targetsEnemy;
                    std::vector<int> targets;
                    if(!enemy)
                    {
                        bool needs=false;
                        for(int b=0;b<3;++b) if(TacticChain::Living(w,b) && w.MapDistance(w.armies[a].tile,w.armies[b].tile)<=2 && w.armies[b].morale<65) needs=true;
                        if(needs) targets.push_back(-1);
                    }
                    else for(const auto& c:j.intelligence[0].contacts)
                        if(c.current && c.observed.troops>0 && w.Hostile(w.player,c.observed.faction)) targets.push_back(c.id);
                    for(int target:targets)
                    {
                        auto orders=plan.tactics;orders.push_back({a,target,tactic,-1});
                        const auto prediction=j.Preview(orders);
                        if(prediction.completed!=int(orders.size()) || prediction.events.empty()) continue;
                        const auto& e=prediction.events.back();
                        int score=e.damage+(!enemy ? 85:20);
                        if(target>=0 && j.HasConvoy())
                        {
                            const auto* c=j.intelligence[0].Find(target);
                            if(c && w.MapDistance(c->observed.tile,w.armies[StrategistJourney::ConvoyArmy].tile)<=2) score+=60;
                        }
                        if(score>bestScore) { bestScore=score;best=orders.back(); }
                    }
                }
            if(bestScore<=0) break;
            used[best.army]=true;plan.tactics.push_back(best);
        }
        return plan;
    }
}
