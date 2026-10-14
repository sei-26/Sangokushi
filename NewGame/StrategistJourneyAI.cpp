#include "StrategistJourney.hpp"

namespace frontline
{
	void StrategistJourney::EnemyTurn()
	{
		RefreshIntelligence();
		// Decisions use current observed player units. Hidden reserves are absent
		// from this list, and disordered opponents spend the phase regrouping.
		std::vector<int> destinations(world.armies.size(), -1);
		for (int i = 3; i < static_cast<int>(world.armies.size()); ++i)
		{
			const auto& enemy = world.armies[i];
			if (!TacticChain::Living(world, i) || chain.disordered[i])
				continue;
			int target = -1, distance = 100;
			for (const auto& contact : intelligence[1].contacts)
				if (contact.current)
				{
					const int d = world.MapDistance(enemy.tile, contact.observed.tile);
					if (d < distance)
					{
						distance = d;
						target = contact.observed.tile;
					}
				}
			if (target < 0 || distance <= 1 || (enemyPolicy == 1 && i == 3))
				continue;
			int best = distance;
			for (int tile : world.MapNeighbors(enemy.tile))
				if (OnMap(tile) && TacticChain::Free(world, tile) && world.MapDistance(tile, target) < best)
				{
					best = world.MapDistance(tile, target);
					destinations[i] = tile;
				}
		}
		for (int i = 3; i < static_cast<int>(world.armies.size()); ++i)
			if (destinations[i] >= 0 && TacticChain::Free(world, destinations[i]))
				world.armies[i].tile = world.armies[i].target = destinations[i];
		RefreshIntelligence();
	}
	bool StrategistJourney::EndTurn()
	{
		if (stage != JourneyStage::Battle)
			return false;
		EnemyTurn();
		// Ordinary clashes, supply consumption and morale still use the shared
		// Campaign engine. Prepared ambushers sacrifice their ordinary offense.
		for (int i = 0; i < 3; ++i)
			world.armies[i].scouting = chain.concealed[i];
		world.AdvanceBattleDay();
		for (int i = 0; i < 3; ++i)
			world.armies[i].scouting = false;
		++turn;
		RefreshIntelligence();
		Evaluate();
		if (stage == JourneyStage::Battle)
		{
			chain.BeginTurn(world);
			Report(U"敵が動き、接近部隊が交戦した。新しい配置から次の連鎖を考えよう。");
		}
		return true;
	}
} // namespace frontline
