#include "../NewGame/CampaignInformation.hpp"
#include <cassert>
#include <iostream>
using namespace frontline;
int main() {
 Campaign g;g.Reset(0);
 assert(information::Rows(g,information::Tab::Cities).size()==30);
 assert(information::Rows(g,information::Tab::Officers).size()==72);
 for(int f=0;f<3;++f){const auto ids=information::Rows(g,information::Tab::Officers,f,1);assert(ids.size()==24);for(size_t i=0;i<ids.size();++i){assert(g.generals[ids[i]].faction==f);if(i>0)assert(g.generals[ids[i-1]].leadership>=g.generals[ids[i]].leadership);}}
 const auto originalGold=g.cities[0].gold;const auto commands=g.commands;
 auto officers=information::Relations(g,0);assert(officers.size()==71);for(size_t i=1;i<officers.size();++i)assert(g.Affinity(0,officers[i-1])>=g.Affinity(0,officers[i]));
 assert(g.cities[0].gold==originalGold && g.commands==commands && g.day==0);
 assert(information::Relations(g,-1).empty() && information::CityMembers(g,-1).empty() && information::Rows(g,information::Tab::Cities,8).empty());
 auto staff=information::CityMembers(g,0);assert(std::find(staff.begin(),staff.end(),24)!=staff.end());
 g.cities[0].owner=1;staff=information::CityMembers(g,0);for(int id:staff)assert(g.generals[id].faction==1);
 assert(information::PostOf(g,24).kind==information::Post::Displaced);
 g.Reset(0);g.generals[24].readyDay=8;auto post=information::PostOf(g,24);assert(post.kind==information::Post::Rest && post.days==8);
 g.Reset(0);assert(g.Develop(0,19,Duty::Commerce,false,1));assert(information::PostOf(g,19).kind==information::Post::Development && information::PostOf(g,1).kind==information::Post::Development);
 g.Reset(0);assert(g.AssignOfficer(19,9));post=information::PostOf(g,19);assert(post.kind==information::Post::Transfer && post.days==g.assignments[0].left);
 g.Reset(0);assert(g.SendMission(1,18,3,MissionKind::Diplomacy,3));assert(information::PostOf(g,18).kind==information::Post::Mission && information::PostOf(g,3).kind==information::Post::Mission);
 g.Reset(0);const auto totalBefore=information::Faction(g,0);assert(g.DispatchTransport(0,19,9,10000)>=0);const auto totalAfter=information::Faction(g,0);
 assert(totalBefore.food==totalAfter.food && totalBefore.troops==totalAfter.troops && totalAfter.armies==1 && totalAfter.gold==totalBefore.gold-100);
 assert(information::PostOf(g,19).kind==information::Post::Army);
 assert(information::Rows(g,information::Tab::Armies,0).size()==1 && information::Rows(g,information::Tab::Armies,1).empty());
 g.armies[0].troops=0;assert(information::Rows(g,information::Tab::Armies).empty());
 g.Reset(0);for(auto& c:g.cities){c.owner=0;c.gold=100000000;}
 assert(information::Faction(g,0).gold==3000000000LL && information::Faction(g,2).cities==0 && information::Faction(g,-1).gold==0);
 g.Note(U"A");g.Note(U"B");const auto entries=information::Rows(g,information::Tab::Chronicle);assert(entries.front()==static_cast<int>(g.chronicle.size())-1);
 g.ResetLegacy(1);assert(information::Rows(g,information::Tab::Cities).size()==9 && information::Rows(g,information::Tab::Officers).size()==24);
 std::cout<<"Information filtering, sorting, posting, relationships, 64-bit totals, resource conservation and legacy views passed\n";
}
