#include "../NewGame/Campaign.hpp"
#include "../NewGame/PortraitCatalog.hpp"
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
 // Officer specialties change actual casualties, not just descriptions.
 Campaign cavalry=Field(); cavalry.armies={Unit(0,0,8,8),Unit(1,1,9,8)};
 cavalry.armies[0].arm=Arm::Cavalry;
 for(auto& a:cavalry.armies) a.tacticReadyDay=1000;
 Campaign horseman=cavalry; horseman.generals[0].trait=Trait::CavalryExpert;
 cavalry.AdvanceDay(); horseman.AdvanceDay(); CHECK(horseman.armies[1].troops<cavalry.armies[1].troops);
 horseman.tiles[horseman.armies[0].tile].terrain=Terrain::Forest; CHECK(horseman.OfficerAttackPercent(0)==100);
 Campaign bow=Field();bow.armies={Unit(0,0,8,8),Unit(1,1,10,8)};bow.armies[0].arm=Arm::Bow;
 for(auto& a:bow.armies) a.tacticReadyDay=1000;
 Campaign archer=bow;archer.generals[0].trait=Trait::ArcherExpert;
 bow.AdvanceDay();archer.AdvanceDay();CHECK(archer.armies[1].troops<bow.armies[1].troops);
 Campaign forest=Field();forest.armies={Unit(0,0,8,8),Unit(1,1,9,8)};forest.tiles[forest.armies[0].tile].terrain=Terrain::Forest;
 for(auto& a:forest.armies)a.tacticReadyDay=1000;
 Campaign terrain=forest;terrain.generals[0].trait=Trait::TerrainExpert;
 forest.AdvanceDay();terrain.AdvanceDay();CHECK(terrain.armies[0].troops>forest.armies[0].troops && terrain.armies[1].troops<forest.armies[1].troops);
 Campaign charge=Field();charge.armies={Unit(0,0,8,8),Unit(1,1,9,8)};charge.generals[0].tactic=Tactic::MountedCharge;
 CHECK(!charge.ActivateTactic(0));charge.armies[0].arm=Arm::Cavalry;charge.armies[1].tacticReadyDay=1000;
 Campaign uncharged=charge;CHECK(charge.ActivateTactic(0));charge.AdvanceDay();uncharged.AdvanceDay();
 CHECK(charge.armies[1].troops<uncharged.armies[1].troops && charge.armies[0].tacticLeft==5 && charge.armies[0].tacticReadyDay==31);
 charge.tiles[charge.armies[0].tile].terrain=Terrain::Forest;CHECK(charge.OfficerAttackPercent(0)==100);
 charge.armies[1].troops=0;for(int d=0;d<5;++d)charge.AdvanceDay();CHECK(charge.armies[0].tacticLeft==0);
 Campaign ambush=Field();ambush.armies={Unit(0,0,8,8),Unit(1,1,9,8)};ambush.generals[0].tactic=Tactic::Ambush;
 CHECK(!ambush.ActivateTactic(0));ambush.tiles[ambush.armies[0].tile].terrain=Terrain::Forest;
 ambush.armies[1].tacticReadyDay=1000;Campaign uncovered=ambush;
 CHECK(ambush.ActivateTactic(0));ambush.AdvanceDay();uncovered.AdvanceDay();
 CHECK(ambush.armies[0].troops>uncovered.armies[0].troops && ambush.armies[1].troops<uncovered.armies[1].troops);
 Campaign siege=Field();siege.armies={Unit(0,0,19,20)};siege.armies[0].target=siege.cities[1].tile;siege.generals[0].tactic=Tactic::SiegeStrike;
 CHECK(!siege.ActivateTactic(0));siege.armies[0].arm=Arm::Siege;Campaign regularSiege=siege;
 siege.generals[0].trait=Trait::SiegeExpert;CHECK(siege.ActivateTactic(0));siege.AdvanceDay();regularSiege.AdvanceDay();
 CHECK(siege.cities[1].troops<regularSiege.cities[1].troops && siege.OfficerAttackPercent(0,true)==185);
 Campaign steady=Field();steady.armies={Unit(0,0,8,8),Unit(1,1,7,8),Unit(1,2,9,8)};
 for(auto& a:steady.armies)a.tacticReadyDay=1000;
 Campaign resolute=steady;resolute.generals[0].trait=Trait::Resolute;
 steady.AdvanceDay();resolute.AdvanceDay();CHECK(resolute.armies[0].morale>steady.armies[0].morale);
 Campaign support=Field();support.armies={Unit(0,0,8,8),Unit(0,3,9,8)};support.armies[0].morale=50;
 Campaign inspired=support;inspired.generals[3].trait=Trait::Inspiring;
 support.AdvanceDay();inspired.AdvanceDay();CHECK(inspired.armies[0].morale==support.armies[0].morale+2);
 Campaign doubleAura=inspired;doubleAura.armies[0].morale=50;doubleAura.generals[0].trait=Trait::Inspiring;
 Campaign singleAura=doubleAura;singleAura.generals[0].trait=Trait::Administrator;
 doubleAura.AdvanceDay();singleAura.AdvanceDay();CHECK(doubleAura.armies[0].morale==singleAura.armies[0].morale);
 Campaign ai=Field();ai.armies={Unit(0,0,8,8),Unit(1,1,9,8)};ai.armies[0].tacticReadyDay=1000;ai.generals[1].tactic=Tactic::MountedCharge;
 ai.AdvanceDay();CHECK(ai.armies[1].tacticLeft==0);ai.armies[1].arm=Arm::Cavalry;ai.AdvanceDay();CHECK(ai.armies[1].tacticLeft==5);
 Campaign aiAmbush=Field();aiAmbush.armies={Unit(0,0,8,8),Unit(1,1,9,8)};aiAmbush.generals[1].tactic=Tactic::Ambush;aiAmbush.armies[0].tacticReadyDay=1000;
 aiAmbush.AdvanceDay();CHECK(aiAmbush.armies[1].tacticLeft==0);aiAmbush.tiles[aiAmbush.armies[1].tile].terrain=Terrain::Forest;aiAmbush.AdvanceDay();CHECK(aiAmbush.armies[1].tacticLeft==5);
 CHECK(charge.tacticEvents.empty());
 CHECK(ambush.tacticEvents.size()==1 && ambush.tacticEvents[0].tactic==Tactic::Ambush && ambush.tacticEvents[0].general==0);
 Campaign cancelled=Field();cancelled.armies={Unit(0,0,8,8),Unit(1,1,9,8)};cancelled.generals[0].tactic=Tactic::Fire;
 CHECK(cancelled.ActivateTactic(0));cancelled.armies[1].troops=0;cancelled.AdvanceDay();CHECK(cancelled.tacticEvents.empty() && cancelled.armies[0].tacticReadyDay==0);
 Campaign fireEvent=Field();fireEvent.armies={Unit(0,0,8,8),Unit(1,1,9,8)};fireEvent.generals[0].tactic=Tactic::Fire;fireEvent.armies[1].tacticReadyDay=1000;
 CHECK(fireEvent.ActivateTactic(0));fireEvent.AdvanceDay();CHECK(fireEvent.tacticEvents.size()==1 && fireEvent.tacticEvents[0].target==Campaign::At(9,8));
 fireEvent.AdvanceDay();CHECK(fireEvent.tacticEvents.empty());
 for(int i=0;i<72;++i){const auto r=campaignvisual::PortraitCrop(i,1024,1536);CHECK(r[0]>=0 && r[1]>=0 && r[2]>0 && r[3]>0 && r[0]+r[2]<=1024 && r[1]+r[3]<=1536);}
 CHECK(campaignvisual::PortraitCrop(8,1024,1536)[1]==477);
 Campaign roster;roster.Reset(0);CHECK(roster.generals.size()==72);
 int factions[3]{};for(size_t i=0;i<roster.generals.size();++i){const auto& g=roster.generals[i];++factions[g.faction];CHECK(campaignvisual::PortraitIndex(g.name)>=0);CHECK(roster.cities[g.home].owner==g.faction);for(size_t j=0;j<i;++j)CHECK(g.name!=roster.generals[j].name);}
 CHECK(factions[0]==24 && factions[1]==24 && factions[2]==24);
 roster.ResetLegacy(0);CHECK(roster.generals.size()==24);
 std::cout << "Expanded roster, terrain, arms, tactical casualties, expiry, morale and aura checks passed" << std::endl;
	std::cout << "Battlefield interdiction, escort relief, directional pressure, guard and ceasefire passed" << std::endl;
}
