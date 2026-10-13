#include "../NewGame/Campaign.hpp"
#include <cassert>
#include <iostream>
using namespace frontline;
int main()
{
	Campaign g;
	g.Reset(0);
	assert(g.cities[0].level == 0 && g.PolicySlots(0) == 1);
	auto& c = g.cities[0];
	const int gold = g.CityIncome(0, false), recruits = g.Recruitment(0), defense = g.CityDamagePercent(0);
	c.farming = c.commerce = c.logistics = 55;
	c.order = 54;
	g.AdvanceDay();
	assert(c.level == 0);
	c.order = 55;
	const auto revision = g.revision;
	g.AdvanceDay();
	assert(c.level == 1 && g.revision > revision && g.PolicySlots(0) == 2);
	assert(g.CityIncome(0, false) > gold && g.Recruitment(0) > recruits && g.CityDamagePercent(0) < defense);
	c.farming = c.commerce = c.logistics = c.order = 70;
	g.AdvanceDay();
	assert(c.level == 2 && g.PolicySlots(0) == 3);
	c.farming = c.commerce = c.logistics = c.order = 85;
	g.AdvanceDay();
	assert(c.level == 3 && g.GarrisonCapacity(0) == 27000);
	c.order = 20;
	g.AdvanceDay();
	assert(c.level == 3); // Growth survives temporary unrest.
	g.Reset(0);
	const int baseFood = g.CityIncome(0, true), baseGold = g.CityIncome(0, false),
	          baseWork = g.WorkGain(0, Duty::Farming);
	const int otherFood = g.CityIncome(3, true), budget = g.commands;
	assert(!g.AdoptPolicy(1, 19, Duty::Commerce));
	assert(g.AdoptPolicy(0, 0, Duty::Farming));
	assert(g.commands == budget - 1 && g.cities[0].gold == 2800 && g.generals[0].policy == 0);
	assert(g.PolicyPower(0, Duty::Farming) == g.PolicyStrength(0, Duty::Farming) &&
	       g.CityIncome(0, true) > baseFood);
	assert(g.CityIncome(0, false) < baseGold && g.WorkGain(0, Duty::Farming) > baseWork &&
	       g.CityIncome(3, true) == otherFood);
	assert(!g.AdoptPolicy(0, 0, Duty::Farming)); // No charging for unchanged adoption.
	g.cities[0].level = 1;
	assert(!g.AdoptPolicy(1, 19, Duty::Farming) && !g.AdoptPolicy(1, 0, Duty::Commerce));
	assert(g.AdoptPolicy(1, 19, Duty::Commerce));
	assert(g.PolicyPower(0, Duty::Commerce) == g.PolicyStrength(19, Duty::Commerce));
	g.cities[0].owner = 1;
	assert(g.PolicyPower(0, Duty::Farming) == 0 && g.PolicyPower(0, Duty::Commerce) == 0);
	g.cities[0].owner = 0;
	Army army;
	army.general = 0;
	army.troops = 1000;
	g.armies.push_back(army);
	assert(g.PolicyPower(0, Duty::Farming) == 0);
	g.armies.clear();
	assert(g.PolicyPower(0, Duty::Farming) == g.PolicyStrength(0, Duty::Farming));
	g.cities[0].level = 0;
	assert(g.PolicyPower(0, Duty::Commerce) == 0 && g.policyCouncil[0][1] == 19);
	assert(g.RemovePolicy(0, 1) && g.policyCouncil[0][1] == -1);
	g.commands = 0;
	assert(!g.AdoptPolicy(0, 19, Duty::Commerce) && !g.RemovePolicy(0, 0));
	g.Reset(0);
	g.generals[0].home = 3;
	assert(!g.AdoptPolicy(0, 0, Duty::Farming));
	Campaign distinct;
	distinct.Reset(0);
	auto& officer = distinct.generals[0];
	officer.politics = 80;
	officer.specialty = Duty::Farming;
	officer.trait = Trait::Farmer;
	assert(distinct.PolicyStrength(0, Duty::Farming) == 23 &&
	       distinct.PolicyStrength(0, Duty::Commerce) == 18);
	officer.trait = Trait::Administrator;
	assert(distinct.PolicyStrength(0, Duty::Farming) == 22 &&
	       distinct.PolicyStrength(0, Duty::Commerce) == 19);
	std::cout << "City thresholds, persistent growth, national slots, policy costs, tradeoffs and absence "
	             "passed.\n";
}
