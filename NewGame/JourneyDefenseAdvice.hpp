#pragma once
#include "JourneyOpening.hpp"
#include <optional>
namespace frontline
{
    class StrategistJourney;
    struct JourneyPlanExample;
    struct JourneyDefenseLesson
    {
        int mission=-1,failedGuards=0,endedAt=0;
        JourneyResolution resolution=JourneyResolution::None;
    };
    // A retry reference only. It is never a settled victory, reward, or inferred old outcome.
    std::optional<JourneyDefenseLesson> ReadDefenseRetryLesson(const StrategistJourney&,const StrategistJourney& beforeBattle);
    void AdaptDefenseRetryPlanExample(const StrategistJourney&,JourneyPlanExample&);
}
