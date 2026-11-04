#include "JourneyDefense.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    namespace
    {
        bool Active(const StrategistJourney& j,int goal)
        {
            return j.stage==JourneyStage::Battle && j.Definition().kind==JourneyObjective::HoldPositions &&
                StrategistJourney::OnMap(goal) && (goal==j.goal || goal==j.Definition().secondGoal);
        }
        JourneyDefenseRead Inspect(const StrategistJourney& j,int goal,bool actual)
        {
            JourneyDefenseRead r;if(!Active(j,goal)) return r;
            r.goal=goal;r.turn=actual ? j.turn-1:j.turn;
            for(int a=0;a<3 && a<int(j.world.armies.size());++a)
                if(TacticChain::Living(j.world,a) && j.world.armies[a].faction==j.world.player && j.world.armies[a].tile==goal) r.guard=a;
            auto Threat=[&](const Army& a) { return a.troops>0 && j.world.Hostile(j.world.player,a.faction) && j.world.MapDistance(a.tile,goal)<=1; };
            if(actual) { for(const auto& a:j.world.armies) if(Threat(a)) ++r.threats; }
            else { for(const auto& c:j.intelligence[0].contacts) if(c.current && Threat(c.observed)) ++r.threats; }
            if(r.guard<0) { r.reason=JourneyDefenseReason::Vacant;return r; }
            const auto& a=j.world.armies[r.guard];r.guarded=j.guarding[r.guard];
            r.attackSpent=j.chain.spent[r.guard] && !r.guarded;r.moved=j.chain.moved[r.guard];r.canGuard=!j.chain.spent[r.guard];
            if(a.troops<150 || a.morale<25) r.reason=JourneyDefenseReason::Depleted;
            else if(actual ? j.PositionSecured(goal):(r.threats==0 || (r.threats==1 && r.guarded && a.troops>=500 && a.morale>=40)))
            { r.secured=true;r.reason=JourneyDefenseReason::Secured; }
            else if(r.threats>1) r.reason=JourneyDefenseReason::Overwhelmed;
            else if(a.troops<500 || a.morale<40) r.reason=JourneyDefenseReason::GuardWeak;
            else if(r.attackSpent) r.reason=JourneyDefenseReason::GuardSpent;
            else r.reason=JourneyDefenseReason::NeedsGuard;
            return r;
        }
    }
    JourneyDefenseRead ReadJourneyDefense(const StrategistJourney& j,int goal) { return Inspect(j,goal,false); }
    JourneyDefenseRead ReadJourneyDefenseCheck(const StrategistJourney& j,int goal)
    {
        if(!Active(j,goal)) return {};
        const auto& r=j.defenseChecks[goal==j.goal ? 0:1];
        return r.goal==goal && r.turn==j.turn-1 ? r:JourneyDefenseRead{};
    }
    void StrategistJourney::RecordDefenseChecks()
    {
        defenseChecks={};if(Definition().kind!=JourneyObjective::HoldPositions) return;
        defenseChecks[0]=Inspect(*this,goal,true);defenseChecks[1]=Inspect(*this,Definition().secondGoal,true);
    }
}
