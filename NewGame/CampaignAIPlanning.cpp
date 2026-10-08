#include "Campaign.hpp"
namespace frontline
{
	int Campaign::AIThreat(int city) const
	{
		int threat = 0;
		for (const auto& a : armies)
			if (a.troops > 0 && a.arm != Arm::Transport && Hostile(a.faction, cities[city].owner) &&
			    Distance(a.tile, cities[city].tile) <= 6)
				threat += a.troops * (7 - Distance(a.tile, cities[city].tile)) / 7;
		return threat;
	}
	int Campaign::AIEnemyCity(int from, int faction, Arm arm, int troops) const
	{
		std::vector<std::pair<int, int>> candidates;
		for (int c = 0; c < static_cast<int>(cities.size()); ++c)
			if (Hostile(faction, cities[c].owner) && Distance(from, cities[c].tile) <= 32 &&
			    cities[c].troops <= troops * 3)
			{
				int committed = 0;
				for (const auto& a : armies)
					if (a.troops > 0 && a.faction == faction && !a.retreat && a.target == cities[c].tile)
						committed += a.troops;
				candidates.push_back({2000 - Distance(from, cities[c].tile) * 35 - cities[c].troops / 20 +
				                          std::min(450, committed / 20),
				                      c});
			}
		std::stable_sort(candidates.begin(), candidates.end(),
		                 [](auto a, auto b) { return a.first > b.first; });
		for (size_t n = 0; n < std::min<size_t>(3, candidates.size()); ++n)
			if (!Route(from, cities[candidates[n].second].tile, arm, faction).empty())
				return candidates[n].second;
		return -1;
	}
	std::vector<Campaign::AIOrder> Campaign::AIPlan(int faction) const
	{
		std::vector<AIOrder> plan;
		if (day >= 30 && day % 30 == 0 &&
		    std::none_of(assignments.begin(), assignments.end(),
		                 [&](const Assignment& a) { return a.faction == faction; }))
		{
			AIOrder o{AIKind::Transfer, 80};
			o.reason = U"後方の余剰人材を無人都市へ回す";
			plan.push_back(o);
		}
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			const auto& a = armies[i];
			if (a.troops <= 0 || a.faction != faction || a.arm == Arm::Transport)
				continue;
			const int city = CityAt(a.target);
			const auto stance = a.morale < 45 || a.troops < 2200 || !a.supplied ? battle::Stance::Guard
			                    : a.arm == Arm::Siege && city >= 0 && Hostile(faction, cities[city].owner) &&
			                            Distance(a.tile, a.target) <= 2
			                        ? battle::Stance::Assault
			                        : battle::Stance::Balanced;
			if (a.stance != stance)
			{
				AIOrder o{AIKind::Stance, stance == battle::Stance::Guard ? 110 : 65};
				o.general = i;
				o.amount = static_cast<int>(stance);
				o.reason = U"損耗・補給と攻城の状況に合わせて構えを調整";
				plan.push_back(o);
			}
		}
		for (int c = 0; c < static_cast<int>(cities.size()); ++c)
		{
			const auto& city = cities[c];
			if (city.owner != faction)
				continue;
			const auto staff = Available(c);
			const int threat = AIThreat(c);
			int commander = -1, administrator = -1;
			for (int g : staff)
			{
				if (commander < 0 || generals[g].leadership > generals[commander].leadership)
					commander = g;
				if (administrator < 0 ||
				    generals[g].politics - generals[g].leadership / 2 >
				        generals[administrator].politics - generals[administrator].leadership / 2)
					administrator = g;
			}
			if (administrator == commander && commander >= 0 && generals[commander].leadership > 80)
			{
				administrator = -1;
				for (int g : staff)
					if (g != commander &&
					    (administrator < 0 || generals[g].politics > generals[administrator].politics))
						administrator = g;
			}
			if (threat > 1500 && commander >= 0 && city.troops >= 4000 && city.food >= 2000 &&
			    ArmyCount(faction) < MaxArmies)
			{
				int enemy = -1;
				for (int a = 0; a < static_cast<int>(armies.size()); ++a)
					if (armies[a].troops > 0 && Hostile(faction, armies[a].faction) &&
					    Distance(armies[a].tile, city.tile) <= 6 &&
					    (enemy < 0 ||
					     Distance(armies[a].tile, city.tile) < Distance(armies[enemy].tile, city.tile)))
						enemy = a;
				int defenders = 0;
				for (const auto& a : armies)
					if (a.troops > 0 && a.faction == faction && a.arm != Arm::Transport &&
					    Distance(a.tile, city.tile) <= 4)
						defenders += a.troops;
				if (enemy >= 0 && defenders < threat)
				{
					AIOrder o{AIKind::Deploy, 150};
					o.city = c;
					o.general = commander;
					o.amount = std::min(4000, city.troops - 2000);
					o.target = armies[enemy].tile;
					o.arm = generals[commander].tactic == Tactic::Volley ? Arm::Bow : Arm::Spear;
					o.reason = U"接近する敵に守備隊を出し、城の守備兵も残す";
					plan.push_back(o);
				}
			}
			if (city.food < 6000 && ArmyCount(faction) < MaxArmies &&
			    std::none_of(
			        armies.begin(), armies.end(),
			        [&](const Army& a) {
				        return a.troops > 0 && a.faction == faction && a.arm == Arm::Transport;
			        }))
				for (int donor = 0; donor < static_cast<int>(cities.size()); ++donor)
				{
					if (donor == c || cities[donor].owner != faction || cities[donor].food < 18000 ||
					    cities[donor].troops < 2000 || AIThreat(donor) > 0)
						continue;
					const auto workers = Available(donor);
					if (workers.empty())
						continue;
					const int g = *std::max_element(workers.begin(), workers.end(), [&](int a, int b) {
						return generals[a].politics - generals[a].leadership <
						       generals[b].politics - generals[b].leadership;
					});
					if (workers.size() == 1 && generals[g].leadership > 80)
						continue;
					AIOrder o{AIKind::Transport, 130};
					o.city = donor;
					o.general = g;
					o.target = c;
					o.amount = 5000;
					o.reason = U"前線の糧不足を後方の実在する蓄えで補う";
					plan.push_back(o);
				}
			if (city.troops < (threat > 0 ? 8000 : 6000) && city.food >= 5000 && city.gold >= 300 &&
			    city.order >= 50)
			{
				AIOrder o{AIKind::Recruit, threat > 0 ? 120 : 55};
				o.city = c;
				o.reason = U"兵糧と治安を確認して不足する守備兵を補う";
				plan.push_back(o);
			}
			if (day >= 60 && commander >= 0 && ArmyCount(faction) < MaxArmies - 1 && threat == 0 &&
			    city.troops >= 7000 && city.food >= 7000 && city.gold >= 400)
			{
				const int target = AIEnemyCity(city.tile, faction, Arm::Spear, 4000);
				if (target >= 0)
				{
					int siege = 0;
					for (const auto& a : armies)
						if (a.troops > 0 && a.faction == faction && a.target == cities[target].tile &&
						    a.arm == Arm::Siege)
							++siege;
					AIOrder o{AIKind::Deploy, 90};
					o.city = c;
					o.general = commander;
					o.amount = std::min(4000, city.troops - 3000);
					o.target = cities[target].tile;
					o.arm = generals[commander].tactic == Tactic::Volley ? Arm::Bow
					        : siege == 0                                 ? Arm::Siege
					        : generals[commander].leadership >= 90       ? Arm::Cavalry
					                                                     : Arm::Spear;
					o.reason = U"到達可能な敵城へ攻城隊と支援部隊を集める";
					plan.push_back(o);
				}
			}
			if (city.worker < 0 && administrator >= 0 && city.gold >= 500 &&
			    (staff.size() > 1 || generals[administrator].leadership <= 80))
			{
				const Duty duty = city.order < 55       ? Duty::Order
				                  : city.food < 12000   ? Duty::Farming
				                  : city.gold < 1500    ? Duty::Commerce
				                  : city.logistics < 55 ? Duty::Logistics
				                  : city.farming < 75   ? Duty::Farming
				                  : city.commerce < 75  ? Duty::Commerce
				                                        : Duty::Order;
				const int level = duty == Duty::Order      ? city.order
				                  : duty == Duty::Farming  ? city.farming
				                  : duty == Duty::Commerce ? city.commerce
				                                           : city.logistics;
				if (level < 100 && !(threat > 0 && administrator == commander))
				{
					AIOrder o{AIKind::Develop, city.order < 55 ? 125 : 70};
					o.city = c;
					o.general = administrator;
					o.duty = duty;
					o.reason = U"指揮官を確保し、都市の不足に合った内政を任せる";
					plan.push_back(o);
				}
			}
			if (day >= 60 && !staff.empty())
			{
				const int target = NearestCity(city.tile, faction, true);
				if (target >= 0 && std::none_of(missions.begin(), missions.end(), [&](const Mission& m) {
					    return m.faction == faction && m.target == target;
				    }))
				{
					const MissionKind kind =
					    threat > city.troops ? MissionKind::Diplomacy : MissionKind::Sabotage;
					int agent = -1;
					for (int g : staff)
						if ((g != commander || staff.size() > 1) && MissionChance(g, target, kind) >= 65 &&
						    (agent < 0 ||
						     MissionChance(g, target, kind) > MissionChance(agent, target, kind)))
							agent = g;
					if (agent >= 0)
					{
						AIOrder o{AIKind::Mission, kind == MissionKind::Diplomacy ? 115 : 45};
						o.city = c;
						o.general = agent;
						o.target = target;
						o.mission = kind;
						o.reason = kind == MissionKind::Diplomacy ? U"劣勢の戦線で時間を稼ぐ停戦を試みる"
						                                          : U"適任者に敵の兵糧を攪乱させる";
						plan.push_back(o);
					}
				}
			}
		}
		std::stable_sort(plan.begin(), plan.end(),
		                 [](const AIOrder& a, const AIOrder& b) { return a.priority > b.priority; });
		return plan;
	}
} // namespace frontline
