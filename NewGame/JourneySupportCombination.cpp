#include "JourneySupportActionInternal.hpp"
#include <algorithm>
namespace frontline::supportactions
{
    bool GuardReady(const StrategistJourney& j,const JourneyRealtime& clock,int guard,int goal)
    {
        if(guard<0 || guard>=3 || !TacticChain::Living(j.world,guard) || j.world.armies[guard].faction!=j.world.player ||
           j.world.armies[guard].tile!=goal || j.chain.spent[guard]) return false;
        const auto& o=clock.orders[guard];const auto& a=j.assignments[guard];
        return (o.kind==JourneyDirective::None || o.kind==JourneyDirective::Hold) &&
            (a.duty==JourneyDuty::None || (a.duty==JourneyDuty::Hold && a.tile==goal));
    }
    std::vector<JourneySupportAction> Combinations(const StrategistJourney& j,const JourneyRealtime& clock,int actor,const std::vector<JourneySupportAction>& solo)
    {
        std::vector<JourneySupportAction> result;if(solo.empty()) return result;
        const auto& best=solo.front();if(!GuardReady(j,clock,best.guard,best.goal)) return result;
        const auto* target=j.intelligence[0].Find(best.plan.target);
        if(target && best.plan.damage>=target->observed.troops) return result; // Do not spend the guard for the same already certain local defeat.
        for(const auto& first:solo) for(const auto tactic:TacticChain::Loadout(j.world,best.guard))
        {
            const auto& info=TacticChain::Definition(tactic);if(info.unique || !info.targetsEnemy || tactic==ChainTactic::Lure) continue;
            auto orders=first.plan.orders;orders.push_back({best.guard,best.plan.target,tactic,-1});
            auto c=Preview(j,actor,best.goal,best.guard,orders);if(c.goal<0) continue;
            if(c.plan.damage>best.plan.damage || c.threatsAfter<best.threatsAfter || c.enemyMoraleLoss>best.enemyMoraleLoss ||
               (c.plan.disordered && !best.plan.disordered) || (c.plan.pressured && !best.plan.pressured)) result.push_back(std::move(c));
        }
        std::stable_sort(result.begin(),result.end(),[&](const auto& a,const auto& b) { return Score(j,a)>Score(j,b); });
        if(result.size()>1) result.resize(1);return result;
    }
}
