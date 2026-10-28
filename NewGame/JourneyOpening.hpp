#pragma once
#include "CampaignTypes.hpp"
#include <vector>

namespace frontline
{
    enum class JourneyConvoyRoute { Road, SouthernWood };
    struct JourneyConvoyPlan
    {
        const char32_t* name;
        const char32_t* description;
        int guideGold;
        std::vector<int> tiles; // Includes the starting cell.
    };
    const JourneyConvoyPlan& ConvoyPlan(JourneyConvoyRoute);
    enum class JourneyResolution { None, EnemyBroken, ExitSecured, Delivered, Held, Withdrawn, ForceLost, ConvoyLost, Deadline, Captured };
    struct JourneyBattleReview
    {
        JourneyResolution resolution=JourneyResolution::None;
        int endedAt=0,enemyStops=0,trafficStops=0,tacticAttempts=0,tacticSteps=0,longestChain=0;
        int coordinatedOfficers=0;
        bool preparationTracked=false;
        int openingAmbushes=0,resupplyTurn=0,resupplyMorale=0,postSupplySteps=0;
    };
}
