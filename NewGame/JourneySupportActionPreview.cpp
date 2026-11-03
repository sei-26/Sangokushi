#include "JourneySupportActionInternal.hpp"
#include <algorithm>
namespace frontline::supportactions
{
    int Threats(const StrategistJourney& j,int goal,int target,int tile,int troops)
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
    int Effect(const StrategistJourney& j,const JourneySupportAction& c)
    {
        const auto& seen=j.intelligence[0].Find(c.plan.target)->observed;
        const bool breaks=seen.stance==battle::Stance::Guard && std::any_of(c.plan.orders.begin(),c.plan.orders.end(),[](const auto& o) { return o.tactic==ChainTactic::Disrupt; });
        return int(c.plan.damage>=seen.troops) | (int(c.enemyTo!=seen.tile)<<1) |
            (int(c.plan.disordered && !j.chain.disordered[c.plan.target])<<2) | (int(c.plan.pressured && !j.chain.pressured[c.plan.target])<<3) | (int(breaks)<<4);
    }
    int Score(const StrategistJourney& j,const JourneySupportAction& c)
    {
        const int effect=Effect(j,c);
        return 2000*(c.threatsBefore-c.threatsAfter)+1600*(effect&1)+200*(bool(effect&2))+150*(bool(effect&4))+
            80*(bool(effect&8))+100*(bool(effect&16))+c.plan.damage+2*c.enemyMoraleLoss-c.plan.friendlyLoss-j.world.MapDistance(c.actorTo,c.goal);
    }
    JourneySupportAction Preview(const StrategistJourney& j,int supporter,int goal,int guard,const std::vector<ChainOrder>& orders)
    {
        JourneySupportAction out;if(orders.empty() || orders.size()>2) return out;
        const int first=orders.front().army,target=orders.front().enemy;
        const auto* seen=j.intelligence[0].Find(target);if(!seen || !seen->current || seen->observed.troops<=0) return out;
        for(std::size_t n=0;n<orders.size();++n)
        {
            const auto& o=orders[n];const auto& info=TacticChain::Definition(o.tactic);
            if(o.enemy!=target || (o.army!=supporter && o.army!=guard) || info.unique || !info.targetsEnemy || (n && o.army==first)) return out;
        }
        const auto result=j.Preview(orders);if(result.completed!=int(orders.size()) || result.events.size()!=orders.size()) return out;
        std::array<int,3> troops{},morale{};for(int a=0;a<3;++a) { troops[a]=j.world.armies[a].troops;morale[a]=j.world.armies[a].morale; }
        int enemyTroops=seen->observed.troops,enemyMorale=seen->observed.morale;
        out.actorTo=j.world.armies[first].tile;out.enemyTo=seen->observed.tile;out.plan.target=target;out.plan.orders=orders;
        out.plan.disordered=j.chain.disordered[target];out.plan.pressured=j.chain.pressured[target];
        for(const auto& event:result.events)
        {
            out.plan.damage+=event.damage;out.plan.moraleCost+=TacticChain::Definition(event.tactic).moraleCost;
            out.guardActs|=event.army==guard;
            for(const auto& change:event.changes)
            {
                if(change.army==first) out.actorTo=change.tile;
                if(change.army==target) { out.enemyTo=change.tile;enemyTroops=change.troops;enemyMorale=change.morale;out.plan.disordered=change.disordered;out.plan.pressured=change.pressured; }
                if(change.army>=0 && change.army<3)
                {
                    troops[change.army]=change.troops;morale[change.army]=change.morale;
                    if(change.army==guard ? change.tile!=goal:change.army!=supporter && change.tile!=j.world.armies[change.army].tile) return {};
                    if(change.army==supporter && (change.tile==j.goal || change.tile==j.Definition().secondGoal)) return {};
                }
            }
            for(int post:{j.goal,j.Definition().secondGoal}) if(Threats(j,post,target,out.enemyTo,enemyTroops)>Threats(j,post)) return {};
        }
        if(!StrategistJourney::OnMap(out.actorTo) || (enemyTroops>0 && !StrategistJourney::OnMap(out.enemyTo)) ||
           (out.guardActs && (troops[guard]<150 || morale[guard]<25))) return {};
        for(int a=0;a<3;++a) out.plan.friendlyLoss+=std::max(0,j.world.armies[a].troops-troops[a]);
        out.goal=goal;out.guard=guard;out.supporter=supporter;out.followup=first==guard;
        out.enemyMoraleLoss=std::max(0,seen->observed.morale-enemyMorale);
        out.threatsBefore=Threats(j,goal);out.threatsAfter=Threats(j,goal,target,out.enemyTo,enemyTroops);
        out.clearsThreat=out.threatsAfter<out.threatsBefore;return out;
    }
}
