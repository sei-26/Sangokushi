#pragma once
#include "Campaign.hpp"
namespace siegestatus
{
	struct Status
	{
		int armies = 0, troops = 0, engines = 0, unsupplied = 0, fronts = 0, bonus = 0;
		std::vector<int> occupied;
	};
	inline Status Of(const frontline::Campaign& game, int city)
	{
		Status s;
		if (city < 0 || city >= static_cast<int>(game.cities.size()))
			return s;
		const auto& c = game.cities[city];
		const auto neighbors = game.MapNeighbors(c.tile);
		s.occupied.resize(neighbors.size());
		for (const auto& a : game.armies)
		{
			if (a.troops <= 0 || a.arm == frontline::Arm::Transport || !game.Hostile(a.faction, c.owner))
				continue;
			for (size_t i = 0; i < neighbors.size(); ++i)
				if (a.tile == neighbors[i])
					s.occupied[i] = 1;
			if (a.retreat || a.target != c.tile || game.MapDistance(a.tile, c.tile) > 1)
				continue;
			++s.armies;
			s.troops += a.troops;
			if (a.arm == frontline::Arm::Siege)
				++s.engines;
			if (!a.supplied)
				++s.unsupplied;
			s.fronts = std::max(s.fronts, game.Fronts(c.tile, a.faction));
		}
		s.bonus = std::min(3, std::max(0, s.fronts - 1)) * 20;
		return s;
	}
} // namespace siegestatus
