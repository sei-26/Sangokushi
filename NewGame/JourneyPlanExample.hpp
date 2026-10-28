#pragma once
#include "JourneyAssignment.hpp"
namespace frontline
{
    class StrategistJourney;
    struct JourneyPlanExample
    {
        std::u32string name,purpose,manualTask,caution;
        std::array<JourneyAssignment,3> assignments{};
    };
    JourneyPlanExample PlanExample(const StrategistJourney&);
}
