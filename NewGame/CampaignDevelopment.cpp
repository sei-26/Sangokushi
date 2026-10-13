#include "Campaign.hpp"
namespace frontline
{
	void Campaign::AdvanceCityGrowth()
	{
		for (auto& city : cities)
		{
			const int level = citygrowth::Eligible(city);
			if (level <= city.level)
				continue;
			city.level = level;
			++revision;
			Note(city.name + U"が" + citygrowth::Name(level) +
			     U"へ発展！ 国の政策枠・募兵・防衛・兵站を強化。");
		}
	}
	int Campaign::GarrisonCapacity(int city) const
	{
		return city < 0 || city >= static_cast<int>(cities.size()) ? 0 : 18000 + cities[city].level * 3000;
	}
	int Campaign::PolicySlots(int faction) const
	{
		int largest = 0;
		for (const auto& city : cities)
			if (city.owner == faction)
				largest = std::max(largest, city.level);
		return 1 + std::min(2, largest);
	}
	bool Campaign::PolicyEligible(int general) const
	{
		if (general < 0 || general >= static_cast<int>(generals.size()))
			return false;
		const auto& g = generals[general];
		if (g.readyDay > day || g.home < 0 || g.home >= static_cast<int>(cities.size()) ||
		    cities[g.home].owner != g.faction)
			return false;
		for (const auto& a : armies)
			if (a.troops > 0 && a.general == general)
				return false;
		for (const auto& a : assignments)
			if (a.general == general)
				return false;
		for (const auto& m : missions)
			if (m.general == general || m.helper == general)
				return false;
		return true;
	}
	int Campaign::PolicyPower(int faction, Duty policy) const
	{
		if (faction < 0 || faction > 2)
			return 0;
		for (int slot = 0; slot < PolicySlots(faction); ++slot)
		{
			const int g = policyCouncil[faction][slot];
			if (PolicyEligible(g) && generals[g].faction == faction &&
			    generals[g].policy == static_cast<int>(policy))
				return PolicyStrength(g, policy);
		}
		return 0;
	}
	int Campaign::PolicyStrength(int general, Duty policy) const
	{
		if (general < 0 || general >= static_cast<int>(generals.size()))
			return 0;
		const auto& g = generals[general];
		const bool matching =
		    (policy == Duty::Farming && g.trait == Trait::Farmer) ||
		    (policy == Duty::Commerce && g.trait == Trait::Merchant) ||
		    (policy == Duty::Order && (g.trait == Trait::Benevolent || g.trait == Trait::Negotiator)) ||
		    (policy == Duty::Logistics && g.trait == Trait::Quartermaster);
		return 10 + g.politics / 10 + (g.specialty == policy ? 3 : 0) +
		       (matching                          ? 2
		        : g.trait == Trait::Administrator ? 1
		                                          : 0);
	}
	bool Campaign::AdoptPolicy(int slot, int general, Duty policy, bool ai)
	{
		const int p = static_cast<int>(policy);
		if (result != 0 || !PolicyEligible(general) || p < 0 || p > 3)
			return false;
		auto& g = generals[general];
		const int f = g.faction;
		if (slot < 0 || slot >= PolicySlots(f) || (!ai && (f != player || commands <= 0)))
			return false;
		for (int other = 0; other < 3; ++other)
		{
			const int member = policyCouncil[f][other];
			if (other != slot && member >= 0 && (member == general || generals[member].policy == p))
				return false;
		}
		if ((policyCouncil[f][slot] == general && g.policy == p) || cities[g.home].gold < 200)
			return false;
		cities[g.home].gold -= 200;
		g.policy = p;
		policyCouncil[f][slot] = general;
		if (!ai)
			--commands;
		Note(FactionName(f) + U"軍が" + g.name + U"の提案「" + nationalpolicy::Name(p) +
		     U"」を国策として採用。");
		return true;
	}
	bool Campaign::RemovePolicy(int faction, int slot)
	{
		if (result != 0 || faction != player || slot < 0 || slot > 2 || commands <= 0 ||
		    policyCouncil[faction][slot] < 0)
			return false;
		policyCouncil[faction][slot] = -1;
		--commands;
		return true;
	}
} // namespace frontline
