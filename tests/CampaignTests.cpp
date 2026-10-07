#include "../NewGame/Campaign.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <set>

#undef assert
#define assert(expr) do { if(!(expr)){std::cerr << "Assertion failed at line " << __LINE__ << ": " << #expr << "\n";std::exit(1);} } while(false)
using namespace frontline;
Campaign Empty()
{
	Campaign game; game.Reset(0);
	for (auto& tile : game.tiles) tile = Tile{};
	game.cities = {{U"Base",Campaign::At(1,1),0}};
	game.tiles[game.cities[0].tile].owner = 0;
	game.generals = {{U"A",0,0,80},{U"B",1,0,80},{U"C",0,0,80}};
	return game;
}
Army Force(int faction, int general, int tile, Arm arm = Arm::Spear)
{
	Army a; a.faction = faction; a.general = general; a.tile = a.target = tile; a.troops = 3000; a.arm = arm; return a;
}
int main()
{
	Campaign game; game.Reset(0);
	assert(game.cities.size() == 30 && game.generals.size() == 24);
	// Preparation takes time; a working officer cannot simultaneously lead an army.
	game.BeginTurn(); assert(game.armies.empty());
	assert(game.Develop(0,19,Duty::Commerce));
	assert(game.cities[0].gold == 2500 && game.commands == 2);
	assert(game.Deploy(0,19,3000,Arm::Spear) == -1);
	assert(!game.Develop(0,0,Duty::Farming));
	assert(game.WorkGain(19,Duty::Commerce) > game.WorkGain(19,Duty::Farming));
	for (int i = 0; i < 29; ++i) game.AdvanceDay();
	assert(game.cities[0].commerce == 40 && game.cities[0].worker == 19);
	game.AdvanceDay(); assert(game.cities[0].commerce > 40 && game.cities[0].worker == -1);
	assert(game.cities[0].troops == 10000); // Income does not create soldiers.
	const int oldOrder = game.cities[0].order; assert(game.Recruit(0)); assert(game.cities[0].order == oldOrder - 10);
	game.cities[0].order = 34; assert(!game.Recruit(0));
	game.Reset(0); assert(game.Develop(0,19,Duty::Commerce)); assert(game.CancelWork(0));
	assert(game.cities[0].gold == 2500 && !game.Busy(19));
	// Both officers are reserved for joint work; completion grows a symmetric bond.
	game.Reset(0); const int beforeBond = game.Affinity(19,1);
	assert(!game.Develop(0,19,Duty::Commerce,false,19));
	assert(game.Develop(0,19,Duty::Commerce,false,1));
	assert(game.Busy(19) && game.Busy(1) && game.Deploy(0,1,3000,Arm::Spear) == -1);
	for (int i = 0; i < 30; ++i) game.AdvanceDay();
	assert(game.Affinity(19,1) == beforeBond + 8 && game.Affinity(1,19) == game.Affinity(19,1));
	assert(!game.Busy(19) && !game.Busy(1) && game.cities[0].commerce > 69);
	// A merchant and an eloquent envoy have different strengths from a fighter.
	assert(game.WorkGain(19,Duty::Commerce) > game.WorkGain(2,Duty::Commerce));
	assert(game.MissionChance(18,3,MissionKind::Diplomacy) > game.MissionChance(2,3,MissionKind::Diplomacy));
	assert(game.MissionChance(5,3,MissionKind::Sabotage) > game.MissionChance(2,3,MissionKind::Sabotage));
	const int guardedChance = game.MissionChance(5,3,MissionKind::Sabotage);
	game.armies = {Force(1,6,Campaign::At(30,10))}; assert(game.MissionChance(5,3,MissionKind::Sabotage) > guardedChance);
	game.Reset(0); game.randomState = 1;
	assert(game.SendMission(0,19,3,MissionKind::Diplomacy,1)); assert(game.Busy(19) && game.Busy(1));
	for (int i = 0; i < 19; ++i) game.AdvanceDay(); assert(game.Hostile(0,1) && game.missions.size() == 1);
	game.AdvanceDay(); assert(game.missions.empty() && !game.Hostile(0,1) && game.truceUntil[0][1] == 80);
	assert(game.Affinity(19,game.Leader(1)) == 10);
	assert(!game.SendMission(2,5,3,MissionKind::Sabotage));
	game.armies = {Force(0,4,Campaign::At(5,5)),Force(1,7,Campaign::At(6,5))};
	game.AdvanceDay(); assert(game.armies[0].troops == 3000 && game.armies[1].troops == 3000);
	assert(!game.Order(0,game.cities[3].tile));
	game.armies[1].tile = Campaign::At(10,5); game.tiles[Campaign::At(6,5)].owner = 1;
	assert(game.Order(0,Campaign::At(6,5))); game.AdvanceDay(); assert(game.tiles[Campaign::At(6,5)].owner == 1);
	game.armies.clear(); while (game.day < 80) game.AdvanceDay(); assert(game.Hostile(0,1));
	// A sabotage attempt takes 30 days and affects real city resources on success.
	game.Reset(0); game.randomState = 2; assert(game.SendMission(2,5,3,MissionKind::Sabotage));
	for (int i = 0; i < 30; ++i) game.AdvanceDay(); assert(game.missions.empty() && game.cities[3].order < 50 && game.cities[3].food < 24000);
	// Fortify changes actual losses, costs morale and cannot be queued twice.
	game.Reset(0); game.armies = {Force(0,1,Campaign::At(5,5)),Force(1,7,Campaign::At(6,5))};
	Campaign exposed = game; assert(game.ActivateTactic(0)); assert(!game.ActivateTactic(0));
	game.AdvanceDay(); exposed.AdvanceDay(); assert(game.armies[0].troops > exposed.armies[0].troops);
	assert(game.armies[0].tacticReadyDay == 31 && game.armies[0].tacticLeft == 5 && !game.ActivateTactic(0));
	// Supply transfers stores rather than creating food.
	game.Reset(0); game.armies = {Force(0,19,game.cities[0].tile)}; game.armies[0].morale = 50;
	const int carried = game.armies[0].food, store = game.cities[0].food; assert(game.ActivateTactic(0)); game.AdvanceDay();
	assert(game.armies[0].food == carried + 300 && game.cities[0].food < store - 300 && game.armies[0].morale == 67);
	// Familiar nearby commanders make the same force more effective.
	game = Empty(); game.generals[1].tactic = Tactic::Supply;
	game.armies = {Force(0,0,Campaign::At(5,5)),Force(0,2,Campaign::At(5,4)),Force(1,1,Campaign::At(6,5))};
	Campaign bonded = game; bonded.ChangeBond(0,2,60); game.AdvanceDay(); bonded.AdvanceDay();
	assert(bonded.armies[2].troops < game.armies[2].troops);
	game.Reset(0);
	// Developing logistics and choosing a logistics officer both reduce actual consumption.
	Campaign ordinary = Empty(); ordinary.generals[0].specialty = Duty::Farming;
	ordinary.armies = {Force(0,0,ordinary.cities[0].tile)};
	Campaign efficient = ordinary; efficient.cities[0].logistics = 100; efficient.generals[0].specialty = Duty::Logistics;
	ordinary.AdvanceDay(); efficient.AdvanceDay(); assert(efficient.cities[0].food > ordinary.cities[0].food);
	for (const auto& c : game.cities) assert(game.tiles[c.tile].owner == c.owner && game.Cost(c.tile,Arm::Spear) < 100000);
	assert(game.Deploy(3,6,3000,Arm::Spear) == -1);
	assert(game.Deploy(0,0,10000,Arm::Spear) == -1);
	const int deployed = game.Deploy(0,0,3000,Arm::Spear);
	assert(deployed == 0 && game.commands == 2 && game.cities[0].troops == 7000 && game.cities[0].gold == 2700);
	assert(game.Deploy(0,0,3000,Arm::Spear) == -1);
	assert(!game.Order(0,Campaign::At(95,0)));
	assert(game.Order(0,game.cities[3].tile));
	int previous = game.armies[0].tile;
	for (int p : game.armies[0].path) { assert(Campaign::Distance(previous,p) == 1); previous = p; }
	assert(previous == game.cities[3].tile);

	// A cut in a one-cell corridor removes the rear army's supply.
	game = Empty();
	for (int x = 1; x <= 5; ++x) game.tiles[Campaign::At(x,1)].owner = 0;
	assert(game.Supply(0)[Campaign::At(5,1)] == 0);
	game.armies.push_back(Force(1,1,Campaign::At(3,1)));
	assert(game.Supply(0)[Campaign::At(5,1)] == -1);
	game.armies.push_back(Force(0,0,Campaign::At(5,1)));
	game.armies[1].food = 0; game.armies[1].morale = 50;
	game.AdvanceDay(); assert(!game.armies[1].supplied && game.armies[1].troops < 3000 && game.armies[1].morale < 50);

	// Combat casualties do not depend on iteration order.
	game = Empty(); game.armies = {Force(0,0,Campaign::At(5,5)),Force(1,1,Campaign::At(6,5))};
	Campaign reversed = game; std::reverse(reversed.armies.begin(),reversed.armies.end());
	game.AdvanceDay(); reversed.AdvanceDay();
	assert(game.armies[0].troops == reversed.armies[1].troops && game.armies[1].troops == reversed.armies[0].troops);
	assert(game.armies[0].troops < 3000 && game.armies[1].troops < 3000);
	const int plainTroops = game.armies[1].troops;
	game = Empty(); game.armies = {Force(0,0,Campaign::At(5,5)),Force(1,1,Campaign::At(6,5))};
	game.tiles[Campaign::At(6,5)].terrain = Terrain::Forest; game.AdvanceDay(); assert(game.armies[1].troops > plainTroops);

	// Archers firing at range two must not pin approaching melee troops in place.
	game = Empty(); game.armies = {Force(0,0,Campaign::At(5,5),Arm::Bow),Force(1,1,Campaign::At(7,5))};
	game.Order(1,Campaign::At(5,5)); game.AdvanceDay();
	assert(game.armies[1].tile == Campaign::At(6,5));

	// Stacking several units in the same direction does not count as encirclement.
	game = Empty(); game.armies = {Force(0,0,Campaign::At(5,5)),Force(0,2,Campaign::At(5,5))};
	assert(game.Fronts(Campaign::At(6,5),0) == 1);
	game.armies[1].tile = Campaign::At(6,4); assert(game.Fronts(Campaign::At(6,5),0) == 2);

	// Siege captures a city and transfers its central territory; troops then return.
	game = Empty(); game.cities.push_back({U"Castle",Campaign::At(6,5),1,800,1000,1000}); game.tiles[Campaign::At(6,5)].owner = 1;
	game.generals[1].home = 1; game.cities[1].worker = 1; game.cities[1].workLeft = 30;
	game.cities.push_back({U"Outpost",Campaign::At(20,5),1}); game.tiles[Campaign::At(20,5)].owner = 1;
	game.armies = {Force(0,0,Campaign::At(5,5),Arm::Siege)}; game.Order(0,Campaign::At(6,5));
	for (int i = 0; i < 20 && game.cities[1].owner != 0; ++i) game.AdvanceDay();
	assert(game.cities[1].owner == 0 && game.tiles[Campaign::At(6,5)].owner == 0);
	assert(game.cities[1].worker == -1 && game.cities[1].workLeft == 0 && game.cities[1].order < 65);
	for (int i = 0; i < 3 && game.armies[0].troops > 0; ++i) game.AdvanceDay();
	assert(game.armies[0].troops == 0 && game.generals[0].home == 1 && game.cities[1].troops > 0);

	// A longer AI campaign must retain ownership, resources and commander uniqueness.
	// Assigning a guardian vs a breakthrough officer changes actual battle outcomes.
	Campaign attackTeam=Empty();attackTeam.generals[0].name=U"劉備";attackTeam.generals[2].name=U"張飛";
	attackTeam.armies={Force(0,0,Campaign::At(5,5)),Force(0,2,Campaign::At(4,4)),Force(1,1,Campaign::At(6,5))};attackTeam.ChangeBond(0,2,60);
	Campaign guardTeam=attackTeam;guardTeam.generals[2].name=U"趙雲";
	attackTeam.AdvanceDay();guardTeam.AdvanceDay();assert(guardTeam.armies[0].troops>attackTeam.armies[0].troops&&attackTeam.armies[2].troops<guardTeam.armies[2].troops);
	Campaign scattered=guardTeam;scattered.armies[1].tile=Campaign::At(1,8);assert(scattered.Formation(0).defense==0);
	guardTeam.armies[1].retreat=true;assert(guardTeam.Formation(0).defense==0);
	// Administration feeds deployment. Extra provisions must come from city stores.
	Campaign prepared;prepared.Reset(0);Campaign neglected=prepared;neglected.cities[0].order=30;neglected.cities[0].logistics=0;
	prepared.cities[0].order=100;prepared.cities[0].logistics=100;
	const int provisionStore=prepared.cities[0].food;assert(prepared.Deploy(0,0,3000,Arm::Spear)==0&&neglected.Deploy(0,0,3000,Arm::Spear)==0);
	assert(prepared.armies[0].morale>neglected.armies[0].morale&&prepared.armies[0].food>neglected.armies[0].food&&prepared.cities[0].food+prepared.armies[0].food==provisionStore);
	Campaign supplyTeam=Empty();supplyTeam.generals[2].name=U"徐晃";supplyTeam.armies={Force(0,0,Campaign::At(5,5)),Force(0,2,Campaign::At(4,4))};supplyTeam.ChangeBond(0,2,60);
	Campaign separatedSupply=supplyTeam;separatedSupply.armies[1].tile=Campaign::At(1,8);supplyTeam.AdvanceDay();separatedSupply.AdvanceDay();assert(supplyTeam.armies[0].food>separatedSupply.armies[0].food);
	game.Reset(0);
	for (int day = 0; day < 300 && game.result == 0; ++day)
	{
		if (day % 10 == 0) game.BeginTurn(); game.AdvanceDay();
		std::set<int> commanders;
		for (const auto& c : game.cities) assert(c.owner >= 0 && c.owner < 3 && c.troops >= 0 && c.food >= 0 && c.gold >= 0 && game.tiles[c.tile].owner == c.owner);
		for (const auto& a : game.armies) if (a.troops > 0)
		{
			assert(a.morale >= 0 && a.morale <= 100 && a.food >= 0 && game.Cost(a.tile,a.arm) < 100000);
			assert(commanders.insert(a.general).second && game.generals[a.general].faction == a.faction);
			if (!a.path.empty()) assert(Campaign::Distance(a.tile,a.path.front()) == 1);
		}
	}
	std::cout << "Campaign tests passed\n";
}
