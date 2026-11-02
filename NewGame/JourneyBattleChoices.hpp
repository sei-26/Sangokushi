#pragma once
#include "StrategistJourney.hpp"
namespace frontline
{
    struct JourneyBattleChoice
    {
        std::vector<ChainOrder> orders;
        int target=-1,damage=0,moraleCost=0,friendlyLoss=0;
        bool disordered=false,pressured=false;
    };
    // Public observation key for a display cache and stale-choice protection.
    std::vector<int> BattleChoiceContext(const StrategistJourney&,int target);
    // At most two distinct choices: one ordinary tactic, or an ordinary combo.
    // Team proposals may include a visible retreat; costs are part of the choice.
    // Never spends a once-per-battle signature.
    std::vector<JourneyBattleChoice> ReadJourneyBattleChoices(const StrategistJourney&,int target);
}
