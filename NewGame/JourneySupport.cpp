#include "JourneySupport.hpp"
namespace frontline
{
    std::vector<JourneySupportChoice> ReadJourneySupport(const StrategistJourney& j,const JourneyRealtime& clock,int army)
    {
        std::vector<JourneySupportChoice> choices;
        if(j.stage!=JourneyStage::Battle || j.Definition().kind!=JourneyObjective::HoldPositions || army<0 || army>=3 || !TacticChain::Living(j.world,army) ||
           j.assignments[army].duty!=JourneyDuty::None || clock.orders[army].kind!=JourneyDirective::None || j.guarding[army] ||
           j.world.armies[army].tile==j.goal || j.world.armies[army].tile==j.Definition().secondGoal) return choices;
        for(int goal:{j.goal,j.Definition().secondGoal})
        {
            JourneySupportChoice c;c.army=army;c.goal=goal;c.destination=goal;c.breaksConcealment=j.chain.concealed[army];
            for(int id=0;id<3;++id) if(id!=army && TacticChain::Living(j.world,id) && j.world.armies[id].tile==goal) { c.guard=id;break; }
            if(c.guard<0) for(int id=0;id<3;++id)
            {
                const auto& o=clock.orders[id];const int last=o.waypoints.empty() ? o.destination:o.waypoints.back();
                if(id!=army && TacticChain::Living(j.world,id) && o.kind==JourneyDirective::March && o.arrival==JourneyArrival::Hold && last==goal) { c.guard=id;break; }
            }
            c.cover=c.guard>=0;
            if(c.cover)
            {
                const auto approach=clock.ReliefApproach(j,army,goal);
                c.destination=approach.staging;c.route=approach.route;c.blocker=approach.blocker;
                c.available=approach.state==JourneyApproachState::Clear || approach.state==JourneyApproachState::Adjacent;
                if(c.available)
                {
                    // Preview exactly the ordinary route that will execute, including a guard still en route.
                    c.available=false;c.route.clear();
                    for(int tile:j.world.MapNeighbors(goal))
                    {
                        auto candidate=clock;if(!candidate.Queue(j,army,JourneyDirective::March,tile)) continue;
                        auto path=candidate.Route(j,army);
                        if(path.empty() || std::find(path.begin(),path.end(),goal)!=path.end()) continue;
                        if(c.route.empty() || path.size()<c.route.size()) { c.route=std::move(path);c.destination=tile;c.available=true; }
                    }
                }
            }
            else
            {
                auto route=clock;
                if(route.Queue(j,army,JourneyDirective::March,goal)) c.route=route.Route(j,army);
                c.available=!c.route.empty();
            }
            c.steps=c.route.empty() ? 0:int(c.route.size())-1;
            c.arrival=c.steps+(c.steps>0 && j.chain.moved[army] ? 1:0);
            c.brace=c.steps>0 ? c.arrival+1:(j.chain.moved[army] || j.chain.spent[army] ? 2:1);
            for(int tile:c.route) if(!j.intelligence[0].visible[tile]) c.uncertain=true;
            for(const auto& seen:j.intelligence[0].contacts)
                if(seen.current && seen.observed.troops>0 && j.world.Hostile(j.world.player,seen.observed.faction))
                { const int distance=j.world.MapDistance(seen.observed.tile,goal);c.enemies+=distance<=2;c.adjacentEnemies+=distance<=1; }
            bool ready=false;
            if(c.guard>=0)
            {
                const auto& guard=j.world.armies[c.guard];
                ready=guard.tile==goal && guard.troops>=150 && guard.morale>=25 && (c.adjacentEnemies==0 ||
                    (c.adjacentEnemies==1 && j.guarding[c.guard] && guard.troops>=500 && guard.morale>=40));
            }
            c.priority=(ready ? 0:4)+c.enemies+2*c.adjacentEnemies;
            choices.push_back(std::move(c));
        }
        std::stable_sort(choices.begin(),choices.end(),[](const auto& a,const auto& b)
        { if(a.available!=b.available) return a.available;if(a.priority!=b.priority) return a.priority>b.priority;return a.arrival<b.arrival; });
        return choices;
    }
    bool QueueJourneySupport(const StrategistJourney& j,JourneyRealtime& clock,const JourneySupportChoice& shown)
    {
        const auto current=ReadJourneySupport(j,clock,shown.army);
        const auto found=std::find_if(current.begin(),current.end(),[&](const auto& c) { return c.goal==shown.goal; });
        if(found==current.end() || !found->available || !shown.available || found->destination!=shown.destination || found->guard!=shown.guard ||
           found->route!=shown.route || found->arrival!=shown.arrival || found->brace!=shown.brace || found->enemies!=shown.enemies || found->adjacentEnemies!=shown.adjacentEnemies ||
           found->breaksConcealment!=shown.breaksConcealment || found->uncertain!=shown.uncertain) return false;
        auto candidate=clock;
        if(!candidate.Queue(j,shown.army,JourneyDirective::March,found->destination) || !candidate.SetArrival(j,shown.army,JourneyArrival::Hold)) return false;
        candidate.paused=true;clock=std::move(candidate);return true;
    }
}
