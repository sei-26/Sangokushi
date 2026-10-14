#include "../NewGame/StrategistBattle.hpp"
#include <cassert>
#include <iostream>
#include <memory>

using namespace frontline;
static void RemoveEnemies(StrategistBattle& battle)
{
	for (int i = 5; i < 9; ++i)
		battle.world.armies[i].troops = 0;
}
int main()
{
	auto storage = std::make_unique<StrategistBattle>();
	auto& battle = *storage;
	battle.Reset();
	assert(battle.world.hexMap && battle.world.armies.size() == 9);
	assert(battle.RemainingSoldiers() == 12000 && battle.Cargo() == 5000);
	assert(!battle.Order(0, Campaign::At(12, 12)));
	assert(!battle.SetPlan(-1) && !battle.SetPlan(3));
	assert(battle.SetPlan(0));
	assert(battle.world.armies[4].path.empty()); // Departure is a player decision.
	assert(!battle.Order(5, Campaign::At(12, 12)));
	assert(!battle.Order(0, Campaign::At(60, 12)) && !battle.Order(0, Campaign::At(0, 0)));
	assert(!battle.SetStance(4, battle::Stance::Guard));
	assert(battle.SetStance(1, battle::Stance::Guard));
	assert(battle.Order(4, battle.world.cities[1].tile));
	for (int tile : battle.world.armies[4].path)
		assert(StrategistBattle::OnMap(tile) && battle.world.tiles[tile].owner == 0);
	RemoveEnemies(battle);
	const int initialFrontFood = battle.world.cities[1].food;
	while (!battle.outcome && battle.world.day < 45)
		battle.AdvanceDay();
	assert(battle.outcome == 1 && battle.deliveredCargo == 5000);
	assert(battle.world.cities[1].food > initialFrontFood && battle.world.armies[4].cargoFood == 0);
	assert(battle.RemainingSoldiers() == 12000 && battle.world.result == 0);
	const int completedDay = battle.world.day;
	battle.AdvanceDay();
	assert(battle.world.day == completedDay && !battle.Order(0, Campaign::At(8, 12)));

	// With real enemies present, the cautious opponent allows this opening.
	// Changing only the enemy commitment makes the same opening fail.
	battle.Reset();
	battle.intelligenceEnabled = false;
	battle.RefreshIntelligence();
	battle.SetPlan(0);
	battle.Order(4, battle.world.cities[1].tile);
	while (!battle.outcome)
		battle.AdvanceDay();
	assert(battle.outcome == 1 && battle.deliveredCargo >= StrategistBattle::RequiredCargo);
	assert(battle.RemainingSoldiers() >= StrategistBattle::RequiredSoldiers);
	battle.Reset();
	battle.intelligenceEnabled = false;
	battle.RefreshIntelligence();
	battle.enemyPolicy = 0;
	battle.SetPlan(0);
	battle.Order(4, battle.world.cities[1].tile);
	while (!battle.outcome)
		battle.AdvanceDay();
	assert(battle.outcome == 2);

	// A return event contains the actual survivors after simultaneous combat,
	// rather than the strength the army had at the start of the day.
	battle.Reset();
	battle.SetPlan(0);
	for (int i = 6; i < 9; ++i)
		battle.world.armies[i].troops = 0;
	auto& army = battle.world.armies[0];
	army.tile = Campaign::At(3, 12);
	army.movement = 12;
	battle.world.armies[5].tile = Campaign::At(3, 11);
	assert(battle.Order(0, battle.world.cities[0].tile, true));
	battle.AdvanceDay();
	assert(army.troops == 0 && battle.evacuatedSoldiers > 0 && battle.evacuatedSoldiers < 3000);
	assert(!battle.world.returnEvents.empty());
	assert(battle.RemainingSoldiers() == 9000 + battle.evacuatedSoldiers);
	assert(!battle.Order(1, Campaign::At(8, 8))); // No changes inside a progress phase.
	const int evacuated = battle.evacuatedSoldiers;
	battle.AdvanceDay();
	assert(battle.evacuatedSoldiers == evacuated); // Daily events are never counted twice.

	// Waiting has a cost, and a battle day does not recruit, earn monthly income,
	// or declare the campaign's nationwide victory.
	battle.Reset();
	battle.SetPlan(2);
	RemoveEnemies(battle);
	for (int i = 0; i < 4; ++i)
		battle.world.Order(i, battle.world.armies[i].tile);
	const int initialFood = battle.world.cities[0].food;
	while (!battle.outcome)
		battle.AdvanceDay();
	assert(battle.world.day == 45 && battle.outcome == 2 && battle.deliveredCargo == 0);
	assert(battle.world.cities[0].food < initialFood && battle.world.armies.size() == 9);

	battle.Reset();
	battle.SetPlan(0);
	battle.world.cities[1].owner = 1;
	battle.AdvanceDay();
	assert(battle.outcome == 2);
	battle.Reset();
	battle.SetPlan(0);
	battle.world.armies[4].troops = 0;
	battle.AdvanceDay();
	assert(battle.outcome == 2 && battle.deliveredCargo == 0);
	battle.Reset();
	battle.SetPlan(0);
	battle.world.armies[0].troops = 0;
	battle.world.armies[1].troops = 0;
	battle.AdvanceDay();
	assert(battle.outcome == 2);

	// All initial plans and enemy policies finish within the advertised deadline.
	// This is a termination check, not evidence that the balance is fun.
	for (int policy = 0; policy < 3; ++policy)
		for (int plan = 0; plan < 3; ++plan)
		{
			battle.Reset();
			battle.enemyPolicy = policy;
			battle.SetPlan(plan);
			assert(battle.Order(4, battle.world.cities[1].tile));
			for (int day = 0; day < 45 && !battle.outcome; ++day)
				battle.AdvanceDay();
			assert(battle.outcome != 0 && battle.world.day <= 45);
			std::cout << "Plan " << plan << " / enemy " << policy << ": day " << battle.world.day
			          << ", result " << battle.outcome << ", remaining " << battle.RemainingSoldiers()
			          << '\n';
		}
	std::cout << "Strategist orders, delivery, real survivors, daily isolation and outcomes passed\n";
}
