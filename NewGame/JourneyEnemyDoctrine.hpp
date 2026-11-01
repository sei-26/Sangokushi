#pragma once
#include "JourneyEnemyPlan.hpp"
namespace frontline
{
    // Transient mission roles. The supplied world contains only the observer's
    // contacts; absent armies are unknown, not evidence of a destroyed unit.
    void ApplyJourneyEnemyDoctrine(const Campaign& observed,const JourneyEnemyContext&,JourneyEnemyPlan&);
    std::u32string EnemyDoctrineHint(const JourneyEnemyPlan&,const JourneyEnemyContext&);
}
