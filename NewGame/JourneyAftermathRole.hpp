#pragma once
#include "JourneyEpisode.hpp"
namespace frontline
{
    struct JourneyRoleAftermath
    {
        std::u32string summary;
        std::vector<JourneyEpisodeLine> lines;
    };
    // Settled, executed handovers only. Reading cannot award trust or change history.
    JourneyRoleAftermath ReadRoleAftermath(const StrategistJourney&);
}
