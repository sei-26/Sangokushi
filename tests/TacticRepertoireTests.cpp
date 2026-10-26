#include "../NewGame/StrategistJourney.hpp"
#include <cassert>
#include <iostream>
#include <memory>

using namespace frontline;
static std::unique_ptr<StrategistJourney> Formation()
{
	auto j = std::make_unique<StrategistJourney>();
	j->Reset(); j->Meet(); assert(j->Prepare(1, 1));
	const int positions[] = {Campaign::At(9, 9), Campaign::At(10, 8), Campaign::At(8, 9), Campaign::At(9, 8)};
	for (int i = 0; i < 4; ++i) j->world.armies[i].tile = j->world.armies[i].target = positions[i];
	j->RefreshIntelligence(); return j;
}
int main()
{
	{
		auto j = Formation();
		for (int i = 0; i < 3; ++i) assert(TacticChain::Loadout(j->world, i).size() == 4);
		assert(TacticChain::CanUse(j->world, 0, ChainTactic::Oath));
		assert(!TacticChain::CanUse(j->world, 1, ChainTactic::Oath));
		std::swap(j->world.armies[0].general, j->world.armies[1].general);
		assert(!TacticChain::CanUse(j->world, 0, ChainTactic::Oath));
		assert(TacticChain::CanUse(j->world, 0, ChainTactic::AzureDragon));
	}
	{
		auto j = Formation();
		const auto fail = j->Execute({{1, 3, ChainTactic::Disrupt, -1}});
		assert(fail.completed == 0 && !j->chain.spent[1] && !j->chain.disordered[3]);
		const std::vector<ChainOrder> orders{{0,3,ChainTactic::Feint,-1},{1,3,ChainTactic::Disrupt,-1},{2,3,ChainTactic::Breakthrough,-1}};
		const int before = j->world.armies[3].troops;
		const auto predicted = j->Preview(orders);
		assert(predicted.completed == 3 && j->world.armies[3].troops == before && !j->chain.pressured[3]);
		const auto actual = j->Execute(orders);
		assert(actual.completed == 3 && j->world.armies[3].stance == battle::Stance::Balanced);
		assert(j->world.armies[3].tile == Campaign::At(9,7) && j->chain.disordered[3]);
		assert(actual.events[2].damage == predicted.events[2].damage);
		assert(actual.events[0].changes.back().pressured && !actual.events[0].changes.back().disordered);
	}
	{
		auto j = Formation();
		const std::vector<ChainOrder> orders{{0,-1,ChainTactic::Oath,-1},{2,3,ChainTactic::SerpentRoar,-1},{1,3,ChainTactic::AzureDragon,-1}};
		const auto preview = j->Preview(orders);
		assert(preview.completed == 3 && !j->chain.signatureUsed[0] && !j->chain.inspired[1]);
		const auto actual = j->Execute(orders);
		assert(actual.completed == 3 && actual.events[0].enemy == -1 && actual.events[0].changes.size() == 3);
		assert(actual.events[1].damage == (140 + 92)*120/100);
		assert(actual.events[2].damage == (350 + 270)*120/100);
		for (int i = 0; i < 3; ++i) assert(j->chain.signatureUsed[i]);
		assert(!j->chain.inspired[1] && !j->chain.inspired[2]);
		j->chain.BeginTurn(j->world);
		assert(j->chain.signatureUsed[0] && !j->chain.inspired[1] && !j->chain.pressured[3]);
		assert(j->chain.Execute(j->world,j->intelligence[0],{{0,-1,ChainTactic::Oath,-1}}).completed == 0);
		assert(!j->chain.spent[0]);
	}
	{
		auto j = Formation();
		j->world.armies[0].morale = 20;
		assert(j->Execute({{0,-1,ChainTactic::Oath,-1}}).completed == 0);
		assert(!j->chain.signatureUsed[0] && j->world.armies[1].morale == 100);
		j->world.armies[0].morale = 90;
		assert(j->Execute({{0,-1,ChainTactic::Oath,-1}}).completed == 1);
		assert(j->chain.inspired[1]);
		const int before = j->world.armies[3].troops;
		std::u32string reason;
		assert(j->chain.Strike(j->world,j->intelligence[0],1,3,reason));
		assert(before-j->world.armies[3].troops == (100+1400/12)*120/100 && !j->chain.inspired[1]);
		j->chain.BeginTurn(j->world); assert(!j->chain.inspired[2]);
	}
	{
		auto j = Formation();
		assert(j->Execute({{1,3,ChainTactic::AzureDragon,-1}}).completed == 0 && !j->chain.signatureUsed[1]);
		assert(j->Execute({{0,3,ChainTactic::Feint,-1},{1,3,ChainTactic::AzureDragon,-1}}).completed == 2);
		assert(j->world.armies[3].tile == Campaign::At(9,8)); // Big strike needs no push destination.
	}
	{
		auto j = Formation();
		Army hidden; hidden.faction=1; hidden.general=4; hidden.troops=500; hidden.tile=Campaign::At(13,2);
		j->world.armies.push_back(hidden);
		const auto result=j->Execute({{0,-1,ChainTactic::Oath,-1},{2,5,ChainTactic::SerpentRoar,-1}});
		assert(result.completed==0 && !j->chain.spent[0] && !j->chain.signatureUsed[0]);
	}
	{
		auto j = Formation();
		j->world.armies[1].morale = 20;
		const std::vector<ChainOrder> orders{{0,3,ChainTactic::Feint,-1},{1,3,ChainTactic::AzureDragon,-1}};
		assert(j->Preview(orders).completed == 1);
		assert(j->Execute(orders).completed == 1 && j->chain.pressured[3]);
		assert(!j->chain.signatureUsed[1] && !j->chain.spent[1]);
		assert(j->Execute({{1,3,ChainTactic::Disrupt,-1}}).completed == 1); // Switch the unused officer to a cheaper tactic.
		assert(j->world.armies[1].morale == 10 && !j->chain.signatureUsed[1]);
	}
	{
		auto j = Formation();
		j->world.armies[1].troops = j->world.armies[2].troops = 0;
		assert(j->Execute({{0,-1,ChainTactic::Oath,-1}}).completed == 0);
		assert(j->world.armies[0].morale == 100 && !j->chain.signatureUsed[0] && !j->chain.spent[0]);
		j->chain.signatureUsed[0] = true; j->chain.pressured[3] = true;
		j->chain.BeginTurn(j->world);
		assert(j->chain.signatureUsed[0] && !j->chain.pressured[3]);
		j->chain.Reset(j->world); assert(!j->chain.signatureUsed[0]);
	}
	std::cout << "Repertoire ownership, alternative chains, guard counter, unique costs, support, prediction and once-per-battle passed\n";
}
