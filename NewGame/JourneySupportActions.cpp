#include "JourneySupportActions.hpp"
#include "JourneySupport.hpp"
#include <algorithm>
namespace frontline
{
    namespace supportactions { JourneySupportAction Preview(const StrategistJourney&,int,int,int,const ChainOrder&); }
    std::vector<int> JourneySupportActionContext(const StrategistJourney& j,const JourneyRealtime& clock,int actor)
    {
        auto key=BattleChoiceContext(j,-1);const auto support=JourneySupportContext(j,clock,actor);
        key.insert(key.end(),support.begin(),support.end());return key;
    }
    namespace
    {
        int Effect(const StrategistJourney& j,const JourneySupportAction& c)
        {
            const auto& o=c.plan.orders[0];const auto& seen=j.intelligence[0].Find(c.plan.target)->observed;
            return int(c.plan.damage>=seen.troops) | (int(c.enemyTo!=seen.tile)<<1) |
                (int(c.plan.disordered && !j.chain.disordered[o.enemy])<<2) | (int(c.plan.pressured && !j.chain.pressured[o.enemy])<<3) |
                (int(o.tactic==ChainTactic::Disrupt && seen.stance==battle::Stance::Guard)<<4);
        }
        int Score(const StrategistJourney& j,const JourneySupportAction& c)
        {
            const int effect=Effect(j,c);
            return 2000*(c.threatsBefore-c.threatsAfter)+1600*(effect&1)+200*(bool(effect&2))+150*(bool(effect&4))+
                80*(bool(effect&8))+100*(bool(effect&16))+c.plan.damage+2*c.enemyMoraleLoss-c.plan.friendlyLoss-j.world.MapDistance(c.actorTo,c.goal);
        }
    }
    std::vector<JourneySupportAction> ReadJourneySupportActions(const StrategistJourney& j,const JourneyRealtime& clock,int actor)
    {
        std::vector<JourneySupportAction> bait,attacks;
        if(j.stage!=JourneyStage::Battle || j.Definition().kind!=JourneyObjective::HoldPositions || actor<0 || actor>=3 ||
           !TacticChain::Living(j.world,actor) || j.world.armies[actor].faction!=j.world.player || j.assignments[actor].duty!=JourneyDuty::None ||
           (clock.orders[actor].kind!=JourneyDirective::None && clock.orders[actor].kind!=JourneyDirective::Hold) ||
           j.world.armies[actor].tile==j.goal || j.world.armies[actor].tile==j.Definition().secondGoal) return {};
        for(int goal:{j.goal,j.Definition().secondGoal})
        {
            const int distance=j.world.MapDistance(j.world.armies[actor].tile,goal);if(distance<1 || distance>2) continue;
            int guard=-1;for(int a=0;a<3;++a) if(a!=actor && TacticChain::Living(j.world,a) && j.world.armies[a].faction==j.world.player && j.world.armies[a].tile==goal) { guard=a;break; }
            if(guard<0) continue;
            for(const auto& seen:j.intelligence[0].contacts)
            {
                if(!seen.current || seen.id<0 || seen.id>=int(j.chain.spent.size()) || seen.observed.troops<=0 ||
                   !j.world.Hostile(j.world.player,seen.observed.faction) || j.world.MapDistance(seen.observed.tile,goal)>1 ||
                   j.world.MapDistance(seen.observed.tile,j.world.armies[actor].tile)!=1) continue;
                for(const auto tactic:TacticChain::Loadout(j.world,actor))
                {
                    const auto& info=TacticChain::Definition(tactic);if(info.unique || !info.targetsEnemy) continue;
                    auto Add=[&](int destination)
                    {
                        auto c=supportactions::Preview(j,actor,goal,guard,{actor,seen.id,tactic,destination});if(c.goal<0) return;
                        if(tactic==ChainTactic::Lure) { if(c.enemyTo!=seen.observed.tile) bait.push_back(std::move(c)); }
                        else attacks.push_back(std::move(c));
                    };
                    if(tactic!=ChainTactic::Lure) { Add(-1);continue; }
                    for(int tile:j.world.MapNeighbors(j.world.armies[actor].tile))
                        if(StrategistJourney::OnMap(tile) && j.intelligence[0].visible[tile] && tile!=j.goal && tile!=j.Definition().secondGoal) Add(tile);
                }
            }
        }
        const JourneySupportAction* focus=nullptr;
        for(const auto* list:{&bait,&attacks}) for(const auto& c:*list) if(!focus || Score(j,c)>Score(j,*focus)) focus=&c;
        if(!focus) return {};
        const int goal=focus->goal,guard=focus->guard,target=focus->plan.target;
        for(auto* list:{&bait,&attacks}) list->erase(std::remove_if(list->begin(),list->end(),[&](const auto& c)
            { return c.goal!=goal || c.guard!=guard || c.plan.target!=target; }),list->end());
        auto Best=[&](auto& list) { std::stable_sort(list.begin(),list.end(),[&](const auto& a,const auto& b) { return Score(j,a)>Score(j,b); }); };
        Best(bait);Best(attacks);std::vector<JourneySupportAction> result;if(!bait.empty()) result.push_back(std::move(bait.front()));
        for(auto& candidate:attacks)
        {
            bool duplicate=false;for(const auto& c:result) if(c.goal==candidate.goal && c.plan.target==candidate.plan.target && Effect(j,c)==Effect(j,candidate)) duplicate=true;
            if(!duplicate) result.push_back(std::move(candidate));if(result.size()==2) break;
        }
        return result;
    }
}
