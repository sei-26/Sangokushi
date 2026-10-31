#include "JourneyRealtime.hpp"
#include <cmath>

namespace frontline
{
    void JourneyRealtime::Reset() { *this=JourneyRealtime{}; }
    void JourneyRealtime::Advance(double seconds)
    {
        if (paused || !std::isfinite(seconds) || seconds<=0) return;
        const double delta=seconds*std::clamp(speed,.5,2.);
        elapsed+=delta;activeSeconds+=delta;
    }
    void JourneyRealtime::Consume(bool singleStep)
    {
        if (singleStep) { activeSeconds+=std::max(0.,Period-elapsed);elapsed=0; }
        else elapsed=std::max(0.,elapsed-Period);
        ++pulses;
    }
    bool JourneyRealtime::Queue(const StrategistJourney& journey,int army,JourneyDirective kind,int value)
    {
        if (journey.stage!=JourneyStage::Battle || army<0 || army>=3 || !TacticChain::Living(journey.world,army)) return false;
        if (kind==JourneyDirective::March && !Ground(journey.world,value)) return false;
        if (kind==JourneyDirective::Attack)
        {
            const auto* seen=journey.intelligence[0].Find(value);
            if (!seen || !seen->current || seen->observed.troops<=0) return false;
        }
        if (kind==JourneyDirective::Ambush && journey.world.generals[journey.world.armies[army].general].tactic!=Tactic::Ambush) return false;
        orders[army]={kind,kind==JourneyDirective::March ? value : -1,kind==JourneyDirective::Attack ? value : -1};
        return true;
    }
    void JourneyRealtime::Cancel(int army) { if (army>=0 && army<3) orders[army]={}; }
    bool JourneyRealtime::Resolve(StrategistJourney& journey, const std::array<bool,3>& reservedAttacks)
    {
        if (journey.stage!=JourneyStage::Battle) return false;
        const auto previousReports=journey.reports.size();
        journey.ValidateReliefOrders(*this);
        journey.RefreshIntelligence();
        // Capture enemy decisions before any friendly movement. The player UI
        // receives Route(), never these private plans.
        auto enemyPlans=journey.PlanEnemyTurn();
        const auto& world=journey.world;
        std::vector<int> from,next;
        for (const auto& unit : world.armies) { from.push_back(unit.tile);next.push_back(unit.tile); }
        for (int army=0;army<3;++army)
        {
            if (!TacticChain::Living(world,army) || journey.chain.moved[army]) continue;
            const auto route=Route(journey,army);
            if (route.size()>1) next[army]=route[1];
        }
        for (const auto& plan : enemyPlans) next[plan.army]=plan.next;
        const auto desired=next;
        for (int army=0;army<static_cast<int>(next.size());++army)
        {
            if (next[army]==from[army] || !TacticChain::Living(world,army)) continue;
            for (int other=0;other<static_cast<int>(next.size());++other)
                if (other!=army && TacticChain::Living(world,other) &&
                    (desired[army]==from[other] || (desired[other]!=from[other] && desired[army]==desired[other])))
                { next[army]=from[army];break; }
        }
        for (int army=0;army<3;++army)
            if (next[army]!=from[army])
                if(journey.chain.Move(journey.world,journey.intelligence[0],army,next[army],journey.message))
                    journey.RecordReliefMove(army,from[army],next[army]);
        for (auto& plan : enemyPlans) plan.next=next[plan.army];
        journey.ApplyEnemyPlans(enemyPlans);
        for (int army=0;army<3 && journey.stage==JourneyStage::Battle;++army)
        {
            auto& order=orders[army];
            if (!TacticChain::Living(world,army)) { order={};continue; }
            AdvanceMarch(world,army);
            if (journey.chain.spent[army]) continue;
            if (order.kind==JourneyDirective::Hold && !journey.chain.moved[army]) journey.Hold(army);
            if (order.kind==JourneyDirective::Ambush && !journey.chain.concealed[army]) journey.Ambush(army);
            if (order.kind==JourneyDirective::Attack && !reservedAttacks[army])
            {
                const auto* target=journey.intelligence[0].Find(order.target);
                if (target && target->current && world.CanStrike(army,target->observed.tile)) journey.chain.Strike(journey.world,journey.intelligence[0],army,order.target,journey.message);
            }
        }
        return journey.stage!=JourneyStage::Battle || journey.CompleteCommandCycle(previousReports);
    }
}
