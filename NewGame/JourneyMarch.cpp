#include "JourneyRealtime.hpp"

namespace frontline
{
    bool JourneyRealtime::AppendWaypoint(const StrategistJourney& journey,int army,int tile)
    {
        if (journey.stage!=JourneyStage::Battle || army<0 || army>=3 || !TacticChain::Living(journey.world,army) || !Ground(journey.world,tile)) return false;
        auto& order=orders[army];
        if (order.kind!=JourneyDirective::March) return Queue(journey,army,JourneyDirective::March,tile);
        const int last=order.waypoints.empty() ? order.destination : order.waypoints.back();
        if (order.waypoints.size()+1>=MaxMarchPoints || tile==last) return false;
        if (order.arrival==JourneyArrival::Ambush && journey.world.tiles[tile].terrain!=Terrain::Forest) return false;
        order.waypoints.push_back(tile);return true;
    }
    bool JourneyRealtime::UndoWaypoint(int army)
    {
        if (army<0 || army>=3 || orders[army].kind!=JourneyDirective::March) return false;
        auto& order=orders[army];
        if (order.waypoints.empty()) { Cancel(army);return true; }
        order.waypoints.pop_back();
        // Undo changes the endpoint. Return to waiting rather than leave an
        // ambush scheduled on a plain tile that was only a transit point.
        order.arrival=JourneyArrival::Wait;return true;
    }
    bool JourneyRealtime::SetArrival(const StrategistJourney& journey,int army,JourneyArrival arrival)
    {
        if (journey.stage!=JourneyStage::Battle || army<0 || army>=3 || !TacticChain::Living(journey.world,army) || orders[army].kind!=JourneyDirective::March) return false;
        auto& order=orders[army];
        const int last=order.waypoints.empty() ? order.destination : order.waypoints.back();
        if (arrival==JourneyArrival::Ambush && (journey.world.generals[journey.world.armies[army].general].tactic!=Tactic::Ambush || journey.world.tiles[last].terrain!=Terrain::Forest)) return false;
        order.arrival=arrival;return true;
    }
    void JourneyRealtime::AdvanceMarch(const Campaign& world,int army)
    {
        auto& order=orders[army];
        if (order.kind!=JourneyDirective::March || world.armies[army].tile!=order.destination) return;
        if (!order.waypoints.empty())
        { order.destination=order.waypoints.front();order.waypoints.erase(order.waypoints.begin());return; }
        const auto arrival=order.arrival;order={};
        order.kind=arrival==JourneyArrival::Hold ? JourneyDirective::Hold : arrival==JourneyArrival::Ambush ? JourneyDirective::Ambush : JourneyDirective::None;
    }
}
