#include "Campaign.hpp"

namespace frontline
{
	std::vector<bool> Campaign::AdvanceAIOperations()
	{
		std::vector<bool> holding(armies.size());
		// Evaluate cohesion from one snapshot so iteration order does not change the group.
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			const auto& a = armies[i];
			const int goal = CityAt(a.target);
			if (a.troops < 1200 || a.faction == player || a.retreat || a.arm == Arm::Transport || goal < 0 ||
			    !Hostile(a.faction, cities[goal].owner))
				continue;
			int partner = -1, gap = 100000;
			for (int j = 0; j < static_cast<int>(armies.size()); ++j)
			{
				const auto& b = armies[j];
				if (j == i || b.faction != a.faction || b.target != a.target || b.retreat ||
				    b.troops < 1200 || b.morale < 40 || (!b.supplied && b.food < 120) ||
				    b.arm == Arm::Transport ||
				    (a.arm == Arm::Siege ? b.arm == Arm::Siege : b.arm != Arm::Siege))
					continue;
				const int distance = MapDistance(a.tile, b.tile);
				if (distance >= gap)
					continue;
				// A nearby unit behind an impassable barrier is not a usable escort.
				if (a.tile != b.tile && Route(a.tile, b.tile, a.arm, a.faction).empty())
					continue;
				partner = j;
				gap = distance;
			}
			if (a.arm == Arm::Siege)
				holding[i] = partner < 0 || (gap > 4 && MapDistance(a.tile, a.target) <
				                                            MapDistance(armies[partner].tile, a.target));
			else if (partner >= 0 && gap > 4 && MapDistance(a.tile, a.target) > 3)
				holding[i] = MapDistance(a.tile, a.target) < MapDistance(armies[partner].tile, a.target);
		}
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			auto& a = armies[i];
			if (!holding[i])
			{
				if (a.aiAssemblyDays > 0 && a.troops > 0 && !a.retreat)
					Note(generals[a.general].name + U"隊が集結を終え、連携進軍を再開。");
				a.aiAssemblyDays = 0;
				continue;
			}
			if (a.aiAssemblyDays++ == 0)
				Note(generals[a.general].name + (a.arm == Arm::Siege
				                                     ? U"隊は攻城支援の集結を待つ。単独進軍を控える。"
				                                     : U"隊は攻城隊の到着を待つ。先行しすぎず戦列を保つ。"));
			if (a.aiAssemblyDays < 20)
				continue;
			Note(generals[a.general].name + U"隊は20日以内に戦列を揃えられず、攻勢を中止。");
			std::vector<int> homes;
			for (int c = 0; c < static_cast<int>(cities.size()); ++c)
				if (cities[c].owner == a.faction && AIThreat(c) < cities[c].troops + 2000)
					homes.push_back(c);
			std::stable_sort(homes.begin(), homes.end(), [&](int x, int y) {
				return MapDistance(a.tile, cities[x].tile) < MapDistance(a.tile, cities[y].tile);
			});
			bool returning = false;
			for (int home : homes)
				if (ReturnToCity(i, home))
				{
					returning = true;
					break;
				}
			if (!returning)
				Order(i, a.tile, true);
			a.aiAssemblyDays = 0;
			holding[i] = false;
		}
		return holding;
	}
} // namespace frontline
