#include "Campaign.hpp"

namespace frontline
{
	std::array<bool, TileCount> Campaign::SupplyBlockade(int faction) const
	{
		std::array<bool, TileCount> blocked{}, protectedTiles{};
		for (const auto& c : cities)
			if (c.owner == faction && c.troops >= 1000)
				protectedTiles[c.tile] = true;
		for (const auto& a : armies)
			if (a.faction == faction && a.troops >= 1200 && a.morale >= 30 && a.arm != Arm::Transport &&
			    !a.retreat)
				protectedTiles[a.tile] = true;
		for (const auto& a : armies)
		{
			if (a.troops <= 0 || !Hostile(a.faction, faction))
				continue;
			blocked[a.tile] = true;
			if (a.troops < 1200 || a.morale < 30 || a.retreat || a.arm == Arm::Transport)
				continue;
			for (int n : MapNeighbors(a.tile))
				if (!protectedTiles[n])
					blocked[n] = true;
		}
		return blocked;
	}
	int Campaign::PressureDirections(int index) const
	{
		if (index < 0 || index >= static_cast<int>(armies.size()) || armies[index].troops <= 0)
			return 0;
		const auto& a = armies[index];
		int directions = 0;
		for (int n : MapNeighbors(a.tile))
			if (std::any_of(armies.begin(), armies.end(), [&](const Army& b) {
				    return b.tile == n && b.troops >= 1200 && b.morale >= 30 && !b.retreat &&
				           b.arm != Arm::Transport && Hostile(a.faction, b.faction);
			    }))
				++directions;
		return directions;
	}
	int Campaign::PressureDamagePercent(int index) const
	{
		const int bonus = std::clamp(PressureDirections(index) - 1, 0, 3) * 15;
		return bonus > 0 && armies[index].stance == battle::Stance::Guard ? bonus / 2 : bonus;
	}
	int Campaign::PressureMoraleLoss(int index) const
	{
		const int loss = std::clamp(PressureDirections(index) - 1, 0, 3) * 3;
		return loss > 0 && armies[index].stance == battle::Stance::Guard ? (loss + 1) / 2 : loss;
	}
} // namespace frontline
