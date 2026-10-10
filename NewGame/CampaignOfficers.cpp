#include "Campaign.hpp"

namespace frontline
{
	int Campaign::Affinity(int a, int b) const
	{
		if (a < 0 || b < 0 || a >= static_cast<int>(generals.size()) ||
		    b >= static_cast<int>(generals.size()) || a == b)
			return 0;
		if (a > b)
			std::swap(a, b);
		for (const auto& bond : bonds)
			if (bond.a == a && bond.b == b)
				return bond.value;
		return generals[a].faction == generals[b].faction ? 20 : 0;
	}

	void Campaign::ChangeBond(int a, int b, int amount)
	{
		if (a < 0 || b < 0 || a == b || a >= static_cast<int>(generals.size()) ||
		    b >= static_cast<int>(generals.size()))
			return;
		if (a > b)
			std::swap(a, b);
		for (auto& bond : bonds)
			if (bond.a == a && bond.b == b)
			{
				bond.value = std::clamp(bond.value + amount, 0, 100);
				return;
			}
		bonds.push_back({a, b, std::clamp(Affinity(a, b) + amount, 0, 100)});
	}

	bool Campaign::Busy(int general) const
	{
		for (const auto& c : cities)
			if (c.governor == general || c.worker == general || (c.worker >= 0 && c.helper == general))
				return true;
		for (const auto& transfer : assignments)
			if (transfer.general == general)
				return true;
		for (const auto& m : missions)
			if (m.general == general || m.helper == general)
				return true;
		for (const auto& a : armies)
			if (a.troops > 0 && a.general == general)
				return true;
		return false;
	}

	std::vector<int> Campaign::Available(int city) const
	{
		std::vector<int> out;
		if (city < 0 || city >= static_cast<int>(cities.size()))
			return out;
		for (int i = 0; i < static_cast<int>(generals.size()); ++i)
			if (generals[i].home == city && generals[i].faction == cities[city].owner &&
			    generals[i].readyDay <= day && !Busy(i))
				out.push_back(i);
		return out;
	}

	int Campaign::SupportBond(int army) const
	{
		const auto& a = armies[army];
		int best = 0;
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
			if (i != army && armies[i].troops > 0 && armies[i].faction == a.faction &&
			    MapDistance(a.tile, armies[i].tile) <= 2)
				best = std::max(best, Affinity(a.general, armies[i].general));
		return best;
	}

	officer::Link Campaign::Formation(int army) const
	{
		officer::Link effect;
		if (army < 0 || army >= static_cast<int>(armies.size()))
			return effect;
		const auto& a = armies[army];
		if (a.troops <= 0 || a.retreat || a.morale < 30 || (!a.supplied && a.food <= 0))
			return effect;
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			const auto& b = armies[i];
			if (i == army || b.arm == Arm::Transport || b.troops <= 0 || b.faction != a.faction ||
			    b.retreat || b.morale < 30 || (!b.supplied && b.food <= 0) || MapDistance(a.tile, b.tile) > 2)
				continue;
			officer::Merge(effect, officer::Contribution(officer::RoleOf(generals[a.general].name),
			                                             officer::RoleOf(generals[b.general].name),
			                                             Affinity(a.general, b.general)));
		}
		return effect;
	}
} // namespace frontline
