#include "JourneyTestFlow.hpp"
#include "../NewGame/JourneyEpisode.hpp"
#include "../NewGame/StrategistJourney.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>

using namespace frontline;
static bool AttackVisible(StrategistJourney& j, int army)
{
	const auto contacts = j.intelligence[0].contacts;
	for (const auto& contact : contacts)
		if (contact.current && j.world.CanStrike(army, contact.observed.tile) && j.Strike(army, contact.id))
			return true;
	return false;
}
static void Approach(StrategistJourney& j, int army, int target, bool avoidRoad)
{
	if (target < 0 || j.chain.moved[army])
		return;
	const auto& unit = j.world.armies[army];
	int destination = -1, best = j.world.MapDistance(unit.tile, target);
	for (int tile : j.world.MapNeighbors(unit.tile))
		if (StrategistJourney::OnMap(tile) && j.intelligence[0].visible[tile] &&
		    TacticChain::Free(j.world, tile) && (!avoidRoad || tile / Width != 11) &&
		    j.world.MapDistance(tile, target) < best)
		{
			best = j.world.MapDistance(tile, target);
			destination = tile;
		}
	if (destination >= 0)
		j.Move(army, destination);
}
static void Fight(StrategistJourney& j, int army, bool avoidRoad)
{
	if (j.stage != JourneyStage::Battle || !TacticChain::Living(j.world, army))
		return;
	if (AttackVisible(j, army))
		return;
	int target = -1, nearest = 100;
	for (const auto& contact : j.intelligence[0].contacts)
		if (contact.current &&
		    j.world.MapDistance(j.world.armies[army].tile, contact.observed.tile) < nearest)
		{
			nearest = j.world.MapDistance(j.world.armies[army].tile, contact.observed.tile);
			target = contact.observed.tile;
		}
	Approach(j, army, target, avoidRoad);
	AttackVisible(j, army);
}
int main()
{
#ifdef _MSC_VER
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
	std::cout << std::unitbuf;
	auto j = std::make_unique<StrategistJourney>();
	j->Reset();
	j->Meet();
	assert(j->Prepare(0, 0));
	assert(j->Execute({{0, 3, ChainTactic::Lure, Campaign::At(8, 11)},
	                   {1, 3, ChainTactic::Ambush, -1},
	                   {2, 3, ChainTactic::Breakthrough, -1}})
	           .completed == 3);
	j->EndTurn();
	j->Execute({{2, 3, ChainTactic::Breakthrough, -1}});
	assert(j->won && ContinueJourneyWithPledge(*j) && j->mission == 1);
	assert(j->Prepare(0, 1));
	for (int phase = 1; phase <= j->Definition().limit && j->stage == JourneyStage::Battle; ++phase)
	{
		if (phase == 1)
		{
			assert(j->Move(0, Campaign::At(9, 9)));
			assert(j->Move(2, Campaign::At(9, 10)));
		}
		if (phase == 2)
		{
			assert(j->Move(0, Campaign::At(10, 9)));
			assert(j->Move(1, Campaign::At(9, 9)));
		}
		for (int army = 0; army < 3 && j->stage == JourneyStage::Battle; ++army)
			Fight(*j, army, true);
		if (j->stage == JourneyStage::Battle)
			j->EndTurn();
		std::cout << "escort phase " << phase << " cart " << j->convoyStep << " enemies "
		          << j->world.armies[3].troops << "," << j->world.armies[4].troops << "\n";
	}
	assert(j->won && j->stage == JourneyStage::Debrief && j->recoveredCargo >= 1200);
	assert(ContinueJourneyWithPledge(*j) && j->mission == 2 && j->Prepare(0, 0));
	for (int phase = 1; phase <= j->Definition().limit && j->stage == JourneyStage::Battle; ++phase)
	{
		for (int army = 0; army < 3 && j->stage == JourneyStage::Battle; ++army)
		{
			const int zone = army == 0 ? j->goal : j->Definition().secondGoal;
			bool opponent = false;
			for (const auto& contact : j->intelligence[0].contacts)
				if (contact.current && j->world.MapDistance(contact.observed.tile, zone) <= 2)
					opponent = true;
			if (army == 1 || opponent)
				Fight(*j, army, false);
			else
				Approach(*j, army, zone, false);
		}
		if (j->stage == JourneyStage::Battle)
			j->EndTurn();
		std::cout << "defense phase " << phase << " hold " << j->heldTurns << " enemies "
		          << j->world.armies[3].troops << "," << j->world.armies[4].troops << "\n";
	}
	assert(j->won && j->stage == JourneyStage::Debrief && j->trust == 30);
	assert(ContinueJourneyWithPledge(*j) && j->stage == JourneyStage::Complete);
    const auto ending=BuildJourneyEpisode(*j);
    assert(ending.title.find(U"終幕")!=std::u32string::npos && ending.lines.back().speaker==-1);
    std::u32string endingText;for(const auto& l:ending.lines) endingText+=l.text;
    assert(endingText.find(U"道を開いたあの日")!=std::u32string::npos);
    assert(endingText.find(U"あの時、車を待った軍師")==std::u32string::npos); // This run chose the road objective.
    auto single=std::make_unique<StrategistJourney>(*j);single->history[0]={};single->history[1]={};
    const auto singleEnding=BuildJourneyEpisode(*single);std::u32string singleText;
    for(const auto& l:singleEnding.lines) singleText+=l.text;
    assert(singleText.find(U"三度とも")==std::u32string::npos && singleText.find(U"三つの采配")==std::u32string::npos);
	j->Reset();
	j->Meet();
	j->mission = 2;
	assert(j->Prepare(0, 0));
	assert(j->Move(0, j->goal));
	for (int phase = 0; phase < 3; ++phase)
	{
		assert(j->Hold(0) && j->Hold(2));
		assert(j->EndTurn());
	}
	assert(j->won && j->world.armies[3].troops > 0 && j->world.armies[4].troops > 0);
	std::cout << "Three distinct missions completed using public player commands only\n";
}
