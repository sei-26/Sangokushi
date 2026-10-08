#include "Campaign.hpp"
namespace frontline
{
	int Campaign::AITargetArmy(int index) const
	{
		const auto& a = armies[index];
		int best = -1, score = -100000;
		for (int j = 0; j < static_cast<int>(armies.size()); ++j)
		{
			const auto& b = armies[j];
			if (b.troops <= 0 || !Hostile(a.faction, b.faction) || !CanStrike(index, b.tile))
				continue;
			const int value =
			    500 - b.troops / 30 + (b.arm == Arm::Transport ? 220 : 0) - Distance(a.tile, b.tile) * 25;
			if (value > score)
			{
				score = value;
				best = j;
			}
		}
		return best;
	}
	bool Campaign::AIUseTactic(int i, const SupplyGrid& supply) const
	{
		const auto& a = armies[i];
		const auto tactic = generals[a.general].tactic;
		if (a.troops <= 0 || a.arm == Arm::Transport || a.retreat || a.tacticLeft > 0 || a.tacticQueued ||
		    a.morale < 40 || a.tacticReadyDay > day)
			return false;
		if (tactic == Tactic::Supply)
		{
			const int city = supply[a.faction][a.tile];
			return city >= 0 && cities[city].food >= 1000 && (a.food < 300 || a.morale < 55);
		}
		if (tactic == Tactic::Rally)
		{
			int hurt = 0;
			for (const auto& b : armies)
				if (b.troops > 0 && b.faction == a.faction && Distance(a.tile, b.tile) <= 2 && b.morale < 70)
					++hurt;
			return hurt >= 2;
		}
		int nearby = 0;
		for (const auto& b : armies)
			if (b.troops > 0 && Hostile(a.faction, b.faction) && Distance(a.tile, b.tile) <= 2)
				nearby += b.troops;
		const int city = CityAt(a.target);
		const bool siege =
		    city >= 0 && Hostile(a.faction, cities[city].owner) && Distance(a.tile, a.target) <= 1;
		if (tactic == Tactic::Fortify)
			return nearby >= a.troops / 2 || siege;
		if (tactic == Tactic::Volley)
		{
			if (a.arm != Arm::Bow)
				return false;
			for (const auto& b : armies)
				if (b.troops > 0 && Hostile(a.faction, b.faction) && Distance(a.tile, b.tile) <= 3 &&
				    battle::ClearRay(a.tile % Width, a.tile / Width, b.tile % Width, b.tile / Width,
				                     [&](int x, int y) {
					                     return tiles[At(x, y)].terrain == Terrain::Mountain ||
					                            tiles[At(x, y)].terrain == Terrain::Forest;
				                     }))
					return true;
			return false;
		}
		if (tactic == Tactic::Fire)
			return nearby >= 1800 || siege;
		return AITargetArmy(i) >= 0 || siege;
	}
	void Campaign::AdvanceAIArmies()
	{
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			auto& a = armies[i];
			if (a.troops <= 0 || a.faction == player || a.arm == Arm::Transport)
				continue;
			const int targetCity = CityAt(a.target);
			const bool invalid = targetCity >= 0 && cities[targetCity].owner != a.faction &&
			                     !Hostile(a.faction, cities[targetCity].owner);
			const bool returnHome = a.retreat || a.troops < 1200 || a.morale < 30 ||
			                        (!a.supplied && a.food < 120) || invalid ||
			                        (targetCity >= 0 && cities[targetCity].owner == a.faction);
			if (returnHome)
			{
				const int here = CityAt(a.tile);
				if (here >= 0 && cities[here].owner == a.faction)
				{
					ReturnToCity(i, here);
					continue;
				}
				if (a.retreat && a.path.empty() && day % 5 != 0)
					continue;

				if (a.retreat && !a.path.empty() && targetCity >= 0 &&
				    cities[targetCity].owner == a.faction && day % 5 != 0)
					continue;
				std::vector<int> homes;
				for (int c = 0; c < static_cast<int>(cities.size()); ++c)
					if (cities[c].owner == a.faction && AIThreat(c) < cities[c].troops + 2000)
						homes.push_back(c);
				std::stable_sort(homes.begin(), homes.end(), [&](int x, int y) {
					return Distance(a.tile, cities[x].tile) < Distance(a.tile, cities[y].tile);
				});
				bool routed = false;
				for (int home : homes)
				{
					if (a.retreat && a.target == cities[home].tile && !a.path.empty())
					{
						routed = true;
						break;
					}
					if (ReturnToCity(i, home))
					{
						routed = true;
						break;
					}
				}
				if (!routed && !a.retreat)
				{
					Order(i, a.tile, true);
					Note(generals[a.general].name + U"隊は帰還路を失い、進軍を止めて生存を優先。");
				}
				continue;
			}
			if (day % 5 != 0)
				continue;
			// Respond with nearby forces. Distant armies keep their campaign objective.
			int defend = -1;
			for (int c = 0; c < static_cast<int>(cities.size()); ++c)
				if (cities[c].owner == a.faction && Distance(a.tile, cities[c].tile) <= 10 &&
				    AIThreat(c) > std::max(2000, cities[c].troops / 2))
				{
					defend = c;
					break;
				}
			if (defend >= 0)
			{
				int enemy = -1;
				for (int j = 0; j < static_cast<int>(armies.size()); ++j)
					if (armies[j].troops > 0 && Hostile(a.faction, armies[j].faction) &&
					    Distance(armies[j].tile, cities[defend].tile) <= 6 &&
					    (enemy < 0 || armies[j].troops > armies[enemy].troops))
						enemy = j;
				if (enemy >= 0 && a.target != armies[enemy].tile && Order(i, armies[enemy].tile))
					Note(generals[a.general].name + U"隊が" + cities[defend].name + U"の救援へ転進。");
				continue;
			}

			bool escorting = false;
			if (a.arm != Arm::Siege)
				for (int cargo = 0; cargo < static_cast<int>(armies.size()) && !escorting; ++cargo)
				{
					const auto& convoy = armies[cargo];
					if (convoy.troops <= 0 || convoy.arm != Arm::Transport || convoy.faction != a.faction ||
					    Distance(a.tile, convoy.tile) > 6)
						continue;
					int enemy = -1;
					for (int j = 0; j < static_cast<int>(armies.size()); ++j)
						if (armies[j].troops > 0 && armies[j].arm != Arm::Transport &&
						    Hostile(a.faction, armies[j].faction) &&
						    Distance(armies[j].tile, convoy.tile) <= 5 &&
						    (enemy < 0 || Distance(armies[j].tile, convoy.tile) <
						                      Distance(armies[enemy].tile, convoy.tile)))
							enemy = j;
					if (enemy < 0)
						continue;
					int nearest = i;
					for (int j = 0; j < static_cast<int>(armies.size()); ++j)
						if (armies[j].troops > 0 && armies[j].faction == a.faction &&
						    armies[j].arm != Arm::Transport && armies[j].arm != Arm::Siege &&
						    !armies[j].retreat &&
						    (Distance(armies[j].tile, convoy.tile) <
						         Distance(armies[nearest].tile, convoy.tile) ||
						     (Distance(armies[j].tile, convoy.tile) ==
						          Distance(armies[nearest].tile, convoy.tile) &&
						      j < nearest)))
							nearest = j;
					if (nearest != i)
						continue;
					escorting = true;
					if (a.target != armies[enemy].tile && Order(i, armies[enemy].tile))
						Note(generals[a.general].name + U"隊が輸送隊に迫る敵を迎撃。兵糧の到着を守る。");
				}
			if (escorting)
				continue;
			bool blocked = !a.path.empty() && CityAt(a.path.front()) >= 0 &&
			               cities[CityAt(a.path.front())].owner != a.faction && a.path.front() != a.target;
			if (a.path.empty() || blocked ||
			    (targetCity >= 0 && !Hostile(a.faction, cities[targetCity].owner)))
			{
				const int enemy = AIEnemyCity(a.tile, a.faction, a.arm, std::max(4000, a.troops));
				if (enemy >= 0 && Order(i, cities[enemy].tile))
					Note(generals[a.general].name + U"隊が到達可能な敵城へ作戦を更新。");
				else if (a.path.empty())
				{
					const int home = NearestCity(a.tile, a.faction, false);
					if (home >= 0)
						ReturnToCity(i, home);
				}
			}
		}
	}
} // namespace frontline
