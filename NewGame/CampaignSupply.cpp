#include "Campaign.hpp"

namespace frontline
{
	std::array<int, TileCount> Campaign::Supply(int faction) const
	{
		std::array<int, TileCount> source;
		source.fill(-1);
		std::array<bool, TileCount> blocked{};
		for (const auto& a : armies)
			if (a.troops > 0 && Hostile(a.faction, faction))
				blocked[a.tile] = true;
		std::queue<int> q;
		for (int c = 0; c < static_cast<int>(cities.size()); ++c)
			if (cities[c].owner == faction && cities[c].food > 0 && !blocked[cities[c].tile])
			{
				source[cities[c].tile] = c;
				q.push(cities[c].tile);
			}
		while (!q.empty())
		{
			const int p = q.front();
			q.pop();
			for (int n : Neighbors(p))
				if (source[n] < 0 && !blocked[n] && tiles[n].owner == faction && Cost(n, Arm::Spear) < 100000)
				{
					source[n] = source[p];
					q.push(n);
				}
		}
		return source;
	}
} // namespace frontline

namespace frontline
{
	void Campaign::ConsumeDailySupply(const SupplyGrid& supply)
	{
		std::vector<officer::Link> supplyLinks;
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
			supplyLinks.push_back(Formation(i));
		for (auto& a : armies)
		{
			if (a.troops <= 0)
				continue;
			const int source = supply[a.faction][a.tile];
			const int efficiency = source >= 0 ? cities[source].logistics / 5 : 0;
			const int baseNeed =
			    a.troops / (40 + efficiency + (generals[a.general].specialty == Duty::Logistics ? 10 : 0) +
			                (generals[a.general].trait == Trait::Quartermaster ? 10 : 0));
			const int need = std::max(
			    20, baseNeed * (100 - supplyLinks[static_cast<size_t>(&a - armies.data())].supply) / 100);
			a.supplied = source >= 0 && cities[source].food >= need;
			if (a.supplied)
			{
				cities[source].food -= need;
				a.morale = std::min(100, a.morale + 2);
			}
			else if (a.food >= need)
			{
				a.food -= need;
				a.morale = std::max(0, a.morale - 1);
			}
			else
			{
				a.food = 0;
				a.morale = std::max(0, a.morale - 8);
				a.troops = std::max(0, a.troops - std::max(20, a.troops / 30));
			}
			if (a.troops <= 0)
			{
				a.tacticQueued = false;
				generals[a.general].readyDay = day + 20;
				Note(generals[a.general].name + U"隊が兵糧不足で崩壊。");
				continue;
			}
			if (a.morale < 25 && !a.retreat)
			{
				const int home = NearestCity(a.tile, a.faction, false);
				if (home >= 0)
				{
					const int index = static_cast<int>(&a - armies.data());
					Order(index, cities[home].tile, true);
					Note(generals[a.general].name + U"隊が撤退を開始。");
				}
			}
		}
	}
} // namespace frontline
