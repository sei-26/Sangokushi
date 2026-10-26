#pragma once
#include "TacticChain.hpp"
#include "JourneyMission.hpp"

namespace frontline
{
	enum class JourneyEnemyIntent { Wait, Engage, Guard, Search, Intercept, Assault, Recover };
	struct JourneyEnemyContext
	{
		JourneyObjective objective = JourneyObjective::Breakout;
		int goal = -1, secondGoal = -1, policy = 0;
		bool convoy = false;
	};
	struct JourneyEnemyPlan
	{
		int army = -1, from = -1, next = -1, target = -1, targetArmy = -1;
		JourneyEnemyIntent intent = JourneyEnemyIntent::Wait;
		std::vector<int> danger;
	};
	// The caller supplies an observed battlefield, never the omniscient board.
	// Stable unit order also reserves destinations, preventing two planned moves
	// into the same cell. Real movement must still validate physical collision.
	std::vector<JourneyEnemyPlan> PlanJourneyEnemies(const Campaign& observed,
	    const TacticChain& chain, const JourneyEnemyContext& context);
	const char32_t* EnemyIntentName(JourneyEnemyIntent intent);
	const char32_t* EnemyIntentCounter(JourneyEnemyIntent intent);
}
