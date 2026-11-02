#include "JourneyBattleChoices.hpp"
#include <algorithm>
#include <limits>
namespace frontline
{
    std::vector<int> BattleChoiceContext(const StrategistJourney& j,int target)
    {
        std::vector<int> key{int(j.stage),j.mission,j.turn,j.goal,j.Definition().secondGoal,target,j.campaignFlow ? 1:0,int(j.Definition().kind),j.convoyStep,j.HasConvoy() ? 1:0};
        auto Add=[&](int id,const Army& a)
        {
            key.insert(key.end(),{id,a.general,a.tile,a.troops,a.morale,int(a.stance),a.faction,int(a.arm),a.supplied ? 1:0,a.retreat ? 1:0});
            const auto& g=j.world.generals[a.general];
            key.insert(key.end(),{g.leadership,g.politics,g.intelligence,g.charm,int(g.trait),int(g.tactic)});
            for(const auto* flags:{&j.chain.spent,&j.chain.moved,&j.chain.concealed,&j.chain.disordered,&j.chain.pressured,&j.chain.inspired,&j.chain.signatureUsed})
                key.push_back(id>=0 && id<int(flags->size()) && (*flags)[id] ? 1:0);
        };
        for(int a=0;a<3 && a<int(j.world.armies.size());++a) Add(a,j.world.armies[a]);
        // Stale contacts and private enemy positions never invalidate a visible choice.
        for(const auto& c:j.intelligence[0].contacts) if(c.current) Add(c.id,c.observed);
        for(int y=0;y<StrategistJourney::Rows;++y) for(int x=0;x<StrategistJourney::Columns;++x)
            key.push_back(int(j.world.tiles[Campaign::At(x,y)].terrain));
        return key;
    }
    namespace
    {
        JourneyBattleChoice Evaluate(const StrategistJourney& j,int target,const std::vector<ChainOrder>& orders)
        {
            JourneyBattleChoice out;const auto preview=j.Preview(orders);
            if(preview.completed!=int(orders.size()) || preview.events.size()!=orders.size()) return out;
            out.target=target;out.orders=orders;
            std::array<int,3> survivors{};
            for(int a=0;a<3;++a) survivors[a]=j.world.armies[a].troops;
            for(const auto& order:orders) out.moraleCost+=TacticChain::Definition(order.tactic).moraleCost;
            for(const auto& event:preview.events)
            {
                if(event.enemy==target) out.damage+=event.damage;
                for(const auto& change:event.changes)
                {
                    if(change.army==target) { out.disordered=change.disordered;out.pressured=change.pressured; }
                    if(change.army>=0 && change.army<3)
                    {
                        const int from=j.world.armies[change.army].tile;
                        if(j.Definition().kind==JourneyObjective::HoldPositions && (from==j.goal || from==j.Definition().secondGoal) && change.tile!=from) return {};
                        if(j.HasConvoy() && change.tile!=from)
                        {
                            const auto& path=j.ConvoyPath();const int next=path[std::min(j.convoyStep+1,int(path.size())-1)];
                            if(change.tile==next) return {};
                        }
                        survivors[change.army]=change.troops;
                    }
                }
            }
            for(int a=0;a<3;++a) out.friendlyLoss+=std::max(0,j.world.armies[a].troops-survivors[a]);
            return out;
        }
        int Value(const StrategistJourney& j,const JourneyBattleChoice& choice)
        {
            if(choice.orders.empty()) return std::numeric_limits<int>::min();
            const bool disordered=j.chain.disordered[choice.target],pressured=j.chain.pressured[choice.target];
            return choice.damage+(choice.disordered && !disordered ? 180:0)+(choice.pressured && !pressured ? 80:0)-choice.friendlyLoss;
        }
        JourneyBattleChoice Extend(const StrategistJourney& j,int target,const std::vector<ChainOrder>& prefix)
        {
            JourneyBattleChoice best;int score=std::numeric_limits<int>::min();
            for(int a=0;a<3;++a)
            {
                if(!TacticChain::Living(j.world,a) || j.chain.spent[a] ||
                   std::any_of(prefix.begin(),prefix.end(),[&](const ChainOrder& o) { return o.army==a; })) continue;
                if(j.Definition().kind==JourneyObjective::HoldPositions && (j.world.armies[a].tile==j.goal || j.world.armies[a].tile==j.Definition().secondGoal)) continue;
                for(auto tactic:TacticChain::Loadout(j.world,a))
                {
                    const auto& info=TacticChain::Definition(tactic);
                    if(info.unique || !info.targetsEnemy || tactic==ChainTactic::Lure) continue;
                    auto orders=prefix;orders.push_back({a,target,tactic,-1});
                    auto choice=Evaluate(j,target,orders);const int value=Value(j,choice);
                    if(value>score || (value==score && !choice.orders.empty() && choice.moraleCost<best.moraleCost))
                    { score=value;best=std::move(choice); }
                }
            }
            return best;
        }
    }
    std::vector<JourneyBattleChoice> ReadJourneyBattleChoices(const StrategistJourney& j,int target)
    {
        const auto* seen=j.intelligence[0].Find(target);
        if(j.stage!=JourneyStage::Battle || !seen || !seen->current || seen->observed.troops<=0 ||
           !j.world.Hostile(0,seen->observed.faction) || target<0 || target>=int(j.chain.disordered.size())) return {};
        auto solo=Extend(j,target,{});
        if(solo.orders.empty() || Value(j,solo)<=0) return {};
        std::vector<JourneyBattleChoice> result{solo},starts{solo};auto team=solo;
        // A bait-and-pincer plan can be worthwhile even when the bait alone is not.
        for(int a=0;a<3;++a)
        {
            if(!TacticChain::Living(j.world,a) || j.chain.spent[a] || j.world.armies[a].tile==j.goal || j.world.armies[a].tile==j.Definition().secondGoal ||
               !TacticChain::CanUse(j.world,a,ChainTactic::Lure)) continue;
            for(int tile:j.world.MapNeighbors(j.world.armies[a].tile))
            {
                if(!StrategistJourney::OnMap(tile) || !j.intelligence[0].visible[tile]) continue;
                if(j.HasConvoy() && std::find(j.ConvoyPath().begin(),j.ConvoyPath().end(),tile)!=j.ConvoyPath().end()) continue;
                auto bait=Evaluate(j,target,{{a,target,ChainTactic::Lure,tile}});
                if(!bait.orders.empty()) starts.push_back(std::move(bait));
            }
        }
        for(auto candidate:starts)
        {
            for(int step=1;step<3;++step)
            {
                auto next=Extend(j,target,candidate.orders);
                if(next.orders.empty() || Value(j,next)<=Value(j,candidate)) break;
                candidate=std::move(next);
            }
            if(candidate.orders.size()>1 && Value(j,candidate)>Value(j,team)) team=std::move(candidate);
        }
        if(team.orders.size()>1 && (team.damage>solo.damage || (team.disordered && !solo.disordered) || (team.pressured && !solo.pressured))) result.push_back(std::move(team));
        return result;
    }
}
