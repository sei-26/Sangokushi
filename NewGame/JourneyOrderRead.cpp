#include "JourneyBattleWatch.hpp"

namespace frontline
{
    JourneyOrderRead ReadJourneyOrder(const StrategistJourney& journey,const JourneyRealtime& clock,int army)
    {
        JourneyOrderRead read;
        if (journey.stage!=JourneyStage::Battle || army<0 || army>=3 || !TacticChain::Living(journey.world,army)) return read;
        const auto& order=clock.orders[army];
        if (order.kind==JourneyDirective::Hold) { read.status=JourneyRouteStatus::Guarding;return read; }
        if (order.kind==JourneyDirective::Ambush) { read.status=JourneyRouteStatus::Ambushing;return read; }
        if (order.kind!=JourneyDirective::March && order.kind!=JourneyDirective::Attack) return read;
        if (order.kind==JourneyDirective::Attack)
        {
            const auto* target=journey.intelligence[0].Find(order.target);
            if (!target || !target->current || target->observed.troops<=0) { read.status=JourneyRouteStatus::LostTarget;return read; }
        }
        read.route=clock.Route(journey,army);
        read.status=read.route.empty() ? JourneyRouteStatus::NoRoute : read.route.size()>1 ? JourneyRouteStatus::Moving :
            order.kind==JourneyDirective::Attack ? JourneyRouteStatus::Engaging : JourneyRouteStatus::Idle;
        if (order.kind==JourneyDirective::March)
        {
            const auto plan=clock.MarchPlan(journey,army);read.route=plan.route;read.complete=plan.complete;
            if (read.status==JourneyRouteStatus::Idle && plan.points.size()>1) read.status=JourneyRouteStatus::Moving;
        }
        return read;
    }
}
