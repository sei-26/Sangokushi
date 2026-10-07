#include "Campaign.hpp"

namespace frontline
{
	std::vector<bool> Campaign::ResolveDailyCombat(const SupplyGrid& supply)
	{
		// Calculate all casualties from the same snapshot, then apply together.
		std::vector<int> losses(armies.size()), cityLoss(cities.size());
		std::vector<bool> fighting(armies.size());
		std::vector<int> moraleGain(armies.size());
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			auto& a = armies[i];
			if (a.troops <= 0)
				continue;
			if (a.tacticLeft > 0)
				--a.tacticLeft;
			if (a.faction != player && !a.tacticQueued)
			{
				bool nearby = false;
				for (const auto& b : armies)
					if (b.troops > 0 && Hostile(a.faction, b.faction) && Distance(a.tile, b.tile) <= 2)
						nearby = true;
				const int c = CityAt(a.target);
				if (c >= 0 && Hostile(a.faction, cities[c].owner) && Distance(a.tile, a.target) <= 1)
					nearby = true;
				if (nearby)
					ActivateTactic(i, true);
			}
			if (!a.tacticQueued)
				continue;
			a.tacticQueued = false;
			const auto& g = generals[a.general];
			bool fired = true;
			if (g.tactic == Tactic::Rally)
			{
				for (int j = 0; j < static_cast<int>(armies.size()); ++j)
					if (armies[j].troops > 0 && armies[j].faction == a.faction &&
					    Distance(a.tile, armies[j].tile) <= 2)
						moraleGain[j] += 20;
			}
			else if (g.tactic == Tactic::Fire)
			{
				int victim = -1, nearest = 3;
				for (int j = 0; j < static_cast<int>(armies.size()); ++j)
					if (armies[j].troops > 0 && Hostile(a.faction, armies[j].faction) &&
					    Distance(a.tile, armies[j].tile) < nearest)
					{
						nearest = Distance(a.tile, armies[j].tile);
						victim = j;
					}
				const int c = CityAt(a.target);
				const int damage = 120 + g.intelligence * 2;
				if (victim >= 0)
					losses[victim] += damage;
				else if (c >= 0 && Hostile(a.faction, cities[c].owner) && Distance(a.tile, a.target) <= 1)
					cityLoss[c] += damage;
				else
					fired = false;
			}
			else if (g.tactic == Tactic::Supply)
			{
				const int c = supply[a.faction][a.tile];
				if (c >= 0 && cities[c].food >= 300)
				{
					cities[c].food -= 300;
					a.food += 300;
					moraleGain[i] += 30;
				}
				else
					fired = false;
			}
			else
				a.tacticLeft = 5;
			if (fired)
			{
				a.morale = std::max(0, a.morale - 15);
				a.tacticReadyDay = day + 30;
				Note(g.name + U"が戦法「" + TacticName(g.tactic) + U"」を発動！");
			}
			else
				Note(g.name + U"の戦法は対象・補給の変化で発動できませんでした。");
		}
		std::vector<officer::Link> formations;
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
			formations.push_back(Formation(i));
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
			armies[i].morale = std::min(100, armies[i].morale + moraleGain[i] + formations[i].morale / 2);
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			const auto& a = armies[i];
			if (a.troops <= 0)
				continue;
			const auto& officer = generals[a.general];
			const int range =
			    a.arm == Arm::Bow ? (a.tacticLeft > 0 && officer.tactic == Tactic::Volley ? 3 : 2) : 1;
			int target = -1, distance = 100000;
			for (int j = 0; j < static_cast<int>(armies.size()); ++j)
				if (armies[j].troops > 0 && Hostile(armies[j].faction, a.faction) &&
				    Distance(a.tile, armies[j].tile) <= range && Distance(a.tile, armies[j].tile) < distance)
				{
					target = j;
					distance = Distance(a.tile, armies[j].tile);
				}
			const double morale = 0.3 + a.morale / 140.0;
			const double coordination = 1 + formations[i].attack / 100.0;
			const double tactic = a.tacticLeft > 0 ? (officer.tactic == Tactic::Charge   ? 1.35
			                                          : officer.tactic == Tactic::Volley ? 1.2
			                                                                             : 1.0)
			                                       : 1.0;
			const double power =
			    (a.troops / 22.0 + officer.leadership * 1.3) * morale * 0.55 * coordination * tactic;
			if (target >= 0)
			{
				const auto& b = armies[target];
				const int flank = Fronts(b.tile, a.faction);
				double defense = tiles[b.tile].terrain == Terrain::Mountain ? 1.5
				                 : tiles[b.tile].terrain == Terrain::Forest ? 1.25
				                                                            : 1.0;
				if (tiles[b.tile].terrain == Terrain::River)
					defense = 0.8;
				double multiplier = a.arm == Arm::Siege ? 0.5 : 1.0;
				if (officer.trait == Trait::Valiant)
					multiplier *= 1.12;
				if (officer.trait == Trait::Raider && tiles[a.tile].terrain == Terrain::Forest)
					multiplier *= 1.18;
				if (a.arm == Arm::Cavalry && tiles[a.tile].terrain == Terrain::Plain)
					multiplier *= 1.2;
				losses[target] += std::max(
				    1, static_cast<int>(power * multiplier *
				                        (1.0 + std::min(3, std::max(0, flank - 1)) * 0.15) / defense));
				fighting[i] = true;
				if (distance == 1)
					fighting[target] = true;
			}
			else if (!a.retreat)
			{
				const int c = CityAt(a.target);
				if (c >= 0 && Hostile(cities[c].owner, a.faction) && Distance(a.tile, cities[c].tile) <= 1)
				{
					const int siege = Fronts(cities[c].tile, a.faction);
					cityLoss[c] +=
					    std::max(1, static_cast<int>(power * (a.arm == Arm::Siege ? 2.0 : 0.6) *
					                                 (1 + std::min(3, std::max(0, siege - 1)) * 0.2)));
					losses[i] += std::min(cities[c].troops / 100 + 15, 70);
					fighting[i] = true;
				}
			}
		}
		if (day % 10 == 0)
			for (int i = 0; i < static_cast<int>(armies.size()); ++i)
				for (int j = i + 1; j < static_cast<int>(armies.size()); ++j)
					if (armies[i].troops > 0 && armies[j].troops > 0 && fighting[i] && fighting[j] &&
					    armies[i].faction == armies[j].faction &&
					    Distance(armies[i].tile, armies[j].tile) <= 2)
						ChangeBond(armies[i].general, armies[j].general, 2);
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			auto& a = armies[i];
			if (a.troops <= 0)
				continue;
			const auto& officer = generals[a.general];
			if (officer.trait == Trait::Guardian)
				losses[i] = static_cast<int>(losses[i] * 0.88);
			if (a.tacticLeft > 0 && officer.tactic == Tactic::Fortify)
				losses[i] = static_cast<int>(losses[i] * 0.7);
			losses[i] = losses[i] * (100 - formations[i].defense) / 100;
			a.troops = std::max(0, a.troops - losses[i]);
			if (losses[i] > 0)
				a.morale = std::max(0, a.morale - 2);
			if (a.troops == 0)
			{
				a.tacticQueued = false;
				generals[a.general].readyDay = day + 20;
				Note(generals[a.general].name + U"隊が敗走。");
			}
		}
		for (int c = 0; c < static_cast<int>(cities.size()); ++c)
		{
			cities[c].troops = std::max(0, cities[c].troops - cityLoss[c]);
			if (cityLoss[c] == 0 || cities[c].troops > 0)
				continue;
			int winner = -1;
			for (int i = 0; i < static_cast<int>(armies.size()); ++i)
				if (armies[i].troops > 0 && !armies[i].retreat && armies[i].target == cities[c].tile &&
				    Hostile(armies[i].faction, cities[c].owner) &&
				    Distance(armies[i].tile, cities[c].tile) <= 1 &&
				    (winner < 0 || armies[i].troops > armies[winner].troops))
					winner = i;
			if (winner >= 0)
			{
				const int previousOwner = cities[c].owner;
				auto& a = armies[winner];
				cities[c].owner = a.faction;
				tiles[cities[c].tile].owner = a.faction;
				cities[c].worker = cities[c].helper = -1;
				cities[c].workLeft = 0;
				cities[c].order = std::max(20, cities[c].order - 25);
				const int garrison = std::min(1000, a.troops / 3);
				cities[c].troops = garrison;
				a.troops -= garrison;
				Note(FactionName(a.faction) + U"軍が" + cities[c].name + U"を攻略！");
				++revision;
				missions.erase(std::remove_if(missions.begin(), missions.end(),
				                              [&](const Mission& m) {
					                              if (m.city != c && m.target != c)
						                              return false;
					                              Note(generals[m.general].name +
					                                   U"の任務は都市の陥落により中止。");
					                              return true;
				                              }),
				               missions.end());
				const int refuge = NearestCity(cities[c].tile, previousOwner, false);
				if (refuge >= 0)
					for (int g = 0; g < static_cast<int>(generals.size()); ++g)
						if (generals[g].home == c && generals[g].faction == previousOwner && !Busy(g))
						{
							generals[g].home = refuge;
							generals[g].readyDay = day + 10;
							Note(generals[g].name + U"が" + cities[refuge].name + U"へ避難。");
						}
			}
		}

		return fighting;
	}
} // namespace frontline
