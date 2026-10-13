#pragma once
#include "Campaign.hpp"
namespace battlefieldmoments
{
	enum class Kind
	{
		Capture,
		Defeat,
		Relief,
		SupplyLost,
		Attrition
	};
	struct Unit
	{
		int general, faction, tile, troops;
		bool supplied;
	};
	struct Snapshot
	{
		std::vector<int> owners;
		std::vector<Unit> units;
	};
	struct Event
	{
		Kind kind;
		int general = -1, faction = 0, tile = 0, city = -1, amount = 0;
	};
	inline Snapshot Observe(const frontline::Campaign& game)
	{
		Snapshot s;
		for (const auto& c : game.cities)
			s.owners.push_back(c.owner);
		for (const auto& a : game.armies)
			s.units.push_back({a.general, a.faction, a.tile, a.troops, a.supplied});
		return s;
	}
	inline std::vector<Event> Changes(const Snapshot& before, const frontline::Campaign& game)
	{
		std::vector<Event> out;
		for (int c = 0;
		     c < static_cast<int>(before.owners.size()) && c < static_cast<int>(game.cities.size()); ++c)
		{
			const auto& city = game.cities[c];
			if (city.owner == before.owners[c])
				continue;
			int general = -1, best = 0;
			for (const auto& a : game.armies)
				if (a.troops > best && a.faction == city.owner && a.target == city.tile &&
				    game.MapDistance(a.tile, city.tile) <= 1)
				{
					general = a.general;
					best = a.troops;
				}
			if (general < 0)
				general = game.Leader(city.owner);
			out.push_back({Kind::Capture, general, city.owner, city.tile, c});
		}
		for (int i = 0; i < static_cast<int>(before.units.size()) && i < static_cast<int>(game.armies.size());
		     ++i)
		{
			const auto& old = before.units[i];
			const auto& a = game.armies[i];
			if (old.troops <= 0 || old.general != a.general || old.faction != a.faction)
				continue;
			if (a.troops <= 0)
			{
				// Return/docking also sets troops to zero; recovery delay identifies actual defeat.
				if (a.general >= 0 && a.general < static_cast<int>(game.generals.size()) &&
				    game.generals[a.general].readyDay == game.day + 20)
					out.push_back({Kind::Defeat, a.general, a.faction, old.tile, -1, old.troops});
				continue;
			}
			if (a.faction != game.player)
				continue;
			if (!old.supplied && a.supplied)
				out.push_back({Kind::Relief, a.general, a.faction, a.tile});
			else if (old.supplied && !a.supplied)
				out.push_back({Kind::SupplyLost, a.general, a.faction, a.tile});
			else if (old.troops - a.troops >= 300 &&
			         std::none_of(out.begin(), out.end(), [&](const Event& e) {
				         return e.kind == Kind::Capture && e.general == a.general;
			         }))
				out.push_back({Kind::Attrition, a.general, a.faction, a.tile, -1, old.troops - a.troops});
		}
		std::stable_sort(out.begin(), out.end(), [](const Event& a, const Event& b) {
			return static_cast<int>(a.kind) < static_cast<int>(b.kind);
		});
		return out;
	}
} // namespace battlefieldmoments
