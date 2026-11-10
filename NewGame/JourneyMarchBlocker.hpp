#pragma once
namespace frontline
{
    class StrategistJourney;
    class JourneyRealtime;
    struct JourneyMarchBlocker
    {
        int army=-1,tile=-1;
    };
    // Reads the first leg using the same observed board as actual route previews.
    JourneyMarchBlocker ReadJourneyMarchBlocker(const StrategistJourney&,const JourneyRealtime&,int army);
}
