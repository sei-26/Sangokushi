#include "Campaign.hpp"

namespace frontline
{
	void Campaign::BeginTurn()
	{
		if (result != 0)
			return;
		for (int f = 0; f < 3; ++f)
		{
			if (f == player)
				continue;
			for (int c = 0; c < static_cast<int>(cities.size()); ++c)
			{
				if (cities[c].owner == f && cities[c].worker < 0)
				{
					const auto staff = Available(c);
					const auto duty = static_cast<Duty>((day / 30 + c) % 4);
					if (!staff.empty())
					{
						const int g = *std::max_element(staff.begin(), staff.end(), [&](int a, int b) {
							return WorkGain(a, duty) * 10 - generals[a].leadership / 3 <
							       WorkGain(b, duty) * 10 - generals[b].leadership / 3;
						});
						Develop(c, g, duty, true);
					}
				}
				if (cities[c].owner == f && day >= 60 && day % 60 == 0)
				{
					const auto staff = Available(c);
					const int enemy = NearestCity(cities[c].tile, f, true);
					int agent = -1;
					for (int g : staff)
						if ((generals[g].trait == Trait::Strategist || generals[g].trait == Trait::Raider) &&
						    (agent < 0 || generals[g].intelligence > generals[agent].intelligence))
							agent = g;
					if (agent >= 0 && enemy >= 0)
						SendMission(c, agent, enemy, MissionKind::Sabotage, -1, true);
				}
				if (day < 60 || day % 30 != 0)
					continue;
				if (cities[c].owner == f && cities[c].troops < 7000)
					Recruit(c, true);
				if (cities[c].owner != f || ArmyCount(f) >= 3 || cities[c].troops < 7000)
					continue;
				const auto available = Available(c);
				if (available.empty())
					continue;
				const int enemy = NearestCity(cities[c].tile, f, true);
				if (enemy < 0)
					continue;
				const int a =
				    Deploy(c, available.front(), 4000, (day / 10 + c) % 2 ? Arm::Spear : Arm::Siege, true);
				if (a >= 0)
					Order(a, cities[enemy].tile);
			}
		}
	}
} // namespace frontline
