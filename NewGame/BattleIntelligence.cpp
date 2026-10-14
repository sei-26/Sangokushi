#include "BattleIntelligence.hpp"

namespace frontline
{
	void BattleIntelligence::Reset(const Campaign& world)
	{
		visible.fill(false);
		contacts.clear();
		// Initial territorial control is part of the military briefing. Subsequent
		// changes are learned only when a tile is observed.
		for (int tile = 0; tile < TileCount; ++tile)
			owners[tile] = world.tiles[tile].owner;
	}
	const ArmyContact* BattleIntelligence::Find(int army) const
	{
		for (const auto& contact : contacts)
			if (contact.id == army)
				return &contact;
		return nullptr;
	}
	void BattleIntelligence::Observe(const Campaign& world, int faction, bool revealAll)
	{
		visible.fill(false);
		const auto reveal = [&](int from, int range) {
			for (int y = std::max(0, from / Width - range); y <= std::min(Height - 1, from / Width + range);
			     ++y)
				for (int x = std::max(0, from % Width - range);
				     x <= std::min(Width - 1, from % Width + range); ++x)
				{
					const int tile = Campaign::At(x, y);
					if (world.tiles[tile].terrain != Terrain::Sea && world.MapDistance(from, tile) <= range &&
					    (world.MapDistance(from, tile) <= 1 || world.ClearShot(from, tile)))
						visible[tile] = true;
				}
		};
		if (revealAll)
		{
			for (int tile = 0; tile < TileCount; ++tile)
				visible[tile] = world.tiles[tile].terrain != Terrain::Sea;
		}
		else
		{
			for (int i = 0; i < static_cast<int>(world.armies.size()); ++i)
				if (world.armies[i].faction == faction && world.armies[i].troops > 0)
					reveal(world.armies[i].tile,
					       std::max(world.armies[i].scouting ? 5 : 3, world.AttackRange(i)));
			for (const auto& city : world.cities)
				if (city.owner == faction && city.troops > 0)
					reveal(city.tile, 2);
		}
		for (int tile = 0; tile < TileCount; ++tile)
			if (visible[tile])
				owners[tile] = world.tiles[tile].owner;
		for (auto& contact : contacts)
			contact.current = false;
		for (int i = 0; i < static_cast<int>(world.armies.size()); ++i)
		{
			const auto& army = world.armies[i];
			if (army.troops <= 0 || !world.Hostile(army.faction, faction) || !visible[army.tile])
				continue;
			auto found = std::find_if(contacts.begin(), contacts.end(),
			                          [i](const ArmyContact& c) { return c.id == i; });
			if (found == contacts.end())
			{
				contacts.push_back({});
				found = contacts.end() - 1;
			}
			found->id = i;
			found->seenDay = world.day;
			found->current = true;
			Army snapshot;
			snapshot.tile = snapshot.target = army.tile;
			snapshot.faction = army.faction;
			snapshot.general = army.general;
			snapshot.troops = army.troops;
			snapshot.morale = army.morale;
			snapshot.arm = army.arm;
			snapshot.stance = army.stance;
			snapshot.retreat = army.retreat;
			found->observed = snapshot;
		}
		// A visible empty former position invalidates the old location. Outside
		// sight, neither movement, damage nor death refreshes a contact.
		contacts.erase(
		    std::remove_if(contacts.begin(), contacts.end(),
		                   [&](const ArmyContact& c) { return !c.current && visible[c.observed.tile]; }),
		    contacts.end());
		std::sort(contacts.begin(), contacts.end(),
		          [](const ArmyContact& a, const ArmyContact& b) { return a.id < b.id; });
	}
	bool BattleIntelligence::EnemyNear(const Campaign& world, int tile, int radius) const
	{
		return std::any_of(contacts.begin(), contacts.end(), [&](const ArmyContact& c) {
			return c.current && world.MapDistance(c.observed.tile, tile) <= radius;
		});
	}
} // namespace frontline
