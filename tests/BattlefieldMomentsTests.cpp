#include "../NewGame/BattlefieldMoments.hpp"
#include "../NewGame/SiegeStatus.hpp"
#include <cassert>
#include <iostream>
using namespace frontline;
using namespace battlefieldmoments;
int main()
{
	Campaign game;
	game.Reset(0);
	auto before = Observe(game);
	assert(Changes(before, game).empty());
	const int unit = game.Deploy(0, 0, 3000, Arm::Spear);
	before = Observe(game);
	assert(game.ReturnToCity(unit, 0));
	assert(Changes(before, game).empty());
	game.Reset(0);
	const int a = game.Deploy(0, 0, 3000, Arm::Spear);
	before = Observe(game);
	game.armies[a].troops = 0;
	game.generals[0].readyDay = game.day + 20;
	auto events = Changes(before, game);
	assert(events.size() == 1 && events[0].kind == Kind::Defeat && events[0].tile == before.units[a].tile);
	game.Reset(0);
	game.Deploy(0, 0, 3000, Arm::Spear);
	before = Observe(game);
	game.armies[0].supplied = false;
	events = Changes(before, game);
	assert(events.size() == 1 && events[0].kind == Kind::SupplyLost);
	before = Observe(game);
	game.armies[0].supplied = true;
	events = Changes(before, game);
	assert(events.size() == 1 && events[0].kind == Kind::Relief);
	before = Observe(game);
	game.armies[0].troops -= 300;
	events = Changes(before, game);
	assert(events.size() == 1 && events[0].amount == 300 && events[0].kind == Kind::Attrition);
	before = Observe(game);
	game.armies[0].troops -= 20;
	assert(Changes(before, game).empty());
	before = Observe(game);
	game.cities[3].owner = 0;
	game.armies[0].target = game.cities[3].tile;
	game.armies[0].tile = game.cities[3].tile;
	events = Changes(before, game);
	assert(events.size() == 1 && events[0].kind == Kind::Capture && events[0].general == 0 &&
	       events[0].city == 3);
	game.armies[0].troops -= 1000;
	events = Changes(before, game);
	assert(events.size() == 1 && events[0].kind == Kind::Capture);
	const auto gold = game.cities[0].gold;
	const int day = game.day, commands = game.commands;
	Changes(before, game);
	assert(game.day == day && game.commands == commands && game.cities[0].gold == gold);
	// Officers/armies may grow after deployment; observation never misidentifies a newly-created army.
	before = Observe(game);
	game.Deploy(0, 19, 2000, Arm::Bow, true);
	assert(Changes(before, game).empty());
	Snapshot empty;
	assert(Changes(empty, game).empty());
	Campaign siege;
	siege.Reset(0);
	const int first = siege.Deploy(0, 0, 3000, Arm::Siege),
	          second = siege.Deploy(0, 19, 2000, Arm::Spear, true);
	const int tile = siege.cities[3].tile;
	const auto ring = siege.MapNeighbors(tile);
	siege.armies[first].tile = ring[0];
	siege.armies[second].tile = ring[1];
	siege.armies[first].target = siege.armies[second].target = tile;
	siege.armies[second].supplied = false;
	const auto siegeGold = siege.cities[3].gold;
	const auto soldiers = siege.cities[3].troops;
	auto status = siegestatus::Of(siege, 3);
	assert(status.armies == 2 && status.troops == 5000 && status.engines == 1 && status.unsupplied == 1 &&
	       status.fronts == 2 && status.bonus == 20);
	assert(status.occupied[0] == 1 && status.occupied[1] == 1 && status.occupied.size() == 6);
	siege.armies[second].tile = ring[0];
	status = siegestatus::Of(siege, 3);
	assert(status.armies == 2 && status.fronts == 1 && status.bonus == 0);
	siege.armies[second].arm = Arm::Transport;
	status = siegestatus::Of(siege, 3);
	assert(status.armies == 1 && status.troops == 3000);
	siege.armies[first].retreat = true;
	assert(siegestatus::Of(siege, 3).armies == 0);
	siege.armies[first].retreat = false;
	siege.truceUntil[0][1] = siege.day + 10;
	assert(siegestatus::Of(siege, 3).armies == 0);
	assert(siege.cities[3].gold == siegeGold && siege.cities[3].troops == soldiers &&
	       siegestatus::Of(siege, -1).armies == 0);
	std::cout << "Battlefield reports match actual capture, defeat and supply changes; docking and small "
	             "losses stay quiet\n";
}
