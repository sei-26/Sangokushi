#pragma once
#include "JourneyAssignment.hpp"
namespace frontline
{
    class StrategistJourney;
    struct JourneyPlanExample
    {
        std::u32string name,purpose,manualTask,caution,reason;
        int adviser=-1;
        std::array<JourneyAssignment,3> assignments{};
    };
    JourneyPlanExample PlanExample(const StrategistJourney&);
}
