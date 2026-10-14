#include "StrategistBattle.hpp"
#include <memory>

namespace frontline
{
	void StrategistBattle::Report(const std::u32string& text)
	{
		reports.push_back({world.day, text});
		world.Note(text);
	}
	void StrategistBattle::RefreshIntelligence()
	{
		for (int faction = 0; faction < 2; ++faction)
			intelligence[faction].Observe(world, faction, !intelligenceEnabled);
	}
	std::vector<int> StrategistBattle::KnownRoute(int army, int target) const
	{
		if (army < 0 || army >= static_cast<int>(world.armies.size()) || !OnMap(target))
			return {};
		const auto& unit = world.armies[army];
		if (unit.faction < 0 || unit.faction >= 2)
			return {};
		const auto& known = intelligence[unit.faction];
		// Path planning receives the same remembered world as the UI and orders.
		// Physical movement still checks the real battlefield in Campaign.
		auto view = std::make_unique<Campaign>(world);
		for (int tile = 0; tile < TileCount; ++tile)
			view->tiles[tile].owner = known.owners[tile];
		for (auto& city : view->cities)
			city.owner = known.owners[city.tile];
		for (auto& other : view->armies)
			if (other.faction != unit.faction)
				other.troops = 0;
		for (const auto& contact : known.contacts)
			view->armies[contact.id] = contact.observed;
		return view->Route(unit.tile, target, unit.arm, unit.faction);
	}
	int StrategistBattle::EnemyTarget(int army) const
	{
		if (army < 5 || army >= static_cast<int>(world.armies.size()) || world.armies[army].troops <= 0)
			return -1;
		const auto& unit = world.armies[army];
		if (unit.troops < 900 || unit.morale < 30)
			return world.cities[2].tile;
		int target = Campaign::At(11, 12);
		if (enemyPolicy == 1 && army == 8)
			return Campaign::At(17, 9);
		if (enemyPolicy == 2 || army == 7)
			target = world.cities[Destination].tile;
		for (const auto& contact : intelligence[unit.faction].contacts)
		{
			// Old reports guide a search, but not forever. Never use the hidden
			// current position or strength to override a remembered contact.
			if (world.day - contact.seenDay > 5 || world.MapDistance(unit.tile, contact.observed.tile) > 4)
				continue;
			target = contact.observed.tile;
			if (contact.observed.arm == Arm::Transport)
				break;
		}
		return target;
	}
} // namespace frontline
