#pragma once
#include "JourneyRealtime.hpp"
namespace frontline
{
    struct JourneySupportChoice
    {
        int army=-1,goal=-1,destination=-1,guard=-1,blocker=-1,steps=0,arrival=0,brace=1,enemies=0,adjacentEnemies=0,priority=0;
        bool available=false,cover=false,uncertain=false,breaksConcealment=false;
        std::vector<int> route;
    };
    // Current public contacts and friendly orders only. Arrival is a lower bound, never a guaranteed guard result.
    std::vector<JourneySupportChoice> ReadJourneySupport(const StrategistJourney&,const JourneyRealtime&,int army);
    std::vector<int> JourneySupportContext(const StrategistJourney&,const JourneyRealtime&,int army);
    bool QueueJourneySupport(const StrategistJourney&,JourneyRealtime&,const JourneySupportChoice& shown);
}
