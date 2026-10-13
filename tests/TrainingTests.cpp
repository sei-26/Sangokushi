#include "../NewGame/CampaignInformation.hpp"
#include <cassert>
#include <iostream>
using namespace frontline;
int main()
{
	Campaign g;
	g.Reset(0);
	const int before = g.Affinity(0, 19), gold = g.cities[0].gold, food = g.cities[0].food;
	assert(g.TrainTogether(0, 0, 19) && g.cities[0].gold == gold - 300 && g.cities[0].food == food - 500 &&
	       g.commands == g.commandBudget - 1);
	assert(g.Busy(0) && g.Busy(19) && g.TrainingAt(0) == 0 &&
	       information::PostOf(g, 0).kind == information::Post::Training);
	assert(g.Deploy(0, 0, 2000, Arm::Spear) < 0 && !g.Develop(0, 19, Duty::Commerce) &&
	       !g.AssignOfficer(19, 9) && !g.AppointGovernor(0, 0));
	assert(!g.TrainTogether(0, 24, 41) && !g.TrainTogether(0, 24, 26) && !g.TrainTogether(0, 0, 19));
	for (int d = 0; d < 9; ++d)
		g.AdvanceDay();
	assert(g.Affinity(0, 19) == before && g.training[0].left == 1 && g.Busy(0));
	g.AdvanceDay();
	assert(g.Affinity(0, 19) == before + 12 && g.training.empty() && !g.Busy(0) && !g.Busy(19));
	g.Reset(0);
	assert(!g.TrainTogether(-1, 0, 19) && !g.TrainTogether(0, 0, 0) && !g.TrainTogether(0, 0, 72) &&
	       !g.TrainTogether(3, 6, 7));
	g.commands = 0;
	assert(!g.TrainTogether(0, 0, 19));
	g.commands = 3;
	g.cities[0].food = 499;
	assert(!g.TrainTogether(0, 0, 19));
	g.cities[0].food = 24000;
	g.cities[0].gold = 299;
	assert(!g.TrainTogether(0, 0, 19));
	g.Reset(0);
	g.ChangeBond(0, 19, 100);
	assert(!g.TrainTogether(0, 0, 19) && g.cities[0].gold == 3000);
	g.ChangeBond(0, 19, -4);
	assert(g.TrainTogether(0, 0, 19));
	for (int d = 0; d < 10; ++d)
		g.AdvanceDay();
	assert(g.Affinity(0, 19) == 100);
	g.Reset(0);
	assert(g.TrainTogether(0, 0, 19));
	for (int d = 0; d < 3; ++d)
		g.AdvanceDay();
	assert(!g.CancelTraining(3) && g.CancelTraining(0));
	assert(g.Affinity(0, 19) == 20 && g.cities[0].gold == 2700 && g.cities[0].food == 23500 && !g.Busy(19));
	g.Reset(0);
	assert(g.TrainTogether(0, 0, 19));
	g.Reset(0);
	assert(g.training.empty() && !g.Busy(0));
	g.ResetLegacy(0);
	assert(g.training.empty());
	// A capture interrupts both participants before refugees are relocated.
	Campaign capture;
	capture.Reset(0);
	capture.hexMap = false;
	capture.regions.clear();
	for (auto& t : capture.tiles)
		t = {Terrain::Plain, 0};
	capture.cities = {{U"Castle", Campaign::At(2, 2), 0},
	                  {U"Enemy", Campaign::At(12, 2), 1},
	                  {U"Refuge", Campaign::At(2, 12), 0}};
	capture.generals = {{U"A", 0, 0, 90}, {U"B", 0, 0, 90}, {U"C", 1, 1, 80}};
	for (const auto& c : capture.cities)
		capture.tiles[c.tile].owner = c.owner;
	assert(capture.TrainTogether(0, 0, 1));
	capture.cities[0].troops = 1;
	Army a;
	a.general = 2;
	a.faction = 1;
	a.tile = Campaign::At(3, 2);
	a.target = capture.cities[0].tile;
	a.troops = 3000;
	capture.armies = {a};
	capture.AdvanceDay();
	assert(capture.cities[0].owner == 1 && capture.training.empty() && capture.Affinity(0, 1) == 20 &&
	       capture.generals[0].home == 2 && capture.generals[1].home == 2 && !capture.Busy(0));
	std::cout << "Joint training reservations, cost, ten-day completion, caps, cancellation, reset and "
	             "capture interruption passed\n";
}
