#include "Campaign.hpp"

namespace frontline
{
	// 異動は兵を使わずに人材を配置する命令。出発時に味方領地の連絡路が必要。
	int Campaign::AssignmentDays(int from, int to) const
	{
		if (from < 0 || to < 0 || from >= static_cast<int>(cities.size()) ||
		    to >= static_cast<int>(cities.size()) || from == to || cities[from].owner != cities[to].owner)
			return -1;
		const int faction = cities[from].owner, start = cities[from].tile, goal = cities[to].tile;
		std::array<int, TileCount> distance;
		distance.fill(1000000);
		std::array<bool, TileCount> blocked{};
		for (const auto& army : armies)
			if (army.troops > 0 && Hostile(faction, army.faction))
				blocked[army.tile] = true;
		using Entry = std::pair<int, int>;
		std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pending;
		distance[start] = 0;
		pending.push({0, start});
		while (!pending.empty())
		{
			const auto [cost, tile] = pending.top();
			pending.pop();
			if (cost != distance[tile])
				continue;
			if (tile == goal)
				return std::clamp((cost + 5) / 6, 3, 180);
			for (int next : Neighbors(tile))
			{
				if (tiles[next].owner != faction || blocked[next] || Cost(next, Arm::Spear) >= 100000)
					continue;
				const int value = cost + Cost(next, Arm::Spear);
				if (value < distance[next])
				{
					distance[next] = value;
					pending.push({value, next});
				}
			}
		}
		return -1;
	}
	bool Campaign::AssignOfficer(int general, int target, bool ai)
	{
		if (result != 0 || general < 0 || general >= static_cast<int>(generals.size()) || target < 0 ||
		    target >= static_cast<int>(cities.size()))
			return false;
		const auto& officer = generals[general];
		const int from = officer.home;
		if (from < 0 || from >= static_cast<int>(cities.size()) || officer.faction != cities[from].owner ||
		    officer.faction != cities[target].owner || officer.readyDay > day || Busy(general) ||
		    (!ai && (officer.faction != player || commands <= 0)) || cities[from].gold < 100)
			return false;
		const int days = AssignmentDays(from, target);
		if (days < 0)
			return false;
		cities[from].gold -= 100;
		if (!ai)
			--commands;
		assignments.push_back({general, from, target, officer.faction, days});
		const auto count = std::to_string(days);
		Note(officer.name + U"を" + cities[target].name + U"へ異動（" +
		     std::u32string(count.begin(), count.end()) + U"日）。");
		return true;
	}
	void Campaign::CancelInvalidAssignments()
	{
		assignments.erase(
		    std::remove_if(assignments.begin(), assignments.end(),
		                   [&](const Assignment& assignment) {
			                   if (cities[assignment.from].owner == assignment.faction &&
			                       cities[assignment.to].owner == assignment.faction)
				                   return false;
			                   auto& officer = generals[assignment.general];
			                   const int refuge =
			                       NearestCity(cities[assignment.from].tile, assignment.faction, false);
			                   if (refuge >= 0)
				                   officer.home = refuge;
			                   officer.readyDay = day + 10;
			                   Note(officer.name + U"の異動は都市の陥落で中止。再任用まで10日。");
			                   return true;
		                   }),
		    assignments.end());
	}
	void Campaign::AdvanceAssignments()
	{
		CancelInvalidAssignments();
		assignments.erase(std::remove_if(assignments.begin(), assignments.end(),
		                                 [&](Assignment& assignment) {
			                                 if (--assignment.left > 0)
				                                 return false;
			                                 generals[assignment.general].home = assignment.to;
			                                 Note(generals[assignment.general].name + U"が" +
			                                      cities[assignment.to].name +
			                                      U"に着任。内政・軍務に任用できます。");
			                                 return true;
		                                 }),
		                  assignments.end());
	}
	// AIも金・移動時間・担当者の拘束を負担し、月ごとに無人都市へ1名を送る。
	void Campaign::RedistributeOfficers(int faction)
	{
		for (int from = 0; from < static_cast<int>(cities.size()); ++from)
		{
			if (cities[from].owner != faction || AIThreat(from) > 0)
				continue;
			const auto available = Available(from);
			if (available.size() < 2)
				continue;
			int candidate = -1;
			for (int g : available)
				if (g != Leader(faction) &&
				    (candidate < 0 || generals[g].politics - generals[g].leadership >
				                          generals[candidate].politics - generals[candidate].leadership))
					candidate = g;
			if (candidate < 0)
				continue;
			std::vector<int> targets;
			for (int to = 0; to < static_cast<int>(cities.size()); ++to)
			{
				if (cities[to].owner != faction || to == from)
					continue;
				const bool staffed =
				    std::any_of(generals.begin(), generals.end(),
				                [&](const General& g) { return g.home == to && g.faction == faction; }) ||
				    std::any_of(assignments.begin(), assignments.end(),
				                [&](const Assignment& move) { return move.to == to; });
				if (!staffed)
					targets.push_back(to);
			}
			std::sort(targets.begin(), targets.end(), [&](int a, int b) {
				const auto score = [&](int city) {
					const int enemy = NearestCity(cities[city].tile, faction, true);
					return AIThreat(city) / 50 +
					       (enemy >= 0 ? std::max(0, 24 - Distance(cities[city].tile, cities[enemy].tile)) * 4
					                   : 0) -
					       Distance(cities[from].tile, cities[city].tile);
				};
				return score(a) > score(b);
			});
			for (int target : targets)
				if (AssignOfficer(candidate, target, true))
					return;
		}
	}
} // namespace frontline
