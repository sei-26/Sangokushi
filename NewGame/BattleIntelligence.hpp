#pragma once
#include "Campaign.hpp"

namespace frontline
{
	struct ArmyContact
	{
		int id = -1, seenDay = -1;
		bool current = false;
		Army observed; // Position and visible attributes only; never enemy orders or cargo.
	};
	class BattleIntelligence
	{
	public:
		std::array<bool, TileCount> visible{};
		std::array<int, TileCount> owners{};
		std::vector<ArmyContact> contacts;
		void Reset(const Campaign& world);
		void Observe(const Campaign& world, int faction, bool revealAll = false);
		const ArmyContact* Find(int army) const;
		bool EnemyNear(const Campaign& world, int tile, int radius) const;
	};
} // namespace frontline
