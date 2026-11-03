#include "JourneySupportActions.hpp"
namespace frontline::supportactions
{
    int Threats(const StrategistJourney& j,int goal,int target=-1,int tile=-1,int troops=0)
    {
        int count=0;
        for(const auto& c:j.intelligence[0].contacts)
        {
            if(!c.current || !j.world.Hostile(j.world.player,c.observed.faction)) continue;
            const int at=c.id==target ? tile:c.observed.tile,number=c.id==target ? troops:c.observed.troops;
            if(number>0 && j.world.MapDistance(at,goal)<=1) ++count;
        }
        return count;
    }
    JourneySupportAction Preview(const StrategistJourney& j,int actor,int goal,int guard,const ChainOrder& order)
    {
        JourneySupportAction out;const auto* seen=j.intelligence[0].Find(order.enemy);
        if(!seen || !seen->current || seen->observed.troops<=0) return out;
        const auto result=j.Preview({order});if(result.completed!=1 || result.events.size()!=1 || !result.events[0].success) return out;
        const auto& event=result.events[0];int enemyTroops=seen->observed.troops,enemyMorale=seen->observed.morale;
        out.actorTo=j.world.armies[actor].tile;out.enemyTo=seen->observed.tile;
        out.plan.target=order.enemy;out.plan.orders={order};out.plan.damage=event.damage;
        out.plan.moraleCost=TacticChain::Definition(order.tactic).moraleCost;
        out.plan.disordered=j.chain.disordered[order.enemy];out.plan.pressured=j.chain.pressured[order.enemy];
        for(const auto& change:event.changes)
        {
            if(change.army==actor) { out.actorTo=change.tile;out.plan.friendlyLoss=std::max(0,j.world.armies[actor].troops-change.troops); }
            if(change.army==order.enemy)
            { out.enemyTo=change.tile;enemyTroops=change.troops;enemyMorale=change.morale;out.plan.disordered=change.disordered;out.plan.pressured=change.pressured; }
            if(change.army>=0 && change.army<3 && change.army!=actor && change.tile!=j.world.armies[change.army].tile) return {};
        }
        if(!StrategistJourney::OnMap(out.actorTo) || out.actorTo==j.goal || out.actorTo==j.Definition().secondGoal ||
           (enemyTroops>0 && !StrategistJourney::OnMap(out.enemyTo))) return {};
        for(int post:{j.goal,j.Definition().secondGoal})
            if(Threats(j,post,order.enemy,out.enemyTo,enemyTroops)>Threats(j,post)) return {};
        out.goal=goal;out.guard=guard;out.enemyMoraleLoss=std::max(0,seen->observed.morale-enemyMorale);
        out.threatsBefore=Threats(j,goal);out.threatsAfter=Threats(j,goal,order.enemy,out.enemyTo,enemyTroops);
        out.clearsThreat=out.threatsAfter<out.threatsBefore;return out;
    }
}
