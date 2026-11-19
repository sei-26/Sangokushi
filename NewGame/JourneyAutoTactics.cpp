#include "JourneyAutoBattle.hpp"

namespace frontline
{
    namespace
    {
        int PlanValue(const StrategistJourney& j,const ChainResult& result)
        {
            int score=0;
            std::vector<ChainUnitChange> final(j.world.armies.size());
            std::vector<bool> changed(final.size(),false);
            for(const auto& event:result.events)
            {
                score+=event.damage;
                if(event.damage>0 && j.HasConvoy())
                {
                    const auto* contact=j.intelligence[0].Find(event.enemy);
                    if(contact && j.world.MapDistance(contact->observed.tile,
                        j.world.armies[StrategistJourney::ConvoyArmy].tile)<=2) score+=60;
                }
                for(const auto& change:event.changes)
                { final[change.army]=change;changed[change.army]=true; }
            }
            for(int a=0;a<int(final.size());++a)
            {
                if(!changed[a]) continue;
                const auto& after=final[a];
                if(a<3)
                {
                    const auto& before=j.world.armies[a];
                    score-=std::max(0,before.troops-after.troops);
                    // Reward real recovery when it is needed, not an unused rally at full morale.
                    score+=2*std::max(0,std::min(65,after.morale)-before.morale);
                }
                else
                {
                    const auto* contact=j.intelligence[0].Find(a);
                    if(!contact || !j.world.Hostile(j.world.player,contact->observed.faction)) continue;
                    if(after.troops<=0) score+=j.depotInterception && a==3 ? 240:120;
                    else
                    {
                        if(after.disordered && !j.chain.disordered[a])
                            score+=j.depotInterception && a==3 ? 240:80;
                        if(after.pressured && !j.chain.pressured[a]) score+=30;
                    }
                }
            }
            return score;
        }
    }

    std::vector<ChainOrder> PlanJourneyAutoTactics(const StrategistJourney& j,const std::array<bool,3>& defending)
    {
        const auto& w=j.world;
        std::array<std::vector<ChainTactic>,3> loadouts;
        for(int a=0;a<3;++a) loadouts[a]=TacticChain::Loadout(w,a);
        std::vector<int> targets;
        for(const auto& contact:j.intelligence[0].contacts)
            if(contact.current && contact.observed.troops>0 && w.Hostile(w.player,contact.observed.faction) &&
               (!j.HasYangrenStrategy() || (contact.id!=GrandCommander && (contact.id!=8 || j.battleReview.grand.yangren.counterOn) && contact.observed.morale>=25)) &&
               (!j.HasGrandStrategy() || contact.id==GrandCommander || contact.observed.morale>=25) &&
               (!j.depotInterception || contact.id==3)) targets.push_back(contact.id);

        // Only friendly state and current observations inform range checks, including
        // the positions after earlier tactics have pushed or defeated an enemy.
        std::vector<Army> observed(w.armies.size());
        for(int a=0;a<3;++a) observed[a]=w.armies[a];
        for(int target:targets) observed[target]=j.intelligence[0].Find(target)->observed;

        std::vector<ChainOrder> best,orders;
        std::array<bool,3> used{};
        int bestValue=0,bestCost=0;
        auto Search=[&](auto&& search,const std::vector<Army>& units,int moraleCost)->void
        {
            if(orders.size()==3) return;
            for(int a=0;a<3;++a)
            {
                if(used[a] || defending[a] || j.chain.spent[a] || units[a].troops<150) continue;
                for(auto tactic:loadouts[a])
                {
                    const auto& definition=TacticChain::Definition(tactic);
                    // Retreats remain player-directed to preserve assigned objectives.
                    if(tactic==ChainTactic::Lure || (definition.unique && j.chain.signatureUsed[a]) ||
                       units[a].morale<std::max(15,definition.moraleCost)) continue;
                    const auto candidates=definition.targetsEnemy ? targets:std::vector<int>{-1};
                    for(int target:candidates)
                    {
                        if(target>=0 && (units[target].troops<=0 || w.MapDistance(units[a].tile,units[target].tile)!=1)) continue;
                        orders.push_back({a,target,tactic,-1});
                        const auto prediction=j.Preview(orders);
                        if(prediction.completed==int(orders.size()))
                        {
                            const int value=PlanValue(j,prediction),cost=moraleCost+definition.moraleCost;
                            if(value>bestValue || (value==bestValue && value>0 &&
                               (cost<bestCost || (cost==bestCost && orders.size()<best.size()))))
                            { best=orders;bestValue=value;bestCost=cost; }
                            auto next=units;
                            for(const auto& change:prediction.events.back().changes)
                            {
                                next[change.army].tile=change.tile;
                                next[change.army].troops=change.troops;
                                next[change.army].morale=change.morale;
                            }
                            used[a]=true;
                            search(search,next,cost);
                            used[a]=false;
                        }
                        orders.pop_back();
                    }
                }
            }
        };
        Search(Search,observed,0);
        return best;
    }
}
