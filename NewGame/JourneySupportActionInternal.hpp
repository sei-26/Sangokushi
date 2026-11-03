#pragma once
#include "JourneySupportActions.hpp"
namespace frontline::supportactions
{
    int Threats(const StrategistJourney&,int goal,int target=-1,int tile=-1,int troops=0);
    int Effect(const StrategistJourney&,const JourneySupportAction&);
    int Score(const StrategistJourney&,const JourneySupportAction&);
    bool GuardReady(const StrategistJourney&,const JourneyRealtime&,int guard,int goal);
    JourneySupportAction Preview(const StrategistJourney&,int supporter,int goal,int guard,const std::vector<ChainOrder>&);
    std::vector<JourneySupportAction> Combinations(const StrategistJourney&,const JourneyRealtime&,int actor,const std::vector<JourneySupportAction>&);
    std::vector<JourneySupportAction> Followups(const StrategistJourney&,const JourneyRealtime&,int actor);
    void AddProvenanceKey(const StrategistJourney&,std::vector<int>&);
}
