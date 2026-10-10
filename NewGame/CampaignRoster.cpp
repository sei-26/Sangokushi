#include "Campaign.hpp"

namespace frontline
{
	void Campaign::AddExpandedRoster()
	{
		// Fictional sandbox roster: generations coexist; IDs 0..23 remain stable.
		struct Entry
		{
			const char32_t* name;
			int faction, leadership, politics, intelligence, charm;
			Duty duty;
			Trait trait;
			Tactic tactic;
		};
		const Entry entries[] = {
		    {U"馬超", 0, 93, 45, 48, 82, Duty::Order, Trait::CavalryExpert, Tactic::MountedCharge},
		    {U"馬岱", 0, 82, 54, 64, 69, Duty::Logistics, Trait::CavalryExpert, Tactic::MountedCharge},
		    {U"魏延", 0, 89, 49, 69, 52, Duty::Order, Trait::TerrainExpert, Tactic::Ambush},
		    {U"龐統", 0, 78, 88, 97, 75, Duty::Logistics, Trait::TerrainExpert, Tactic::Ambush},
		    {U"姜維", 0, 91, 71, 90, 78, Duty::Logistics, Trait::Resolute, Tactic::Charge},
		    {U"法正", 0, 77, 84, 94, 65, Duty::Commerce, Trait::Strategist, Tactic::Fire},
		    {U"徐庶", 0, 82, 79, 93, 85, Duty::Order, Trait::Strategist, Tactic::Fire},
		    {U"張苞", 0, 82, 40, 48, 63, Duty::Order, Trait::Valiant, Tactic::Charge},
		    {U"関興", 0, 84, 56, 63, 74, Duty::Order, Trait::Resolute, Tactic::Fortify},
		    {U"劉封", 0, 77, 46, 50, 61, Duty::Order, Trait::Valiant, Tactic::Charge},
		    {U"李厳", 0, 80, 83, 76, 62, Duty::Logistics, Trait::SiegeExpert, Tactic::SiegeStrike},
		    {U"王平", 0, 84, 64, 70, 75, Duty::Order, Trait::Guardian, Tactic::Fortify},
		    {U"蔣琬", 0, 62, 94, 86, 89, Duty::Commerce, Trait::Inspiring, Tactic::Rally},
		    {U"費禕", 0, 55, 92, 88, 90, Duty::Order, Trait::Negotiator, Tactic::Rally},
		    {U"董允", 0, 42, 90, 81, 83, Duty::Commerce, Trait::Administrator, Tactic::Supply},
		    {U"鄧芝", 0, 68, 86, 80, 91, Duty::Logistics, Trait::Negotiator, Tactic::Supply},
		    {U"夏侯淵", 1, 90, 53, 62, 67, Duty::Order, Trait::ArcherExpert, Tactic::Volley},
		    {U"張郃", 1, 92, 66, 79, 74, Duty::Order, Trait::ArcherExpert, Tactic::Volley},
		    {U"楽進", 1, 85, 48, 54, 65, Duty::Order, Trait::SiegeExpert, Tactic::SiegeStrike},
		    {U"于禁", 1, 87, 62, 72, 60, Duty::Order, Trait::Guardian, Tactic::Fortify},
		    {U"許褚", 1, 74, 23, 29, 60, Duty::Order, Trait::Resolute, Tactic::Charge},
		    {U"典韋", 1, 75, 21, 25, 58, Duty::Order, Trait::Guardian, Tactic::Fortify},
		    {U"曹洪", 1, 78, 51, 49, 56, Duty::Logistics, Trait::Quartermaster, Tactic::Supply},
		    {U"曹真", 1, 86, 70, 75, 72, Duty::Order, Trait::CavalryExpert, Tactic::MountedCharge},
		    {U"曹休", 1, 82, 61, 68, 69, Duty::Order, Trait::CavalryExpert, Tactic::MountedCharge},
		    {U"鄧艾", 1, 93, 83, 91, 64, Duty::Farming, Trait::TerrainExpert, Tactic::Ambush},
		    {U"鍾会", 1, 84, 80, 93, 62, Duty::Commerce, Trait::SiegeExpert, Tactic::SiegeStrike},
		    {U"司馬懿", 1, 96, 91, 98, 77, Duty::Logistics, Trait::Strategist, Tactic::Fire},
		    {U"郭嘉", 1, 70, 80, 97, 82, Duty::Logistics, Trait::Strategist, Tactic::Fire},
		    {U"荀攸", 1, 73, 89, 95, 78, Duty::Commerce, Trait::TerrainExpert, Tactic::Ambush},
		    {U"程昱", 1, 79, 85, 90, 61, Duty::Farming, Trait::Farmer, Tactic::Supply},
		    {U"陳群", 1, 46, 96, 84, 87, Duty::Commerce, Trait::Inspiring, Tactic::Rally},
		    {U"黄蓋", 2, 83, 60, 69, 78, Duty::Order, Trait::SiegeExpert, Tactic::SiegeStrike},
		    {U"程普", 2, 85, 72, 76, 82, Duty::Order, Trait::Guardian, Tactic::Fortify},
		    {U"韓当", 2, 81, 52, 61, 69, Duty::Order, Trait::ArcherExpert, Tactic::Volley},
		    {U"周泰", 2, 83, 38, 49, 70, Duty::Order, Trait::Resolute, Tactic::Fortify},
		    {U"蔣欽", 2, 79, 49, 60, 68, Duty::Order, Trait::ArcherExpert, Tactic::Volley},
		    {U"凌統", 2, 84, 51, 65, 73, Duty::Order, Trait::Valiant, Tactic::Charge},
		    {U"徐盛", 2, 85, 68, 77, 75, Duty::Order, Trait::Guardian, Tactic::Fortify},
		    {U"丁奉", 2, 83, 53, 69, 68, Duty::Order, Trait::TerrainExpert, Tactic::Ambush},
		    {U"朱桓", 2, 85, 58, 72, 65, Duty::Order, Trait::SiegeExpert, Tactic::SiegeStrike},
		    {U"朱然", 2, 84, 67, 75, 72, Duty::Order, Trait::ArcherExpert, Tactic::Volley},
		    {U"孫策", 2, 95, 67, 76, 92, Duty::Order, Trait::Valiant, Tactic::Charge},
		    {U"孫尚香", 2, 77, 61, 69, 83, Duty::Order, Trait::ArcherExpert, Tactic::Volley},
		    {U"魯粛", 2, 75, 93, 91, 95, Duty::Logistics, Trait::Inspiring, Tactic::Rally},
		    {U"張昭", 2, 48, 96, 87, 81, Duty::Commerce, Trait::Administrator, Tactic::Supply},
		    {U"諸葛瑾", 2, 60, 89, 85, 93, Duty::Order, Trait::Negotiator, Tactic::Rally},
		    {U"陸抗", 2, 93, 85, 91, 84, Duty::Logistics, Trait::TerrainExpert, Tactic::Ambush},
		};
		std::array<std::vector<int>, 3> homes;
		for (int c = 0; c < static_cast<int>(cities.size()); ++c)
			homes[cities[c].owner].push_back(c);
		std::array<int, 3> count{};
		for (const auto& e : entries)
		{
			const auto& available = homes[e.faction];
			General g{e.name, e.faction, available[count[e.faction]++ % available.size()], e.leadership};
			g.politics = e.politics;
			g.intelligence = e.intelligence;
			g.charm = e.charm;
			g.specialty = e.duty;
			g.trait = e.trait;
			g.tactic = e.tactic;
			generals.push_back(g);
		}
	}
} // namespace frontline
