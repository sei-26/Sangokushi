#include "../NewGame/Campaign.hpp"
#include "../NewGame/MapCamera.hpp"
#include <cassert>
#include <set>
#include <iostream>
#include <chrono>
int main(){
 using namespace frontline;
 Campaign game;game.Reset(0);std::set<int> sites;
 assert(TileCount==6144 && game.cities.size()==30);
 for(const auto& city:game.cities){
  assert(sites.insert(city.tile).second && game.Cost(city.tile,Arm::Spear)<100000);
  if(city.tile==game.cities[0].tile)continue;
  const auto path=game.Route(game.cities[0].tile,city.tile,Arm::Spear);
  assert(!path.empty() && path.back()==city.tile);
  int previous=game.cities[0].tile;
  for(int p:path){assert(Campaign::Distance(previous,p)==1 && game.Cost(p,Arm::Spear)<100000);previous=p;}
 }
 assert(game.Route(game.cities[0].tile,game.cities[6].tile,Arm::Spear).size()>50);
 world::MapCamera camera;camera.Fit(96,64);
 const double vw=890,vh=530;
 assert(camera.Tile(vw/2,vh/2,vw,vh,96)==Campaign::At(48,32));
 assert(camera.Tile(-1,100,vw,vh,96)==-1);
 // A zoom anchored inside the map must retain the tile beneath the cursor.
 const int anchor=camera.Tile(570,320,vw,vh,96);
 camera.Zoom(3,570,320,vw,vh);
 assert(camera.Tile(570,320,vw,vh,96)==anchor);
 camera.Pan(100000,100000,vw,vh);
 assert(camera.x>=0 && camera.y>=0 && camera.Tile(0,0,vw,vh,96)==0);
 camera.Zoom(100,vw/2,vh/2,vw,vh);assert(camera.zoom==6);
 camera.Zoom(.00001,vw/2,vh/2,vw,vh);assert(camera.zoom==1 && camera.x==48 && camera.y==32);
 camera.Fit(36,22);assert(camera.Tile(vw/2,vh/2,vw,vh,96)==Campaign::At(18,11));
 Campaign legacy;legacy.ResetLegacy(0);assert(legacy.cities.size()==9);
 assert(legacy.MapWidth()==36 && legacy.tiles[Campaign::At(36,0)].terrain==Terrain::Sea);
 const auto start=std::chrono::steady_clock::now();
 for(int d=0;d<180 && game.result==0;++d){if(d%10==0)game.BeginTurn();game.AdvanceDay();}
 const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
 std::cout<<"World routes and camera tests passed; 180 days: "<<ms<<" ms\n";
}
