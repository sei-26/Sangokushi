#include "../NewGame/Campaign.hpp"
#include <cstdlib>
#include <iostream>
#define CHECK(x)                                                                                             \
	do                                                                                                       \
	{                                                                                                        \
		if (!(x))                                                                                            \
		{                                                                                                    \
			std::cerr << "Region check " << __LINE__ << ": " << #x << std::endl;                             \
			std::exit(1);                                                                                    \
		}                                                                                                    \
	} while (false)
using namespace frontline;

int main()
{
	Campaign g;
	g.Reset(0);
	CHECK(g.regions.size() > g.cities.size() * 2);
	for (int i = 0; i < static_cast<int>(g.regions.size()); ++i)
	{
		const auto& r = g.regions[i];
		CHECK(g.RegionAt(r.tile) == i && g.Cost(r.tile, Arm::Spear) < 100000);
		CHECK(r.city >= 0 && g.cities[r.city].owner == r.owner);
	}
	int enemy = -1;
	for (int i = 0; i < static_cast<int>(g.regions.size()); ++i)
		if (g.regions[i].owner == 1 && g.CityAt(g.regions[i].tile) < 0)
		{
			enemy = i;
			break;
		}
	CHECK(enemy >= 0);
	const int unit = g.Deploy(0, 0, 3000, Arm::Spear);
	CHECK(unit >= 0);
	g.armies[unit].tile = g.armies[unit].target = g.regions[enemy].tile;
	Campaign weak = g;
	weak.armies[unit].troops = 999;
	weak.AdvanceDay();
	CHECK(weak.regions[enemy].owner == 1);
	Campaign contested = g;
	Army defender = contested.armies[unit];
	defender.faction = 1;
	defender.general = 6;
	contested.armies.push_back(defender);
	contested.AdvanceDay();
	CHECK(contested.regions[enemy].owner == 1);
	g.armies[unit].arm = Arm::Transport;
	g.AdvanceDay();
	CHECK(g.regions[enemy].owner == 1);
	g.armies[unit].arm = Arm::Spear;
	g.armies[unit].retreat = true;
	g.AdvanceDay();
	CHECK(g.regions[enemy].owner == 1);
	g.armies[unit].retreat = false;
	g.truceUntil[0][1] = g.truceUntil[1][0] = 100;
	g.AdvanceDay();
	CHECK(g.regions[enemy].owner == 1);
	g.truceUntil[0][1] = g.truceUntil[1][0] = 0;
	g.AdvanceDay();
	CHECK(g.regions[enemy].owner == 0 && g.regions[enemy].city >= 0);
	int before = 0;
	for (int p = 0; p < TileCount; ++p)
		if (g.tileRegion[p] == enemy && g.tiles[p].owner == 0)
			++before;
	g.AdvanceDay();
	int after = 0;
	for (int p = 0; p < TileCount; ++p)
		if (g.tileRegion[p] == enemy && g.tiles[p].owner == 0)
			++after;
	CHECK(after > before && after - before <= 3);
	CHECK(!g.RegionConnected(enemy));

	Campaign income;
	income.Reset(0);
	for (auto& t : income.tiles)
		if (t.terrain != Terrain::Sea)
			t.owner = 0;
	int own = 1;
	CHECK(income.regions[own].owner == 0 && income.RegionConnected(own));
	Campaign base = income;
	base.regions.clear();
	base.tileRegion.fill(-1);
	income.day = base.day = 29;
	income.AdvanceDay();
	base.AdvanceDay();
	CHECK(income.cities[0].gold > base.cities[0].gold && income.cities[0].food > base.cities[0].food);
	for (int n : income.MapNeighbors(income.regions[own].tile))
		income.tiles[n].owner = 1;
	CHECK(!income.RegionConnected(own));
    Campaign ai; ai.Reset(2); ai.day = 60;
    ai.regions[1].owner = 1; ai.regions[1].city = 3; ai.tiles[ai.regions[1].tile].owner = 1;
    ai.BeginTurn(); int dispatched = 0;
    for (const auto& a : ai.armies) if (a.troops > 0 && a.faction == 0 && a.target == ai.regions[1].tile) ++dispatched;
    CHECK(dispatched == 1);
	g.ResetLegacy(0);
	CHECK(g.regions.empty() && g.RegionAt(g.cities[0].tile) == -1);
	std::cout << "Regional hubs, capture restrictions, gradual occupation, income, blockade and legacy "
	             "compatibility passed\n";
}
