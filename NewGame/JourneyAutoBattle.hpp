#pragma once
#include "JourneyRealtime.hpp"
namespace frontline
{
    struct JourneyAutoPlan
    {
        JourneyRealtime movement;
        std::vector<ChainOrder> tactics;
        std::array<std::u32string,3> reasons;
    };
    // Only current friendly state, mission data and player contacts inform decisions.
    // Produces normal orders; it never resolves actions or grants resources.
    JourneyAutoPlan PlanJourneyAutoBattle(const StrategistJourney&);
}
