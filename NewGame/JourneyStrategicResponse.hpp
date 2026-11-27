#pragma once
#include "StrategistJourney.hpp"
namespace frontline
{
    // Both real enemy planning and the player's estimate pass their own observed board.
    void ApplyGuangzongFlankResponse(const Campaign&,const TacticChain&,int goal,std::vector<JourneyEnemyPlan>&);
    int GuangzongVeteranOfficer(const StrategistJourney&);
    bool HasGuangzongVeterans(const StrategistJourney&);
    // Names of officers whose tactic action remains after the proposed orders.
    std::u32string RemainingOfficers(const StrategistJourney&,const std::vector<ChainOrder>&);
}
