#include "JourneySupportActionInternal.hpp"
#include <algorithm>
#include <memory>
namespace frontline::supportactions
{
    void AddProvenanceKey(const StrategistJourney& j,std::vector<int>& key)
    {
        key.insert(key.end(),{int(j.lastChain.prediction),j.lastChain.completed,int(j.lastChain.events.size())});
        for(const auto& e:j.lastChain.events)
        {
            key.insert(key.end(),{e.army,e.enemy,int(e.tactic),int(e.success),e.to,e.combo,e.setupPercent,int(e.changes.size())});
            for(const auto& c:e.changes) key.insert(key.end(),{c.army,c.tile,c.troops,int(c.pressured),int(c.disordered)});
        }
    }
    namespace
    {
        int Proven(const StrategistJourney& j,int actor,int target)
        {
            if(j.lastChain.prediction || !j.chain.spent[actor]) return 0;
            const auto* seen=j.intelligence[0].Find(target);if(!seen || !seen->current) return 0;
            int opening=0;
            for(const auto& e:j.lastChain.events) if(e.success && e.combo>0 && e.army==actor && e.enemy==target && e.to==seen->observed.tile)
            {
                const bool pressure=e.tactic==ChainTactic::Feint || (e.tactic==ChainTactic::Ambush && e.setupPercent==100);
                const bool disorder=(e.setupPercent==125 && (e.tactic==ChainTactic::Ambush || e.tactic==ChainTactic::Disrupt)) ||
                    e.tactic==ChainTactic::AzureDragon || e.tactic==ChainTactic::SerpentRoar;
                for(const auto& c:e.changes) if(c.army==target)
                { if(pressure && c.pressured && j.chain.pressured[target]) opening|=1;if(disorder && c.disordered && j.chain.disordered[target]) opening|=2; }
            }
            return opening;
        }
    }
    std::vector<JourneySupportAction> Followups(const StrategistJourney& j,const JourneyRealtime& clock,int actor)
    {
        std::vector<JourneySupportAction> result;
        for(int goal:{j.goal,j.Definition().secondGoal})
        {
            if(j.world.MapDistance(j.world.armies[actor].tile,goal)>2) continue;
            int guard=-1;for(int a=0;a<3;++a) if(a!=actor && GuardReady(j,clock,a,goal)) { guard=a;break; }
            if(guard<0) continue;
            for(const auto& seen:j.intelligence[0].contacts)
            {
                if(!seen.current || seen.id<0 || seen.id>=int(j.chain.spent.size()) || seen.observed.troops<=0 ||
                   !j.world.Hostile(j.world.player,seen.observed.faction) || j.world.MapDistance(seen.observed.tile,goal)>1) continue;
                const int opening=Proven(j,actor,seen.id);if(!opening) continue;
                for(const auto tactic:TacticChain::Loadout(j.world,guard))
                {
                    const auto& info=TacticChain::Definition(tactic);if(info.unique || !info.targetsEnemy || tactic==ChainTactic::Lure) continue;
                    const std::vector<ChainOrder> orders{{guard,seen.id,tactic,-1}};const auto prediction=j.Preview(orders);
                    if(prediction.completed!=1 || prediction.events.size()!=1 || prediction.events[0].setupPercent<=100) continue;
                    // Remove only this supporter's proven effects; another officer's opening must remain.
                    auto plain=std::make_unique<StrategistJourney>(j);
                    if(opening&1) plain->chain.pressured[seen.id]=false;if(opening&2) plain->chain.disordered[seen.id]=false;
                    const auto baseline=plain->Preview(orders);
                    if(baseline.completed==1 && baseline.events[0].setupPercent>=prediction.events[0].setupPercent) continue;
                    auto c=Preview(j,actor,goal,guard,orders);if(c.goal>=0) result.push_back(std::move(c));
                }
            }
        }
        std::stable_sort(result.begin(),result.end(),[&](const auto& a,const auto& b) { return Score(j,a)>Score(j,b); });
        if(result.empty()) return result;
        const auto goal=result[0].goal,target=result[0].plan.target;result.erase(std::remove_if(result.begin(),result.end(),[&](const auto& c) { return c.goal!=goal || c.plan.target!=target; }),result.end());
        if(result.size()>2) result.resize(2);return result;
    }
}
