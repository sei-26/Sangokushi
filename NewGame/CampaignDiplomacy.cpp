#include "Campaign.hpp"

namespace frontline
{
	bool Campaign::Hostile(int a, int b) const
	{
		return a != b && day >= truceUntil[a][b];
	}

	int Campaign::MissionChance(int general, int target, MissionKind kind, int helper) const
	{
		const auto& g = generals[general];
		int chance;
		if (kind == MissionKind::Diplomacy)
		{
			chance = 20 + g.charm / 2 + regard[g.faction][cities[target].owner] / 5 +
			         Affinity(general, Leader(cities[target].owner)) / 10;
			if (g.trait == Trait::Negotiator)
				chance += 18;
			if (g.trait == Trait::Merchant)
				chance += 8;
		}
		else
		{
			int defense = 35;
			for (int i = 0; i < static_cast<int>(generals.size()); ++i)
				if (generals[i].home == target && generals[i].faction == cities[target].owner)
				{
					const bool away =
					    std::any_of(armies.begin(), armies.end(),
					                [&](const Army& a) { return a.troops > 0 && a.general == i; }) ||
					    std::any_of(missions.begin(), missions.end(),
					                [&](const Mission& m) { return m.general == i || m.helper == i; });
					if (!away)
						defense = std::max(defense, generals[i].intelligence +
						                                (generals[i].trait == Trait::Guardian     ? 15
						                                 : generals[i].trait == Trait::Strategist ? 10
						                                                                          : 0));
				}
			chance = 35 + g.intelligence / 2 - defense / 3;
			if (g.trait == Trait::Strategist)
				chance += 15;
			if (g.trait == Trait::Raider)
				chance += 10;
		}
		if (helper >= 0)
			chance +=
			    (kind == MissionKind::Diplomacy ? generals[helper].charm : generals[helper].intelligence) /
			        20 +
			    Affinity(general, helper) / 10;
		return std::clamp(chance, 10, 90);
	}

	bool Campaign::SendMission(int city, int general, int target, MissionKind kind, int helper, bool ai)
	{
		if (city < 0 || city >= static_cast<int>(cities.size()) || target < 0 ||
		    target >= static_cast<int>(cities.size()) || static_cast<int>(kind) < 0 ||
		    static_cast<int>(kind) > 1)
			return false;
		const auto staff = Available(city);
		auto& c = cities[city];
		if ((!ai && (c.owner != player || commands <= 0)) || c.gold < 300 ||
		    c.owner == cities[target].owner ||
		    std::find(staff.begin(), staff.end(), general) == staff.end() ||
		    (helper != -1 &&
		     (helper == general || std::find(staff.begin(), staff.end(), helper) == staff.end())) ||
		    (kind == MissionKind::Sabotage && !Hostile(c.owner, cities[target].owner)))
			return false;
		c.gold -= 300;
		if (!ai)
			--commands;
		missions.push_back({general, helper, city, target, c.owner, cities[target].owner,
		                    kind == MissionKind::Diplomacy ? 20 : 30, kind});
		Note(generals[general].name + U"が" + cities[target].name +
		     (kind == MissionKind::Diplomacy ? U"へ停戦交渉に出発。" : U"へ兵糧攪乱の工作に出発。"));
		return true;
	}

	unsigned Campaign::Roll()
	{
		randomState ^= randomState << 13;
		randomState ^= randomState >> 17;
		randomState ^= randomState << 5;
		return randomState % 100;
	}

	void Campaign::ResolveMissions()
	{
		for (auto it = missions.begin(); it != missions.end();)
		{
			const Mission m = *it;
			if (cities[m.city].owner != m.faction || cities[m.target].owner != m.targetFaction ||
			    (m.kind == MissionKind::Sabotage && !Hostile(m.faction, m.targetFaction)))
			{
				Note(generals[m.general].name + U"の任務を中止。勢力・停戦状況が変化しました。");
				it = missions.erase(it);
				continue;
			}
			if (--it->left > 0)
			{
				++it;
				continue;
			}
			const bool success =
			    static_cast<int>(Roll()) < MissionChance(m.general, m.target, m.kind, m.helper);
			if (success && m.kind == MissionKind::Diplomacy)
			{
				truceUntil[m.faction][m.targetFaction] = truceUntil[m.targetFaction][m.faction] = day + 60;
				regard[m.faction][m.targetFaction] = regard[m.targetFaction][m.faction] =
				    std::min(100, regard[m.faction][m.targetFaction] + 15);
				ChangeBond(m.general, Leader(m.targetFaction), 10);
				for (auto& a : armies)
				{
					const int target = CityAt(a.target);
					if (a.troops > 0 && target >= 0 && a.faction != cities[target].owner &&
					    !Hostile(a.faction, cities[target].owner))
					{
						a.path.clear();
						a.target = a.tile;
					}
				}
				Note(generals[m.general].name + U"の交渉が成立。" + FactionName(m.faction) + U"・" +
				     FactionName(m.targetFaction) + U"が60日間の停戦。");
			}
			else if (success)
			{
				auto& target = cities[m.target];
				target.food = std::max(0, target.food - std::min(3000, target.food / 5));
				target.order = std::max(0, target.order - 15);
				regard[m.faction][m.targetFaction] = regard[m.targetFaction][m.faction] =
				    std::max(0, regard[m.faction][m.targetFaction] - 15);
				ChangeBond(m.general, Leader(m.targetFaction), -5);
				Note(generals[m.general].name + U"の工作が成功。" + target.name + U"の兵糧と治安が低下。");
			}
			else
			{
				generals[m.general].readyDay = day + 10;
				if (m.helper >= 0)
					generals[m.helper].readyDay = day + 10;
				Note(generals[m.general].name + U"の任務が失敗。再任用まで10日必要です。");
			}
			ChangeBond(m.general, m.helper, success ? 6 : 2);
			it = missions.erase(it);
		}
	}
} // namespace frontline
