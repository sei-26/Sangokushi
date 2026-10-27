#include "JourneyTestFlow.hpp"
#include "../NewGame/StrategistJourney.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>

using namespace frontline;
static std::unique_ptr<StrategistJourney> CreateMission(int mission, int policy = 0)
{
	auto j = std::make_unique<StrategistJourney>();
	j->Reset();
	j->Meet();
	j->mission = mission;
	assert(j->Prepare(0, policy));
	return j;
}
static void RemoveOpponents(StrategistJourney& j)
{
	j.world.armies[3].troops = j.world.armies[4].troops = 0;
	j.RefreshIntelligence();
}
int main()
{
#ifdef _MSC_VER
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
	{
		auto j = std::make_unique<StrategistJourney>();
		j->Reset(); j->Meet();
		assert(j->SelectPromise(JourneyPromise::EscortRefugees));
		assert(j->PreparationCosts(0).TotalFood() == 3900 && j->TurnBudget() == 10);
		assert(j->Prepare(0, 0));
		assert(j->world.armies.size() == 6 && j->world.armies[5].troops == 600);
		assert(j->world.armies[5].cargoFood == 0 && j->world.cities[0].food == 14100);
		assert(!j->SelectPromise(JourneyPromise::OpenRoad));
		RemoveOpponents(*j);
		assert(j->Move(0, Campaign::At(8, 11)));
		for (int turn = 0; turn < 3; ++turn) assert(j->EndTurn());
		assert(j->stage == JourneyStage::Battle && j->convoyStep == 1);
		assert(j->Move(0, Campaign::At(8, 12)));
		for (int turn = 0; turn < 5; ++turn) assert(j->EndTurn());
		assert(j->won && j->history[0].rescued && j->survivors == 4800);
		assert(j->world.cities[0].troops == 6000 && j->recoveredCargo == 0);
		assert(ContinueJourneyWithPledge(*j) && j->Prepare(0, 1));
		assert(j->world.generals[5].name == U"季安の糧車");
	}
	{
		auto j = std::make_unique<StrategistJourney>();
		j->Reset(); j->Meet(); assert(j->SelectPromise(JourneyPromise::EscortRefugees));
		assert(j->Prepare(0, 0));
		j->world.armies[5].troops = 0;
		RemoveOpponents(*j); // Isolate the lost escort from ordinary frontline losses.
		assert(j->EndTurn() && !j->won && !j->history[0].rescued);
		assert(j->world.cities[0].troops == 5400 && j->casualties == 600);
	}
	{
		auto j = std::make_unique<StrategistJourney>();
		j->Reset(); j->Meet(); assert(j->SelectPromise(JourneyPromise::EscortRefugees));
		assert(j->Prepare(0, 0) && j->Withdraw());
		assert(j->withdrawn && !j->won && j->trust == 0 && j->history[0].withdrawn && !j->history[0].rescued);
		assert(j->world.cities[0].troops == 6000 && j->world.cities[0].food == 18000 && j->world.cities[0].gold == 700);
		assert(!j->Withdraw() && j->world.cities[0].food == 18000 && !ContinueJourneyWithPledge(*j));
	}
	// The council's advertised bill must match the real deployment, including
	// the escort's cargo. Preview and duplicate deployment must not spend twice.
	for (int mission = 0; mission < 3; ++mission)
		for (int choice = 0; choice < 2; ++choice)
		{
			auto j = std::make_unique<StrategistJourney>();
			j->Reset(); j->Meet(); j->mission = mission;
			const auto before = j->world.cities[0];
			const auto bill = j->PreparationCosts(choice);
			assert(j->CanPrepare(choice) && j->world.cities[0].food == before.food);
			assert(j->Prepare(choice, 0));
			assert(j->world.cities[0].food == before.food - bill.TotalFood());
			assert(j->world.cities[0].gold == before.gold - bill.gold);
			assert(j->world.cities[0].troops == before.troops - j->deployed - bill.escort);
			assert(!j->CanPrepare(choice) && !j->Prepare(choice, 0));
			assert(j->world.cities[0].food == before.food - bill.TotalFood());
		}
	{
		auto j = std::make_unique<StrategistJourney>();
		j->Reset(); j->Meet(); j->mission = 1;
		j->world.cities[0].food = j->PreparationCosts(0).TotalFood() - 1;
		const int food = j->world.cities[0].food;
		assert(!j->CanPrepare(0) && !j->Prepare(0, 0));
		assert(j->world.cities[0].food == food && j->world.armies.empty() && j->convoyEscort == 0);
	}
	{
		auto j = CreateMission(1);
		assert(j->world.cities[0].troops == 1200);
		assert(j->world.cities[0].food == 12100); // 3,000 + cart provisions 900 + actual cargo 2,000.
		RemoveOpponents(*j);
		for (int i = 0; i < 2; ++i)
			assert(j->EndTurn());
		assert(j->stage == JourneyStage::Battle &&
		       j->convoyStep == 2); // Killing the leader cannot win escort.
		for (int i = 0; i < 4; ++i)
			assert(j->EndTurn());
		assert(j->won && j->stage == JourneyStage::Debrief && j->convoyStep == 6);
		assert(j->recoveredCargo == 2000 && j->survivors == 4800);
		assert(j->world.cities[0].troops == 6000);
		const int food = j->world.cities[0].food;
		assert(!j->EndTurn() && j->world.cities[0].food == food);
		assert(ContinueJourneyWithPledge(*j) && j->mission == 2 && j->stage == JourneyStage::Preparation);
	}
	{
		auto j = CreateMission(1);
		RemoveOpponents(*j);
		assert(j->Move(0, Campaign::At(8, 11)));
		for (int i = 0; i < 3; ++i)
			assert(j->EndTurn());
		assert(j->convoyStep == 1); // A friendly bait unit blocks the road too.
		assert(j->Move(0, Campaign::At(8, 12)));
		assert(j->EndTurn() && j->convoyStep == 2);
	}
	{
		auto j = CreateMission(1);
		j->world.armies[StrategistJourney::ConvoyArmy].cargoFood = 1199;
		assert(j->EndTurn() && !j->won && j->stage == JourneyStage::Debrief);
		assert(j->recoveredCargo == 1199); // Return only what remained, even on retreat.
	}
	{
		auto j = CreateMission(2);
		RemoveOpponents(*j);
		assert(j->Move(0, j->goal));
		assert(j->heldTurns == 0); // Repeated input cannot manufacture elapsed defense time.
		assert(j->Hold(0) && j->chain.spent[0] && !j->Strike(0, 3));
		assert(j->EndTurn() && j->heldTurns == 1 && !j->guarding[0]);
		assert(j->Move(2, Campaign::At(9, 9)));
		assert(j->EndTurn() && j->heldTurns == 0); // A longer attack that abandons a point loses the hold.
		assert(j->Move(2, j->Definition().secondGoal));
		for (int i = 0; i < 3; ++i)
			assert(j->EndTurn());
		assert(j->won && j->heldTurns == 3 && ContinueJourneyWithPledge(*j) && j->stage == JourneyStage::Complete);
	}
	{
		auto j = CreateMission(2);
		RemoveOpponents(*j);
		for (int i = 0; i < j->Definition().limit; ++i)
			assert(j->EndTurn());
		assert(!j->won && j->stage == JourneyStage::Debrief); // Wiping all enemies is not this objective.
	}
	{
		auto j = CreateMission(0);
		assert(j->Hold(0));
		assert(j->Preview({{0, 3, ChainTactic::Lure, Campaign::At(8, 11)}}).completed == 0);
		assert(j->EndTurn());
		assert(j->world.armies[0].stance == battle::Stance::Balanced);
	}

	{
		auto j = CreateMission(2);
		RemoveOpponents(*j);
		assert(j->Move(0, j->goal) && j->Hold(0));
		j->world.armies[3].troops = j->world.armies[4].troops = 1000;
		j->world.armies[3].tile = Campaign::At(8, 6);
		j->world.armies[4].tile = Campaign::At(7, 7);
		assert(!j->PositionSecured(j->goal));
		j->world.armies[4].troops = 0;
		assert(j->PositionSecured(j->goal));
		j->world.armies[0].morale = 39;
		assert(!j->PositionSecured(j->goal));
	}
	std::cout << "Escort resources, genuine progress, defense interruption, stance tradeoffs passed\n";
}
