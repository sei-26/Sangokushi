#include "JourneyTestFlow.hpp"
#include "../NewGame/StrategistJourney.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <cstdlib>

using namespace frontline;
static std::vector<ChainOrder> Standard()
{
	return {{0, 3, ChainTactic::Lure, Campaign::At(8, 11)},
	        {1, 3, ChainTactic::Ambush, -1},
	        {2, 3, ChainTactic::Breakthrough, -1}};
}
static std::unique_ptr<StrategistJourney> Battle(int preparation = 0, int policy = 0)
{
	auto journey = std::make_unique<StrategistJourney>();
	journey->Reset();
	journey->Meet();
	assert(journey->Prepare(preparation, policy));
	return journey;
}
int main()
{
#ifdef _MSC_VER
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
	{
		auto j = Battle();
		const int before = j->world.armies[3].troops;
		const auto preview = j->Preview(Standard());
		assert(preview.completed == 3 && preview.prediction);
		assert(j->world.armies[3].troops == before && !j->chain.spent[0]);
		const auto actual = j->Execute(Standard());
		assert(actual.completed == 3 && !actual.prediction);
		assert(j->world.armies[0].tile == Campaign::At(8, 11));
		assert(j->world.armies[2].tile == Campaign::At(9, 10));
		assert(j->world.armies[3].tile == Campaign::At(10, 10));
		assert(j->chain.disordered[3] && !j->chain.concealed[1]);
		assert(j->world.armies[0].troops == 1340); // Bait takes a real casualty cost.
		const int after = j->world.armies[3].troops;
		assert(after < before && after > 0);
		assert(j->Execute(Standard()).completed == 0 && j->world.armies[3].troops == after);
		assert(actual.events[1].damage == preview.events[1].damage);
	}
	{
		auto j = Battle(0, 1);
		const auto before = j->world.armies;
		assert(j->Preview(Standard()).completed == 0);
		const auto result = j->Execute(Standard());
		assert(result.completed == 0 && result.events.size() == 1);
		assert(result.events[0].reason.find(U"固守") != std::u32string::npos);
		for (int i = 0; i < 5; ++i)
		{
			assert(j->world.armies[i].tile == before[i].tile);
			assert(j->world.armies[i].troops == before[i].troops);
		}
		assert(!j->chain.spent[0] && !j->chain.spent[1] && !j->chain.spent[2]);
		assert(j->Strike(0, 3)); // Countered combo leaves another valid decision.
	}
	{
		auto j = Battle(1);
		assert(!j->chain.concealed[1]);
		const auto result = j->Execute(Standard());
		assert(result.completed == 3 && result.events.size() == 3);
		assert(j->chain.spent[0] && j->chain.spent[1] && j->chain.spent[2]);
		assert(j->world.armies[3].tile == Campaign::At(9, 10)); // Unprepared ambush creates pressure; healthy target is not pushed.
		const auto again = j->Execute({{2, 3, ChainTactic::Breakthrough, -1}});
		assert(again.completed == 0); // The same actor cannot act twice in one command interval.
	}
	{
		auto j = Battle();
		assert(j->Move(1, Campaign::At(7, 9)) == false); // Pre-armed unit sacrificed movement.
		assert(j->EndTurn());
		assert(j->Move(1, Campaign::At(7, 9)) && !j->chain.concealed[1]);
		const auto reordered = j->Preview({{2, 3, ChainTactic::Breakthrough, -1}});
		assert(reordered.completed == 0); // Still outside the target's adjacency range.
	}
	{
		auto j = Battle();
		Army hidden;
		hidden.faction = 1;
		hidden.general = 4;
		hidden.troops = 700;
		hidden.tile = hidden.target = Campaign::At(10, 10);
		j->world.armies.push_back(hidden);
		j->chain.concealed.push_back(false);
		j->chain.moved.push_back(false);
		j->chain.spent.push_back(false);
		j->chain.disordered.push_back(false);
		const auto before = j->Preview(Standard());
		j->world.armies[5].troops = 9000;
		const auto changed = j->Preview(Standard());
		assert(before.completed == 3 && changed.completed == before.completed);
		assert(before.events[2].damage == changed.events[2].damage);
		const auto hiddenTarget = j->Execute({{0, 5, ChainTactic::Lure, Campaign::At(8, 11)}});
		assert(hiddenTarget.completed == 0 &&
		       hiddenTarget.events[0].reason.find(U"確認中") != std::u32string::npos);
		const auto actual = j->Execute(Standard());
		assert(actual.completed == 2 && !actual.events.back().success);
		assert(!j->chain.spent[2]); // Real obstruction stops the push, without leaking into prediction.
	}
	{
		auto j = Battle();
		j->Execute(Standard());
		assert(j->EndTurn());
		if (j->stage == JourneyStage::Battle) assert(j->Execute({{2, 3, ChainTactic::Breakthrough, -1}}).completed == 1);
		assert(j->stage == JourneyStage::Debrief && j->won);
		assert(j->casualties > 0 && j->survivors == j->deployed - j->casualties);
		assert(j->world.cities[0].troops == 6000 - j->casualties);
		const int soldiers = j->world.cities[0].troops, food = j->world.cities[0].food,
		          gold = j->world.cities[0].gold;
		assert(j->trust == 10 && j->world.Affinity(0, 1) == 43);
		assert(ContinueJourneyWithPledge(*j) && j->mission == 1 && j->stage == JourneyStage::Preparation);
		assert(j->world.cities[0].troops == soldiers && j->world.cities[0].food == food &&
		       j->world.cities[0].gold == gold);
		assert(j->Prepare(0, 1));
		assert(j->world.cities[0].gold == gold - 200 && j->trust == 10);
		assert(j->Preview(Standard()).completed == 0); // Next opponent changes the useful plan.
		assert(j->Definition().kind == JourneyObjective::Escort && j->world.armies.size() == 6);
		assert(j->world.armies[StrategistJourney::ConvoyArmy].cargoFood == 2000);
	}
	{
		auto j = Battle(0, 2);
		assert(!j->chain.concealed[1] && j->Preview(Standard()).completed == 3);
	}
	{
		auto j = Battle();
		j->world.armies[4].tile = j->world.armies[4].target = Campaign::At(10, 9);
		j->world.armies[4].scouting = true;
		j->world.tiles[Campaign::At(9, 9)].terrain = Terrain::Plain;
		// Scout reaches detection range with an unobstructed ray.
		assert(j->EndTurn());
		assert(!j->chain.concealed[1]);
	}
	{
		auto j = Battle();
		j->world.cities[0].food = 0;
		for (int i = 0; i < 3; ++i)
			j->world.armies[i].troops = 1;
		assert(j->EndTurn() && j->stage == JourneyStage::Debrief && !j->won);
		const int troops = j->world.cities[0].troops;
		assert(!j->EndTurn() && j->world.cities[0].troops == troops); // Results settle exactly once.
	}
	std::cout << "Chain causality, counters, concealment, hidden preview, resource continuity passed\n";
}
