#pragma once
#include "JourneyBattleChoices.hpp"
#include "JourneyRealtime.hpp"
namespace frontline
{
    struct JourneySupportAction
    {
        JourneyBattleChoice plan;
        int goal=-1,guard=-1,supporter=-1,actorTo=-1,enemyTo=-1,enemyMoraleLoss=0,threatsBefore=0,threatsAfter=0;
        bool clearsThreat=false,guardActs=false,followup=false;
    };
    // Ordinary solo/paired tactics from current observations. A lower local threat count is not a secured objective.
    std::vector<JourneySupportAction> ReadJourneySupportActions(const StrategistJourney&,const JourneyRealtime&,int actor);
    std::vector<int> JourneySupportActionContext(const StrategistJourney&,const JourneyRealtime&,int actor);
}
