#include "Campaign.hpp"

namespace frontline
{
	bool Campaign::ActivateTactic(int index, bool ai)
	{
		if (index < 0 || index >= static_cast<int>(armies.size()))
			return false;
		auto& a = armies[index];
		if (a.arm == Arm::Transport || a.troops <= 0 || (!ai && a.faction != player) || a.tacticQueued ||
		    a.tacticReadyDay > day || a.morale < 30)
			return false;
		const auto t = generals[a.general].tactic;
		if (t == Tactic::Volley && a.arm != Arm::Bow)
			return false;
		if (t == Tactic::Fire)
		{
			const int city = CityAt(a.target);
			bool target =
			    city >= 0 && Hostile(a.faction, cities[city].owner) && MapDistance(a.tile, a.target) <= 1;
			for (const auto& b : armies)
				if (b.troops > 0 && Hostile(a.faction, b.faction) && MapDistance(a.tile, b.tile) <= 2)
					target = true;
			if (!target)
				return false;
		}
		if (t == Tactic::Supply)
		{
			const int source = Supply(a.faction)[a.tile];
			if (source < 0 || cities[source].food < 300)
				return false;
		}
		a.tacticQueued = true;
		return true;
	}

	int Campaign::Fronts(int tile, int faction) const
	{
		int count = 0;
		for (int n : MapNeighbors(tile))
			if (std::any_of(armies.begin(), armies.end(), [&](const Army& a) {
				    return a.arm != Arm::Transport && a.troops > 0 && a.faction == faction && a.tile == n;
			    }))
				++count;
		return count;
	}
} // namespace frontline
