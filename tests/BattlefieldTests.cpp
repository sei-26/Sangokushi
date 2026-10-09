#include "../NewGame/Campaign.hpp"
#include <cstdlib>
#include <iostream>
#define CHECK(x) do { if (!(x)) { std::cerr << "Battlefield check " << __LINE__ << ": " << #x << std::endl; std::exit(1); } } while (false)
using namespace frontline;
Campaign Field()
{
	Campaign g; g.Reset(0); g.hexMap=false;
	for (auto& t : g.tiles) t = {Terrain::Plain, -1};
	g.cities = {{U"Base", Campaign::At(2,8), 0}, {U"Enemy", Campaign::At(20,20), 1}, {U"Third", Campaign::At(30,30), 2}};
	g.generals = {{U"A",0,0,80}, {U"B",1,1,80}, {U"C",1,1,80}, {U"D",0,0,80}, {U"E",2,2,80}};
	for (const auto& c : g.cities) g.tiles[c.tile].owner = c.owner;
	return g;
}
Army Unit(int f, int g, int x, int y)
{
	Army a; a.faction=f; a.general=g; a.tile=a.target=Campaign::At(x,y); a.troops=3000; return a;
}
int main()
{
	Campaign road=Field();
	for (int x=2;x<=9;++x) road.tiles[Campaign::At(x,8)].owner=0;
	CHECK(road.Supply(0)[Campaign::At(9,8)]==0);
	road.armies={Unit(1,1,5,7)};
	CHECK(road.SupplyBlockade(0)[Campaign::At(5,8)] && road.Supply(0)[Campaign::At(9,8)]==-1);
	CHECK(road.TransportDays(road.cities[0].tile,Campaign::At(9,8),0)==-1);
	// A viable escort secures its own tile, reconnecting a threatened narrow corridor.
	road.armies.push_back(Unit(0,0,5,8));
	CHECK(!road.SupplyBlockade(0)[Campaign::At(5,8)] && road.Supply(0)[Campaign::At(9,8)]==0);
	CHECK(road.TransportDays(road.cities[0].tile,Campaign::At(9,8),0)>0);
	road.armies[1].retreat=true; CHECK(road.Supply(0)[Campaign::At(9,8)]==-1);
	road.armies[1].retreat=false; road.armies[1].troops=1199; CHECK(road.Supply(0)[Campaign::At(9,8)]==-1);
	road.armies.resize(1); road.armies[0].retreat=true; CHECK(road.Supply(0)[Campaign::At(9,8)]==0);
	road.armies[0].retreat=false; road.armies[0].arm=Arm::Transport; CHECK(road.Supply(0)[Campaign::At(9,8)]==0);
	road.armies[0].arm=Arm::Spear; road.truceUntil[0][1]=road.truceUntil[1][0]=60; CHECK(road.Supply(0)[Campaign::At(9,8)]==0);
	road.truceUntil={}; road.armies[0].tile=Campaign::At(2,7);
	CHECK(!road.SupplyBlockade(0)[road.cities[0].tile]);
	road.cities[0].troops=999; CHECK(road.SupplyBlockade(0)[road.cities[0].tile]);
	// Occupation still blocks a tile even when an army is too weak to project a zone.
	road.armies[0].troops=1; road.armies[0].tile=Campaign::At(5,8);
	CHECK(road.SupplyBlockade(0)[Campaign::At(5,8)] && !road.SupplyBlockade(0)[Campaign::At(6,8)]);
	Campaign pincer=Field(); pincer.armies={Unit(0,0,8,8),Unit(1,1,7,8),Unit(1,2,9,8)};
	CHECK(pincer.PressureDirections(0)==2 && pincer.PressureDamagePercent(0)==15 && pincer.PressureMoraleLoss(0)==3);
	Campaign guarding=pincer; CHECK(guarding.SetStance(0,battle::Stance::Guard));
	CHECK(guarding.PressureDamagePercent(0)==7 && guarding.PressureMoraleLoss(0)==2);
	pincer.AdvanceDay(); guarding.AdvanceDay();
	CHECK(guarding.armies[0].troops>pincer.armies[0].troops && guarding.armies[0].morale>pincer.armies[0].morale);
	Campaign sameFront=Field(); sameFront.armies={Unit(0,0,8,8),Unit(1,1,7,8),Unit(1,2,7,8)};
	CHECK(sameFront.PressureDirections(0)==1 && sameFront.PressureMoraleLoss(0)==0);
	Campaign flanked=sameFront; flanked.armies[2].tile=Campaign::At(9,8); flanked.armies[2].target=flanked.armies[2].tile;
	sameFront.AdvanceDay(); flanked.AdvanceDay();
	CHECK(flanked.armies[0].troops<sameFront.armies[0].troops && flanked.armies[0].morale<sameFront.armies[0].morale);
	Campaign exclusions=Field(); exclusions.armies={Unit(0,0,8,8),Unit(1,1,7,8),Unit(1,2,9,8)};
	exclusions.armies[2].arm=Arm::Transport; CHECK(exclusions.PressureDirections(0)==1);
	exclusions.armies[2].arm=Arm::Spear; exclusions.armies[2].retreat=true; CHECK(exclusions.PressureDirections(0)==1);
	exclusions.armies[2].retreat=false; exclusions.armies[2].morale=29; CHECK(exclusions.PressureDirections(0)==1);
	exclusions.armies[2].morale=100; exclusions.armies[2].troops=1199; CHECK(exclusions.PressureDirections(0)==1);
	exclusions.truceUntil[0][1]=exclusions.truceUntil[1][0]=60; CHECK(exclusions.PressureDirections(0)==0);
	CHECK(exclusions.PressureDamagePercent(-1)==0 && exclusions.PressureMoraleLoss(1000)==0);
	// Supply loss uses carried food immediately and resumes city consumption after relief.
	Campaign relief=Field(); for(int x=2;x<=9;++x)relief.tiles[Campaign::At(x,8)].owner=0;
	relief.armies={Unit(0,0,9,8),Unit(1,1,5,7)};
	const int provisions=relief.armies[0].food; relief.AdvanceDay();
	CHECK(!relief.armies[0].supplied && relief.armies[0].food<provisions);
	relief.armies[1].retreat=true; const int carried=relief.armies[0].food; relief.AdvanceDay();
	CHECK(relief.armies[0].supplied && relief.armies[0].food==carried);
	std::cout << "Battlefield interdiction, escort relief, directional pressure, guard and ceasefire passed" << std::endl;
}
