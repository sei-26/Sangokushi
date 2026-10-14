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
static BattleOperation Task(int target)
{
	BattleOperation task;
	task.target = target;
	return task;
}
static void AdditionalScenarios()
{
	auto storage = std::make_unique<StrategistBattle>();
	auto& battle = *storage;
	battle.Reset();
	battle.SetPlan(0);
	RemoveEnemies(battle);
	auto convoy = Task(battle.world.cities[1].tile);
	convoy.trigger = OperationTrigger::AllyArrival;
	convoy.signalArmy = 0;
	convoy.signalTile = battle.world.armies[0].target;
	assert(battle.QueueOperation(4, convoy));
	battle.AdvanceDay();
	assert(battle.world.armies[4].tile == Campaign::At(3, 12));
	while (!battle.outcome)
		battle.AdvanceDay();
	assert(battle.outcome == 1 && battle.deliveredCargo == 5000);

	// Newly acquired scouting information becomes actionable on the next day,
	// not while the current snapshot's orders are still being applied.
	battle.Reset();
	battle.SetPlan(0);
	for (int i = 5; i < 8; ++i)
		battle.world.armies[i].troops = 0;
	battle.world.armies[8].tile = Campaign::At(13, 8);
	battle.world.armies[2].tile = Campaign::At(8, 8);
	auto watcher = Task(Campaign::At(8, 8));
	watcher.scout = true;
	assert(battle.QueueOperation(2, watcher));
	auto reaction = Task(Campaign::At(8, 6));
	reaction.trigger = OperationTrigger::EnemySighted;
	reaction.watchTile = Campaign::At(13, 8);
	assert(battle.QueueOperation(1, reaction));
	battle.AdvanceDay();
	assert(battle.intelligence[0].Find(8) && battle.intelligence[0].Find(8)->current);
	assert(battle.operations[1].state == OperationState::Waiting);
	battle.AdvanceDay();
	assert(battle.operations[1].state == OperationState::Executing ||
	       battle.operations[1].state == OperationState::Holding);

	// Replacing the signal source's destination cancels the now-impossible plan.
	battle.Reset();
	battle.SetPlan(0);
	auto linked = Task(Campaign::At(6, 11));
	linked.trigger = OperationTrigger::AllyArrival;
	linked.signalArmy = 0;
	linked.signalTile = battle.world.armies[0].target;
	linked.fallback = battle.world.cities[0].tile;
	assert(battle.QueueOperation(2, linked));
	assert(battle.Order(0, Campaign::At(8, 13)));
	battle.AdvanceDay();
	assert(battle.operations[2].state == OperationState::Aborted);

	// An old arrival must not release a newly issued plan after the source has
	// already withdrawn. Exercise the real holding/withdrawal/return sequence.
	battle.Reset();
	battle.SetPlan(0);
	RemoveEnemies(battle);
	battle.SetStance(0, battle::Stance::Balanced);
	auto source = Task(Campaign::At(9, 12));
	source.minMorale = 35;
	source.fallback = battle.world.cities[0].tile;
	assert(battle.QueueOperation(0, source));
	battle.AdvanceDay();
	assert(battle.arrivals[0].tile == source.target);
	battle.world.armies[0].morale = 34;
	while (battle.world.day < 10)
		battle.AdvanceDay();
	assert(battle.world.armies[0].tile != source.target);
	linked.signalTile = source.target;
	assert(battle.QueueOperation(2, linked));
	battle.AdvanceDay();
	assert(battle.operations[2].state == OperationState::Aborted);

	// Measure the information tradeoff in actual simultaneous combat, with
	// identical formation, morale and supplies in both comparisons.
	auto normal = std::make_unique<Campaign>();
	normal->hexMap = true;
	normal->generals.resize(2);
	Army friendly, enemy;
	friendly.tile = Campaign::At(5, 5);
	friendly.troops = 3000;
	friendly.stance = battle::Stance::Guard;
	enemy.tile = Campaign::At(6, 5);
	enemy.troops = 3000;
	enemy.faction = 1;
	enemy.general = 1;
	normal->armies = {friendly, enemy};
	auto scouting = std::make_unique<Campaign>(*normal);
	scouting->armies[0].scouting = true;
	normal->AdvanceBattleDay();
	scouting->AdvanceBattleDay();
	assert(scouting->armies[1].troops > normal->armies[1].troops);
	assert(scouting->armies[0].troops == normal->armies[0].troops);
}
int main()
{
	AdditionalScenarios();
	// Ordinary sight, scouting, woodland occlusion and contact aging.
	auto world = std::make_unique<Campaign>();
	world->hexMap = true;
	world->generals.resize(2);
	Army friendly, enemy;
	friendly.tile = Campaign::At(4, 4);
	friendly.troops = 3000;
	enemy.tile = Campaign::At(9, 4);
	enemy.troops = 3500;
	enemy.faction = 1;
	enemy.general = 1;
	world->armies = {friendly, enemy};
	BattleIntelligence known;
	known.Reset(*world);
	known.Observe(*world, 0);
	assert(!known.Find(1));
	world->armies[0].scouting = true;
	world->day = 5;
	known.Observe(*world, 0);
	assert(known.Find(1) && known.Find(1)->current && known.Find(1)->seenDay == 5);
	assert(known.Find(1)->observed.target == enemy.tile && known.Find(1)->observed.path.empty());
	world->armies[0].scouting = false;
	world->armies[1].tile = Campaign::At(12, 4);
	world->armies[1].troops = 800;
	world->day = 9;
	known.Observe(*world, 0);
	assert(known.Find(1) && !known.Find(1)->current && known.Find(1)->seenDay == 5);
	assert(known.Find(1)->observed.tile == enemy.tile && known.Find(1)->observed.troops == 3500);
	assert(!known.EnemyNear(*world, enemy.tile, 2)); // An old sighting is not a current trigger.
	world->armies[1].troops = 0;
	known.Observe(*world, 0);
	assert(known.Find(1)); // Hidden death must not update the report.
	world->armies[0].tile = Campaign::At(8, 4);
	known.Observe(*world, 0);
	assert(!known.Find(1)); // Re-observing an empty old location removes the ghost.
	world->armies[0].tile = Campaign::At(4, 4);
	world->armies[0].scouting = true;
	world->armies[1].troops = 3500;
	world->armies[1].tile = Campaign::At(8, 4);
	world->tiles[Campaign::At(6, 4)].terrain = Terrain::Forest;
	known.Reset(*world);
	known.Observe(*world, 0);
	assert(!known.Find(1));
	world->armies[1].tile = Campaign::At(5, 4);
	known.Observe(*world, 0);
	assert(known.Find(1) && known.Find(1)->current); // Adjacent opponents are always observable.
	const int distant = Campaign::At(20, 4);
	world->tiles[distant].owner = 1;
	known.Observe(*world, 0);
	assert(known.owners[distant] == -1); // Hidden territorial changes also remain hidden.

	auto storage = std::make_unique<StrategistBattle>();
	auto& battle = *storage;
	battle.Reset();
	battle.SetPlan(0);
	const auto route = battle.KnownRoute(4, battle.world.cities[1].tile);
	assert(!route.empty() && !battle.intelligence[0].Find(8));
	battle.world.armies[8].tile = Campaign::At(13, 12);
	battle.RefreshIntelligence();
	assert(!battle.intelligence[0].Find(8));
	assert(battle.KnownRoute(4, battle.world.cities[1].tile) == route);
	assert(battle.world.Route(battle.world.armies[4].tile, battle.world.cities[1].tile, Arm::Transport, 0) !=
	       route);
	// Neither a path preview nor a conditional departure uses the hidden blocker.
	assert(battle.Order(4, battle.world.cities[1].tile));
	assert(battle.world.armies[4].path == route);

	battle.Reset();
	battle.SetPlan(0);
	battle.world.day = 10;
	battle.RefreshIntelligence();
	assert(!battle.intelligence[1].Find(1));
	std::array<int, 4> targets;
	for (int i = 5; i < 9; ++i)
		targets[i - 5] = battle.EnemyTarget(i);
	battle.world.armies[1].tile = Campaign::At(3, 2);
	battle.world.armies[1].troops = 1700;
	battle.RefreshIntelligence();
	assert(!battle.intelligence[1].Find(1));
	for (int i = 5; i < 9; ++i)
		assert(battle.EnemyTarget(i) == targets[i - 5]);

	// A real unseen defeat is present in the global simulation log, but must
	// not enter the player's reports or reveal the defeated enemy's identity.
	battle.Reset();
	battle.SetPlan(0);
	battle.world.armies[8].tile = Campaign::At(3, 2);
	battle.world.armies[8].troops = 1;
	battle.world.armies[8].food = 0;
	battle.AdvanceDay();
	assert(battle.world.armies[8].troops == 0);
	assert(std::any_of(battle.world.chronicle.begin(), battle.world.chronicle.end(),
	                   [](const Chronicle& c) { return c.text.find(U"張遼") != std::u32string::npos; }));
	assert(std::none_of(battle.reports.begin(), battle.reports.end(),
	                    [](const Chronicle& c) { return c.text.find(U"張遼") != std::u32string::npos; }));

	// Date triggers run at the specified daily boundary, inside the 10-day phase.
	battle.Reset();
	battle.SetPlan(0);
	RemoveEnemies(battle);
	auto dated = Task(Campaign::At(6, 11));
	dated.trigger = OperationTrigger::Date;
	dated.startDay = 3;
	assert(battle.QueueOperation(2, dated));
	battle.AdvanceDay();
	battle.AdvanceDay();
	assert(battle.operations[2].state == OperationState::Waiting &&
	       battle.world.armies[2].tile == Campaign::At(5, 11));
	battle.AdvanceDay();
	assert(battle.world.armies[2].tile == Campaign::At(6, 11) &&
	       battle.operations[2].state == OperationState::Holding);
	assert(!battle.QueueOperation(2, dated)); // Progress-phase editing is blocked.

	// Arrival is recorded after physical movement, and releases the dependent
	// unit on the next day; it cannot cascade inside the same snapshot.
	battle.Reset();
	battle.SetPlan(0);
	RemoveEnemies(battle);
	battle.SetStance(0, battle::Stance::Balanced);
	battle.Order(0, Campaign::At(9, 12));
	auto linked = Task(Campaign::At(6, 11));
	linked.trigger = OperationTrigger::AllyArrival;
	linked.signalArmy = 0;
	linked.signalTile = Campaign::At(9, 12);
	assert(battle.QueueOperation(2, linked));
	battle.AdvanceDay();
	assert(battle.arrivals[0].day == 1 && battle.operations[2].state == OperationState::Waiting);
	assert(battle.world.armies[2].tile == Campaign::At(5, 11));
	battle.AdvanceDay();
	assert(battle.world.armies[2].tile == Campaign::At(6, 11));

	// A hidden real enemy does not release an ambush. An observed enemy does.
	battle.Reset();
	battle.SetPlan(0);
	for (int i = 5; i < 8; ++i)
		battle.world.armies[i].troops = 0;
	battle.world.armies[8].tile = Campaign::At(13, 12);
	auto ambush = Task(Campaign::At(6, 11));
	ambush.trigger = OperationTrigger::EnemySighted;
	ambush.watchTile = Campaign::At(13, 12);
	assert(battle.QueueOperation(2, ambush));
	battle.AdvanceDay();
	assert(battle.operations[2].state == OperationState::Waiting);
	battle.world.armies[0].tile = Campaign::At(12, 12);
	battle.world.armies[0].path.clear();
	battle.AdvanceDay();
	assert(battle.operations[2].state != OperationState::Waiting);

	// Stop conditions beat a ready start condition; they apply while holding.
	battle.Reset();
	battle.SetPlan(0);
	RemoveEnemies(battle);
	auto stop = Task(Campaign::At(6, 11));
	stop.minTroops = 3000;
	stop.fallback = battle.world.cities[0].tile;
	assert(battle.QueueOperation(2, stop));
	battle.AdvanceDay();
	assert(battle.operations[2].state == OperationState::Aborted && battle.world.armies[2].retreat);
	assert(battle.world.armies[2].target == stop.fallback);
	battle.Reset();
	battle.SetPlan(0);
	RemoveEnemies(battle);
	stop.target = battle.world.armies[2].tile;
	stop.minTroops = 1500;
	assert(battle.QueueOperation(2, stop));
	battle.AdvanceDay();
	assert(battle.operations[2].state == OperationState::Holding);
	battle.world.armies[2].troops = 1400;
	battle.AdvanceDay();
	assert(battle.operations[2].state == OperationState::Aborted);

	battle.Reset();
	battle.SetPlan(0);
	RemoveEnemies(battle);
	dated.startDay = 20;
	dated.deadline = 1;
	dated.fallback = battle.world.cities[0].tile;
	assert(battle.QueueOperation(2, dated));
	battle.AdvanceDay();
	battle.AdvanceDay();
	assert(battle.operations[2].state == OperationState::Aborted);
	battle.Reset();
	battle.SetPlan(0);
	linked.fallback = battle.world.cities[0].tile;
	assert(battle.QueueOperation(2, linked));
	battle.world.armies[0].troops = 0;
	battle.AdvanceDay();
	assert(battle.operations[2].state == OperationState::Aborted);

	// Reject cyclic signals without replacing an already-issued order.
	battle.Reset();
	battle.SetPlan(0);
	linked.signalArmy = 1;
	linked.signalTile = battle.world.armies[1].target;
	assert(battle.QueueOperation(0, linked));
	linked.signalArmy = 0;
	assert(!battle.QueueOperation(1, linked));
	assert(battle.operations[1].state == OperationState::None);

	// Scouting pays a real combat and movement cost, and cannot be assigned
	// to a transport or changed into assault while active.
	battle.Reset();
	battle.SetPlan(0);
	RemoveEnemies(battle);
	auto scout = Task(Campaign::At(5, 11));
	scout.scout = true;
	assert(!battle.QueueOperation(4, scout));
	assert(battle.QueueOperation(2, scout));
	battle.AdvanceDay();
	assert(battle.world.armies[2].scouting && battle.world.armies[2].stance == battle::Stance::Guard);
	battle.world.day = 10;
	assert(!battle.SetStance(2, battle::Stance::Assault));
	assert(battle.Order(2, battle.world.armies[2].tile) && !battle.world.armies[2].scouting);

	std::cout << "Intelligence, hidden state isolation, date/arrival/sighting triggers, stop priority and "
	             "scouting passed\n";
}
