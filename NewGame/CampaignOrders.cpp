#include "Campaign.hpp"

namespace frontline
{
	int Campaign::Cost(int tile, Arm arm) const
	{
		if (!Valid(tile) || tiles[tile].terrain == Terrain::Sea)
			return 100000;
		switch (tiles[tile].terrain)
		{
		case Terrain::Mountain:
			return (arm == Arm::Siege || arm == Arm::Transport) ? 9 : 6;
		case Terrain::Forest:
			return arm == Arm::Cavalry || arm == Arm::Siege || arm == Arm::Transport ? 6 : 4;
		case Terrain::River:
			return 9;
		default:
			return 3;
		}
	}

	std::vector<int> Campaign::Route(int from, int to, Arm arm, int faction) const
	{
		if (!Valid(from) || !Valid(to) || Cost(to, arm) >= 100000)
			return {};
		std::array<int, TileCount> dist, parent;
		dist.fill(1000000);
		parent.fill(-1);
		using Entry = std::pair<int, int>;
		std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> q;
		dist[from] = 0;
		q.push({0, from});
		while (!q.empty())
		{
			const auto [distance, p] = q.top();
			q.pop();
			if (distance != dist[p])
				continue;
			if (p == to)
				break;
			for (int n : Neighbors(p))
			{
				if (arm == Arm::Transport && (tiles[n].owner != faction ||
				                              std::any_of(armies.begin(), armies.end(), [&](const Army& a) {
					                              return a.troops > 0 && a.faction != faction && a.tile == n;
				                              })))
					continue;
				const int city = CityAt(n);
				if (faction >= 0 && city >= 0 && cities[city].owner != faction && n != to)
					continue;
				const int next = distance + Cost(n, arm);
				if (next < dist[n] && Cost(n, arm) < 100000)
				{
					dist[n] = next;
					parent[n] = p;
					q.push({next, n});
				}
			}
		}
		if (dist[to] == 1000000)
			return {};
		std::vector<int> path;
		for (int p = to; p != from; p = parent[p])
			path.push_back(p);
		std::reverse(path.begin(), path.end());
		return path;
	}

	bool Campaign::Order(int index, int target, bool retreat)
	{
		if (index < 0 || index >= static_cast<int>(armies.size()) || armies[index].troops <= 0 ||
		    !Valid(target))
			return false;
		const auto& unit = armies[index];
		const int city = CityAt(target);
		if (unit.arm == Arm::Transport && target != unit.tile &&
		    (city < 0 || cities[city].owner != unit.faction))
			return false;
		if (city >= 0 && cities[city].owner != armies[index].faction &&
		    !Hostile(armies[index].faction, cities[city].owner))
			return false;
		auto path = Route(armies[index].tile, target, armies[index].arm, armies[index].faction);
		if (path.empty() && armies[index].tile != target)
			return false;
		auto& a = armies[index];
		a.target = target;
		a.path = std::move(path);
		a.retreat = retreat;
		return true;
	}

	int Campaign::Deploy(int city, int general, int soldiers, Arm arm, bool ai)
	{
		if (arm < Arm::Spear || arm > Arm::Cavalry)
			return -1;
		if (city < 0 || city >= static_cast<int>(cities.size()) || general < 0 ||
		    general >= static_cast<int>(generals.size()) || soldiers < 1000 || soldiers > 6000)
			return -1;
		auto& c = cities[city];
		const auto available = Available(city);
		if ((!ai && (c.owner != player || commands <= 0)) ||
		    std::find(available.begin(), available.end(), general) == available.end() ||
		    c.troops < soldiers + 1000 || c.gold < soldiers / 10 ||
		    c.food < officer::SupplyPack(soldiers, c.logistics) || ArmyCount(c.owner) >= MaxArmies)
			return -1;
		const int pack = officer::SupplyPack(soldiers, c.logistics);
		c.troops -= soldiers;
		c.gold -= soldiers / 10;
		c.food -= pack;
		Army a;
		a.faction = c.owner;
		a.general = general;
		a.tile = c.tile;
		a.target = c.tile;
		a.troops = soldiers;
		a.arm = arm;
		a.food = pack;
		a.morale = officer::StartingMorale(c.order);
		armies.push_back(a);
		if (!ai)
			--commands;
		Note(generals[general].name + U"が出陣。都市の治安が士気を、兵站が携行糧を支える。");
		return static_cast<int>(armies.size()) - 1;
	}

	int Campaign::NearestCity(int tile, int faction, bool enemy) const
	{
		int best = -1, distance = 100000;
		for (int i = 0; i < static_cast<int>(cities.size()); ++i)
			if ((enemy ? Hostile(cities[i].owner, faction) : cities[i].owner == faction) &&
			    Distance(tile, cities[i].tile) < distance)
			{
				distance = Distance(tile, cities[i].tile);
				best = i;
			}
		return best;
	}

	void Campaign::Return(Army& a, int city)
	{
		if (a.troops <= 0 || cities[city].owner != a.faction)
			return;
		cities[city].troops += a.troops;
		cities[city].food += a.food + a.cargoFood;
		if (a.arm == Arm::Transport)
			Note(generals[a.general].name + U"が" + cities[city].name + U"へ兵糧を届けた。");
		a.cargoFood = 0;
		generals[a.general].home = city;
		Note(generals[a.general].name + U"が" + cities[city].name + U"に帰還。");
		a.troops = 0;
		a.path.clear();
		a.tacticQueued = false;
		a.tacticLeft = 0;
	}
} // namespace frontline

namespace frontline
{
	void Campaign::MoveArmies(const std::vector<bool>& fighting)
	{
		// Resolve movement after combat. Opposing forces cannot pass through one another.
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			auto& a = armies[i];
			if (a.troops <= 0)
				continue;
			if (a.arm == Arm::Transport)
			{
				const int destination = CityAt(a.target);
				if (destination >= 0 && cities[destination].owner != a.faction)
				{
					a.target = a.tile;
					a.path.clear();
					for (int c = 0; c < static_cast<int>(cities.size()); ++c)
						if (cities[c].owner == a.faction && Order(i, cities[c].tile, true))
							break;
					Note(generals[a.general].name + U"の輸送先が陥落。安全な帰還路を探す。");
				}
				if (a.tile == a.target && CityAt(a.tile) >= 0 && cities[CityAt(a.tile)].owner == a.faction)
				{
					Return(a, CityAt(a.tile));
					continue;
				}
				if (a.tile != a.target && day % 5 == 0 &&
				    (a.path.empty() || tiles[a.path.front()].owner != a.faction ||
				     std::any_of(armies.begin(), armies.end(), [&](const Army& b) {
					     return b.troops > 0 && b.faction != a.faction && b.tile == a.path.front();
				     })))
					a.path = Route(a.tile, a.target, a.arm, a.faction);
			}
			if (a.arm != Arm::Transport && !a.retreat && fighting[i])
				continue;
			a.movement =
			    std::min(12, a.movement + (a.stance == battle::Stance::Guard
			                                   ? 2
			                                   : (a.arm == Arm::Cavalry                              ? 4
			                                      : (a.arm == Arm::Siege || a.arm == Arm::Transport) ? 2
			                                                                                         : 3)));
			if (a.path.empty())
			{
				const int currentCity = CityAt(a.tile);
				if (a.retreat && a.target == a.tile && currentCity >= 0 &&
				    cities[currentCity].owner == a.faction)
					Return(a, currentCity);
				continue;
			}
			const int next = a.path.front(), city = CityAt(next);
			if (a.arm == Arm::Transport && tiles[next].owner != a.faction)
				continue;
			if (a.movement < Cost(next, a.arm) || (city >= 0 && cities[city].owner != a.faction))
				continue;
			bool blocked = false;
			for (const auto& b : armies)
				if (b.troops > 0 && b.faction != a.faction && b.tile == next)
					blocked = true;
			if (blocked)
				continue;
			a.movement -= Cost(next, a.arm);
			a.tile = next;
			a.path.erase(a.path.begin());
			if (tiles[next].owner != a.faction &&
			    (tiles[next].owner < 0 || Hostile(tiles[next].owner, a.faction)))
			{
				tiles[next].owner = a.faction;
				++revision;
			}
			if (city >= 0 && next == a.target)
				Return(a, city);
		}
	}
} // namespace frontline
