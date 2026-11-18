#include "JourneyAutoBattle.hpp"
#include <limits>

namespace frontline
{
    void CoordinateJourneyAutoEngagement(const StrategistJourney& journey,JourneyAutoPlan& plan,
                                        const std::array<int,3>& duties)
    {
        const auto& world=journey.world;
        int focus=-1;
        int bestScore=std::numeric_limits<int>::min();
        for(const auto& contact:journey.intelligence[0].contacts)
        {
            if(!contact.current || contact.observed.troops<=0 ||
               !world.Hostile(world.player,contact.observed.faction) ||
               (journey.depotInterception && contact.id==4)) continue;
            int fighters=0;
            int reachable=0;
            int travel=0;
            for(int officer=0;officer<3;++officer)
            {
                if(duties[officer]>=0 || !TacticChain::Living(world,officer)) continue;
                auto movement=plan.movement;
                movement.Queue(journey,officer,JourneyDirective::Attack,contact.id);
                const auto route=movement.Route(journey,officer);
                if(route.empty()) continue;
                ++reachable;
                const int steps=int(route.size())-1;
                if(steps<=3) ++fighters;
                travel+=std::min(steps,12);
            }
            if(!reachable) continue;
            int score=fighters*90-travel*12;
            if(contact.observed.troops<600) score+=35;
            if(journey.Definition().kind==JourneyObjective::CaptureCamp && !journey.depotInterception)
                score+=std::max(0,4-world.MapDistance(contact.observed.tile,journey.goal))*110;
            if(journey.HasConvoy())
            {
                const auto& path=journey.ConvoyPath();
                const int next=path[std::min(journey.convoyStep+1,int(path.size())-1)];
                score+=std::max(0,4-world.MapDistance(contact.observed.tile,next))*90;
            }
            if(journey.Definition().kind==JourneyObjective::HoldPositions)
            {
                const int distance=std::min(world.MapDistance(contact.observed.tile,journey.goal),
                    world.MapDistance(contact.observed.tile,journey.Definition().secondGoal));
                score+=std::max(0,3-distance)*90;
            }
            if(journey.HasSunJian() && !journey.SunJianAtRally() &&
               world.MapDistance(contact.observed.tile,world.armies[StrategistJourney::AlliedArmy].tile)<=1) score+=180;
            if(score>bestScore)
            {
                focus=contact.id;
                bestScore=score;
            }
        }
        if(focus<0) return;
        for(int officer=0;officer<3;++officer)
        {
            if(duties[officer]>=0 || !TacticChain::Living(world,officer)) continue;
            auto movement=plan.movement;
            movement.Queue(journey,officer,JourneyDirective::Attack,focus);
            const auto route=movement.Route(journey,officer);
            if(route.empty()) continue;
            const auto current=plan.movement.Route(journey,officer);
            // Keep an engaged front if joining the main attack would expose it
            // to a free pursuit. Distant units can instead join its relief.
            if(!current.empty() && route.size()>current.size()+1) continue;
            plan.movement.orders[officer]=movement.orders[officer];
            plan.reasons[officer]=U"仲間と同じ敵へ。隊を集め、戦法をつなぐ！";
        }
    }
}
