#include "Campaign.hpp"
namespace frontline
{
	void Campaign::BeginTurn()
	{
		if (result != 0)
			return;
		for (int f = 0; f < 3; ++f)
		{
			if (f == player || aiPlannedDay[f] == day)
				continue;
			if (aiPlannedDay[f] < 0 || aiPlannedDay[f] / 10 != day / 10)
				aiCommands[f] = 3;
			aiPlannedDay[f] = day;
			// Re-evaluate after each successful action: officers and stores may now be reserved.
			while (aiCommands[f] > 0)
			{
				const auto plan = AIPlan(f);
				bool acted = false;
				for (const auto& order : plan)
					if (ExecuteAI(f, order))
					{
						--aiCommands[f];
						acted = true;
						break;
					}
				if (!acted)
					break;
			}
		}
	}
	bool Campaign::ExecuteAI(int faction, const AIOrder& o)
	{
		bool success = false;
		switch (o.kind)
		{
		case AIKind::Govern:
			success = AppointGovernor(o.city, o.general, true);
			break;
		case AIKind::Develop:
			success = Develop(o.city, o.general, o.duty, true);
			break;
		case AIKind::Recruit:
			success = Recruit(o.city, true);
			break;
		case AIKind::Deploy: {
			auto path = Route(cities[o.city].tile, o.target, o.arm, faction);
			if (path.empty() && cities[o.city].tile != o.target)
				break;
			const int a = Deploy(o.city, o.general, o.amount, o.arm, true);
			if (a >= 0)
			{
				armies[a].target = o.target;
				armies[a].path = std::move(path);
				success = true;
			}
			break;
		}
		case AIKind::Transport:
			success = DispatchTransport(o.city, o.general, o.target, o.amount, true) >= 0;
			break;
		case AIKind::Mission:
			success = SendMission(o.city, o.general, o.target, o.mission, -1, true);
			break;
		case AIKind::Transfer: {
			const auto before = assignments.size();
			RedistributeOfficers(faction);
			success = assignments.size() > before;
			break;
		}
		case AIKind::Stance:
			success = SetStance(o.general, static_cast<battle::Stance>(o.amount), true);
			break;
		}
		if (success)
			Note(FactionName(faction) + U"軍の判断：" + (o.city >= 0 ? cities[o.city].name + U"：" : U"") +
			     o.reason);
		return success;
	}
} // namespace frontline
