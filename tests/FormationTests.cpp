#include "../NewGame/FormationStatus.hpp"
#include <cassert>
#include <iostream>
using namespace frontline;
Campaign Field()
{
	Campaign g;
	g.Reset(0);
	g.generals = {{U"関羽", 0, 0, 90}, {U"趙雲", 0, 0, 90}, {U"黄忠", 0, 0, 85},
	              {U"徐晃", 0, 0, 80}, {U"劉備", 0, 0, 80}, {U"糜竺", 0, 0, 70}};
	g.armies.clear();
	for (int i = 0; i < 6; ++i)
	{
		Army a;
		a.general = i;
		a.faction = 0;
		a.tile = a.target = Campaign::At(49, 34);
		a.troops = 3000;
		g.armies.push_back(a);
		if (i)
			g.ChangeBond(0, i, 60);
	}
	return g;
}
int main()
{
	auto g = Field();
	const auto total = g.Formation(0);
	assert(total.attack == 15 && total.defense == 15 && total.supply == 15 && total.morale == 6);
	const auto members = formationstatus::Members(g, 0);
	assert(members.size() == 5);
	officer::Link sum;
	for (const auto& m : members)
		officer::Merge(sum, m.actual);
	assert(sum.attack == total.attack && sum.defense == total.defense && sum.morale == total.morale &&
	       sum.supply == total.supply);
	auto member = [&](int index) {
		for (const auto& m : formationstatus::Members(g, 0))
			if (m.army == index)
				return m;
		return formationstatus::Member{};
	};
	assert(member(5).state == formationstatus::State::CivilRole && !formationstatus::Any(member(5).actual));
	g.armies[1].tile = Campaign::At(1, 1);
	assert(member(1).state == formationstatus::State::OutOfRange && member(1).potential.defense == 15 &&
	       member(1).actual.defense == 0 && g.Formation(0).defense == 0);
	g.armies[1].tile = g.armies[0].tile;
	g.armies[1].retreat = true;
	assert(member(1).state == formationstatus::State::Retreat);
	g.armies[1].retreat = false;
	g.armies[1].morale = 29;
	assert(member(1).state == formationstatus::State::LowMorale && g.FormationLink(0, 1).defense == 0);
	g.armies[1].morale = 30;
	g.armies[1].supplied = false;
	g.armies[1].food = 0;
	assert(member(1).state == formationstatus::State::Starving);
	g.armies[1].food = 1;
	assert(member(1).state == formationstatus::State::Active && g.FormationLink(0, 1).defense == 15);
	g.armies[1].arm = Arm::Transport;
	assert(member(1).state == formationstatus::State::Transport);
	g.armies[1].arm = Arm::Spear;
	g.ChangeBond(0, 1, -100);
	assert(member(1).state == formationstatus::State::LowAffinity);
	g.armies[0].retreat = true;
	for (const auto& m : formationstatus::Members(g, 0))
		assert(m.state == formationstatus::State::ReceiverUnavailable && !formationstatus::Any(m.actual));
	g = Field();
	g.generals[0].name = U"黄忠";
	assert(g.FormationLink(0, 2).attack == 8);
	g.armies[1].faction = 1;
	assert(!formationstatus::Any(g.FormationLink(0, 1)) && formationstatus::Members(g, 0).size() == 4);
	assert(!formationstatus::Any(g.FormationLink(-1, 0)) && !formationstatus::Any(g.FormationLink(0, 0)) &&
	       !formationstatus::Any(g.FormationLink(0, 72)));
	const int day = g.day, commands = g.commands, bond = g.Affinity(0, 2);
	formationstatus::Members(g, 0);
	assert(day == g.day && commands == g.commands && bond == g.Affinity(0, 2));
	std::cout << "Formation contributions match combat totals; role caps, range, morale, food, retreat, "
	             "transport and bond reasons passed\n";
}
