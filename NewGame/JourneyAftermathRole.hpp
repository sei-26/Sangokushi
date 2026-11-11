#pragma once
#include "JourneyEpisode.hpp"
namespace frontline
{
    struct JourneyRoleAftermath
    {
        std::u32string summary;
        std::vector<JourneyEpisodeLine> lines;
    };
    // Settled/public contributions only. Reading cannot award trust or change history.
    JourneyRoleAftermath ReadRoleAftermath(const StrategistJourney&);
    JourneyRoleAftermath ReadContributionAftermath(const StrategistJourney&);
    JourneyRoleAftermath ReadDefenseAftermath(const StrategistJourney&);
    JourneyRoleAftermath ReadDutyAftermath(const StrategistJourney&);
}
