#include "../NewGame/Campaign.hpp"
#include "../NewGame/CampaignInformation.hpp"
#include <cassert>
#include <iostream>
using namespace frontline;
Campaign Siege(bool fortress, bool governor)
{
	Campaign g;
	g.Reset(0);
	g.hexMap = false;
	g.regions.clear();
	for (auto& t : g.tiles)
	{
		t.terrain = Terrain::Plain;
		t.owner = 0;
	}
	g.cities = {{fortress ? U"漢中" : U"Test", Campaign::At(2, 2), 0},
	            {U"Enemy", Campaign::At(12, 2), 1},
	            {U"Refuge", Campaign::At(2, 12), 0}};
	g.generals = {{U"Governor", 0, 0, 90, 0, 90}, {U"Attacker", 1, 1, 80}};
	for (const auto& c : g.cities)
		g.tiles[c.tile].owner = c.owner;
	if (governor)
		assert(g.AppointGovernor(0, 0));
	Army a;
	a.general = 1;
	a.faction = 1;
	a.tile = Campaign::At(3, 2);
	a.target = g.cities[0].tile;
	a.troops = 3000;
	a.arm = Arm::Siege;
	g.armies = {a};
	return g;
}
int main()
{
	Campaign g;
	g.Reset(0);
	for (int c = 0; c < 30; ++c)
		assert(g.CityKind(c) != cityidentity::Kind::Ordinary);
	assert(g.CityKind(0) == cityidentity::Kind::Granary && g.CityKind(3) == cityidentity::Kind::Market);
	assert(g.CityIncome(0, true) == 3640 && g.CityIncome(3, false) == 637);
	const int baseGold = g.CityIncome(0, false), baseFood = g.CityIncome(0, true),
	          baseDefense = g.CityDamagePercent(0);
	assert(g.AppointGovernor(0, 19) && g.commands == 2 && g.cities[0].gold == 2900);
	assert(g.Busy(19) && information::PostOf(g, 19).kind == information::Post::Governor);
	assert(g.Deploy(0, 19, 2000, Arm::Spear) < 0 && !g.Develop(0, 19, Duty::Farming) &&
	       !g.AssignOfficer(19, 9) && !g.SendMission(0, 19, 3, MissionKind::Diplomacy));
	assert(g.CityIncome(0, false) > baseGold && g.CityIncome(0, true) > baseFood &&
	       g.CityDamagePercent(0) < baseDefense);
	assert(!g.AppointGovernor(3, 6) && !g.AppointGovernor(0, 6) && !g.AppointGovernor(0, 19));
	const int liveGold = g.cities[0].gold, liveGovernor = g.Governor(0);
	assert(g.CityIncome(0, false, 0) > 0 && g.CityLogistics(0, 0) > 40 && g.CityDamagePercent(0, 0) < 100);
	assert(g.Governor(0) == liveGovernor && g.cities[0].gold == liveGold);
	assert(g.Develop(0, 0, Duty::Farming));
	const int gain = g.CityWorkGain(0);
	g.ChangeBond(19, 0, 60);
	assert(g.CityWorkGain(0) > gain);
	assert(g.AppointGovernor(0, -1) && !g.Busy(19));
	assert(!g.AppointGovernor(0, 19) && g.commands == 0);
	g.Reset(0);
	assert(g.AppointGovernor(0, 19));
	const int expectedGold = g.CityIncome(0, false), expectedFood = g.CityIncome(0, true),
	          gold = g.cities[0].gold, food = g.cities[0].food;
	g.day = 29;
	g.regions.clear();
	g.AdvanceDay();
	assert(g.cities[0].gold == gold + expectedGold && g.cities[0].food == food + expectedFood);
	// Troops and provisions come from actual stores even when logistics bonuses apply.
	Campaign plain;
	plain.Reset(0);
	plain.cities[0].name = U"Test";
	Campaign hub = plain;
	hub.cities[0].name = U"江陵";
	assert(hub.CityLogistics(0) == plain.CityLogistics(0) + 20);
	int store = hub.cities[0].food;
	assert(hub.Deploy(0, 0, 3000, Arm::Spear) >= 0);
	assert(hub.cities[0].food + hub.armies[0].food == store);
	assert(plain.Deploy(0, 0, 3000, Arm::Spear) >= 0 && hub.armies[0].food > plain.armies[0].food);
	const int beforeHub = hub.cities[0].food, beforePlain = plain.cities[0].food;
	hub.AdvanceDay();
	plain.AdvanceDay();
	assert(beforeHub - hub.cities[0].food < beforePlain - plain.cities[0].food);
	Campaign military;
	military.Reset(0);
	military.cities[0].name = U"武威";
	assert(military.Recruit(0) && military.cities[0].troops == 12500 && military.cities[0].order == 55);
	Campaign normal = Siege(false, false), fort = Siege(true, false), governed = Siege(true, true);
	normal.AdvanceDay();
	fort.AdvanceDay();
	governed.AdvanceDay();
	assert(normal.cities[0].troops < fort.cities[0].troops &&
	       fort.cities[0].troops < governed.cities[0].troops);
	governed = Siege(true, true);
	governed.cities[0].troops = 1;
	governed.AdvanceDay();
	assert(governed.cities[0].owner == 1 && governed.cities[0].governor == -1 &&
	       governed.generals[0].home == 2 && !governed.Busy(0));
	Campaign ai;
	ai.Reset(0);
	ai.day = 30;
	for (auto& c : ai.cities)
		c.owner = 0;
	ai.cities[3].owner = 1;
	ai.generals = {{U"Commander", 1, 3, 95, 0, 30}, {U"Administrator", 1, 3, 40, 0, 90}};
	ai.BeginTurn();
	assert(ai.Governor(3) == 1 && !ai.Busy(0) && ai.commands == 3);
	const int budget = ai.aiCommands[1];
	ai.BeginTurn();
	assert(ai.aiCommands[1] == budget);
	Campaign legacy;
	legacy.ResetLegacy(0);
	assert(legacy.CityKind(0) == cityidentity::Kind::Ordinary && legacy.Governor(0) == -1);
	std::cout << "City identities, governor reservations, income, logistics conservation, bonds, siege, "
	             "capture and AI passed\n";
}
