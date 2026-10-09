#include "../NewGame/Campaign.hpp"
#include "../NewGame/HeroStory.hpp"
#include <iostream>
#include <set>
#include <cstdlib>
#include <chrono>
#define CHECK(x) do { if(!(x)){std::cerr<<"AI check failed "<<__LINE__<<": "<<#x<<std::endl;std::exit(1);} }while(false)
using namespace frontline;
Campaign Small()
{
    Campaign g;g.ResetLegacy(0);g.legacyLayout=false;
    for(int p=0;p<TileCount;++p)g.tiles[p]={Terrain::Plain,p/Width<10?0:p/Width<20?1:2};
    for(int c=0;c<9;++c){g.cities[c].tile=Campaign::At(4+(c%3)*4,4+(c/3)*10);g.tiles[g.cities[c].tile].owner=g.cities[c].owner;}
    return g;
}
Army Unit(int faction,int general,int tile,int troops=4000)
{
    Army a;a.faction=faction;a.general=general;a.tile=tile;a.target=tile;a.troops=troops;return a;
}
void Invariants(const Campaign& g)
{
    std::set<int> reserved;
    for(const auto& c:g.cities) {
        CHECK(c.food>=0 && c.gold>=0 && c.troops>=0 && g.tiles[c.tile].owner==c.owner);
        if(c.worker>=0){CHECK(reserved.insert(c.worker).second);if(c.helper>=0)CHECK(reserved.insert(c.helper).second);}
    }
    for(const auto& m:g.missions){CHECK(reserved.insert(m.general).second);if(m.helper>=0)CHECK(reserved.insert(m.helper).second);}
    for(const auto& m:g.assignments)CHECK(reserved.insert(m.general).second);
    for(const auto& a:g.armies)if(a.troops>0) {
        CHECK(reserved.insert(a.general).second && a.faction==g.generals[a.general].faction);
        CHECK(a.food>=0 && a.cargoFood>=0 && a.morale>=0 && a.morale<=100 && g.Cost(a.tile,a.arm)<100000);
        if(!a.path.empty())CHECK(g.MapDistance(a.tile,a.path.front())==1);
    }
    for(int f=0;f<3;++f)CHECK(g.ArmyCount(f)<=Campaign::MaxArmies && g.aiCommands[f]>=0 && g.aiCommands[f]<=3);
}
int main()
{
    Campaign budget=Small();budget.BeginTurn();
    const auto cities=budget.cities;const auto credit=budget.aiCommands;const auto notes=budget.chronicle.size();
    budget.BeginTurn();CHECK(budget.chronicle.size()==notes && budget.aiCommands==credit);
    for(size_t c=0;c<cities.size();++c)CHECK(budget.cities[c].gold==cities[c].gold && budget.cities[c].food==cities[c].food);
    budget.day=1;budget.BeginTurn();CHECK(budget.aiCommands[1]==0 && budget.aiCommands[2]==0);
    budget.day=10;budget.BeginTurn();CHECK(budget.aiPlannedDay[1]==10);
    Campaign threat=Small();threat.armies.push_back(Unit(0,0,Campaign::At(5,14),6000));threat.BeginTurn();
    CHECK(threat.ArmyCount(1)>0 && threat.cities[3].troops>=2000 && threat.commands==3);
    bool defender=false;for(const auto& a:threat.armies)if(a.faction==1 && a.target==threat.armies[0].tile)defender=true;CHECK(defender);
    Campaign food=Small();food.cities[3].food=1000;const int reserve=food.cities[4].food;food.BeginTurn();
    int cargo=-1;for(int i=0;i<static_cast<int>(food.armies.size());++i)if(food.armies[i].faction==1 && food.armies[i].arm==Arm::Transport)cargo=i;
    CHECK(cargo>=0 && food.armies[cargo].target==food.cities[3].tile && food.armies[cargo].cargoFood==5000);
    CHECK(food.cities[4].food+food.armies[cargo].food+food.armies[cargo].cargoFood==reserve && food.Busy(food.armies[cargo].general));
    Campaign retreat=Small();retreat.armies={Unit(1,6,Campaign::At(5,12),1100),Unit(0,0,Campaign::At(8,7),1100)};
    retreat.armies[0].target=retreat.cities[0].tile;retreat.AdvanceDay();
    CHECK(retreat.armies[0].retreat && retreat.cities[retreat.CityAt(retreat.armies[0].target)].owner==1 && !retreat.armies[1].retreat);
    for(int d=0;d<10;++d)retreat.AdvanceDay();CHECK(retreat.armies[0].troops==0 && !retreat.Busy(6));
    Campaign peace=Small();peace.armies={Unit(1,6,Campaign::At(5,12))};peace.armies[0].target=peace.cities[0].tile;
    peace.truceUntil[0][1]=peace.truceUntil[1][0]=60;peace.AdvanceDay();CHECK(peace.armies[0].retreat);
    Campaign disconnected=Small();disconnected.day=60;
    for(int x=0;x<Width;++x)disconnected.tiles[Campaign::At(x,10)].terrain=Terrain::Sea;
    disconnected.truceUntil[1][2]=disconnected.truceUntil[2][1]=120;disconnected.BeginTurn();CHECK(disconnected.ArmyCount(1)==0);
    Campaign idle=Small();idle.armies={Unit(1,6,Campaign::At(5,12))};idle.generals[6].tactic=Tactic::Charge;idle.day=4;idle.AdvanceDay();CHECK(!idle.armies[0].tacticQueued && idle.armies[0].tacticReadyDay==0);
    // Siege forces wait for a viable escort; waiting cannot accumulate movement credit.
    Campaign lone=Small();lone.armies={Unit(1,6,lone.cities[3].tile)};
    lone.armies[0].arm=Arm::Siege;CHECK(lone.Order(0,lone.cities[0].tile));
    const int staging=lone.armies[0].tile;const int stores=lone.cities[3].food;
    for(int d=0;d<19;++d)lone.AdvanceDay();
    CHECK(lone.armies[0].tile==staging && lone.armies[0].aiAssemblyDays==19 && lone.armies[0].movement==0 && lone.cities[3].food<stores);
    lone.AdvanceDay();CHECK(lone.armies[0].troops==0 && lone.armies[0].aiAssemblyDays==0 && !lone.Busy(6));
    Campaign joined=Small();joined.armies={Unit(1,6,Campaign::At(4,14)),Unit(1,7,Campaign::At(4,17))};
    joined.armies[0].arm=Arm::Siege;
    CHECK(joined.Order(0,joined.cities[0].tile) && joined.Order(1,joined.cities[0].tile));
    joined.AdvanceDay();joined.AdvanceDay();CHECK(joined.armies[0].tile!=Campaign::At(4,14) && joined.armies[0].aiAssemblyDays==0);
    Campaign ahead=Small();ahead.armies={Unit(1,6,Campaign::At(4,18)),Unit(1,7,Campaign::At(4,12))};
    ahead.armies[0].arm=Arm::Siege;ahead.armies[1].arm=Arm::Cavalry;
    CHECK(ahead.Order(0,ahead.cities[0].tile) && ahead.Order(1,ahead.cities[0].tile));
    ahead.AdvanceDay();CHECK(ahead.armies[0].tile==Campaign::At(4,18) && ahead.armies[1].tile==Campaign::At(4,12) && ahead.armies[1].aiAssemblyDays==1);
    // The rear siege closes the gap while the leading cavalry holds position.
    for(int d=0;d<4;++d)ahead.AdvanceDay();CHECK(ahead.armies[0].tile!=Campaign::At(4,18));
    Campaign reinforcement=Small();reinforcement.day=60;reinforcement.armies={Unit(1,6,Campaign::At(4,14))};
    reinforcement.armies[0].arm=Arm::Siege;CHECK(reinforcement.Order(0,reinforcement.cities[0].tile));reinforcement.BeginTurn();
    bool reinforced=false;
    for(size_t i=1;i<reinforcement.armies.size();++i)if(reinforcement.armies[i].faction==1 && reinforcement.armies[i].arm!=Arm::Siege && reinforcement.armies[i].target==reinforcement.cities[0].tile)reinforced=true;
    CHECK(reinforced);
    Campaign playerSiege=Small();playerSiege.armies={Unit(0,0,Campaign::At(4,5))};playerSiege.armies[0].arm=Arm::Siege;
    CHECK(playerSiege.Order(0,playerSiege.cities[3].tile));playerSiege.AdvanceDay();playerSiege.AdvanceDay();CHECK(playerSiege.armies[0].tile!=Campaign::At(4,5) && playerSiege.armies[0].aiAssemblyDays==0);
    std::cout<<"AI siege assembly, support deployment, bounded waiting and player control passed"<<std::endl;
    hero::Story story;story.chapter=5;story.phase=2;story.StartMission();story.units.resize(3);
    story.units[0].hero=0;story.units[0].x=4;story.units[0].y=3;story.units[0].enemy=false;
    story.units[1].hero=1;story.units[1].x=7;story.units[1].y=5;story.units[1].enemy=false;
    story.units[2].hero=-1;story.units[2].enemy=true;story.units[2].x=6;story.units[2].y=3;story.units[2].range=2;story.units[2].stunned=0;
    story.terrain.fill(0);for(int y=0;y<hero::H;++y)story.terrain[y*hero::W+5]=2;
    auto intent=story.PlanEnemy(2);CHECK(intent.target==1 && intent.next>=0 && story.At(intent.next%hero::W,intent.next/hero::W)<0);
    story.units[2].stunned=1;CHECK(story.PlanEnemy(2).target==-1);story.units[2].stunned=0;
    story.terrain.fill(0);story.units[0].x=5;story.units[0].y=3;story.units[1].x=7;story.units[1].y=3;story.units[1].hp=1;
    CHECK(story.PlanEnemy(2).target==1);
    const auto start=std::chrono::steady_clock::now();
    int ticks=0,launched=0;
    for(int faction=0;faction<3;++faction) {
        Campaign g;g.Reset(faction);
        for(int d=0;d<1200 && g.result==0;++d) {
            if(d%10==0){g.BeginTurn();const auto actions=g.chronicle.size();g.BeginTurn();CHECK(g.chronicle.size()==actions);}
            g.AdvanceDay();Invariants(g);++ticks;
        }
        launched+=static_cast<int>(g.armies.size());
    }
    CHECK(ticks>=1800 && launched>0);
    const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    std::cout<<"AI budgets, emergencies, logistics, retreat, ceasefire, reachability and hero intents passed"<<std::endl;
    std::cout<<"Three-faction simulation: "<<ticks<<" days, "<<launched<<" launched units, "<<ms<<" ms"<<std::endl;
}
