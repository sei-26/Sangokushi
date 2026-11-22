#pragma once
#include "JourneyAssignment.hpp"
#include "JourneyCouncilStage.hpp"
namespace frontline
{
    class StrategistJourney;
    struct PreludeOpeningPlan
    {
        const char32_t* name;
        const char32_t* roles;
        const char32_t* caution;
        std::array<JourneyAssignment,3> assignments;
    };
    bool CanChoosePreludeOpening(const StrategistJourney&);
    const PreludeOpeningPlan& PreludeOpening(int choice);
    bool MatchesPreludeOpening(const StrategistJourney&,int choice);
    // Explicitly replaces the three duties atomically. Preparation is paid only on deployment.
    bool AdoptPreludeOpening(StrategistJourney&,int choice);
    CouncilExchange PreludeOpeningCouncil(const StrategistJourney&,int accepted=-1);
    JourneyEpisode PreludeOpeningAftermath(const StrategistJourney&);
}
