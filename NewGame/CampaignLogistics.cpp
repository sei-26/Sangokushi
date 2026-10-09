#include "Campaign.hpp"
namespace frontline
{
	int Campaign::TransportDays(int from, int to, int faction) const
	{
		if (from == to)
			return 0;
		const auto path = Route(from, to, Arm::Transport, faction);
		if (path.empty())
			return -1;
		int cost = 0;
		for (int p : path)
			cost += Cost(p, Arm::Transport);
		return (cost + 1) / 2;
	}
	int Campaign::DispatchTransport(int city, int general, int targetCity, int cargo, bool ai)
	{
		if (result != 0 || city < 0 || targetCity < 0 || city >= static_cast<int>(cities.size()) ||
		    targetCity >= static_cast<int>(cities.size()) || city == targetCity || general < 0 ||
		    general >= static_cast<int>(generals.size()) || cargo < 1000 || cargo > 20000)
			return -1;
		auto& c = cities[city];
		const auto available = Available(city);
		const int pack = officer::SupplyPack(1000, c.logistics);
		if ((!ai && (c.owner != player || commands <= 0)) || cities[targetCity].owner != c.owner ||
		    std::find(available.begin(), available.end(), general) == available.end() || c.troops < 2000 ||
		    c.gold < 100 || c.food < cargo + pack || ArmyCount(c.owner) >= MaxArmies)
			return -1;
		auto path = Route(c.tile, cities[targetCity].tile, Arm::Transport, c.owner);
		if (path.empty())
			return -1;
		Army a;
		a.faction = c.owner;
		a.general = general;
		a.tile = c.tile;
		a.target = cities[targetCity].tile;
		a.troops = 1000;
		a.food = pack;
		a.cargoFood = cargo;
		a.arm = Arm::Transport;
		a.morale = officer::StartingMorale(c.order);
		a.path = std::move(path);
		c.troops -= 1000;
		c.food -= cargo + pack;
		c.gold -= 100;
		armies.push_back(a);
		++revision;
		if (!ai)
			--commands;
		Note(generals[general].name + U"が" + cities[targetCity].name + U"への兵糧輸送を開始。");
		return static_cast<int>(armies.size()) - 1;
	}
} // namespace frontline
