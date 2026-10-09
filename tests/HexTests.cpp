#include "../NewGame/Campaign.hpp"
#include "../NewGame/MapCamera.hpp"
#include <iostream>
#include <cstdlib>
#define CHECK(x) do { if(!(x)){std::cerr<<"Hex check "<<__LINE__<<": "<<#x<<std::endl;std::exit(1);} }while(false)
int main()
{
	using namespace frontline;
	Campaign g;g.Reset(0);CHECK(g.hexMap);
	for(int y=1;y<Height-1;++y)for(int x=1;x<Width-1;++x)
	{
		const int p=Campaign::At(x,y);const auto neighbors=g.MapNeighbors(p);CHECK(neighbors.size()==6);
		for(int n:neighbors){CHECK(g.MapDistance(p,n)==1 && g.MapDistance(n,p)==1);const auto back=g.MapNeighbors(n);CHECK(std::find(back.begin(),back.end(),p)!=back.end());}
		const auto c=hexgrid::Center(x,y);const auto picked=hexgrid::Pick(c.first,c.second);CHECK(picked.first==x && picked.second==y);
	}
	world::MapCamera camera;camera.Fit(96,64,true);const double vw=890,vh=530;
	for(int y=18;y<45;y+=7)for(int x=22;x<75;x+=9)
	{
		camera.x=x;camera.y=y*hexgrid::Row+hexgrid::Radius;camera.zoom=4;
		const auto c=hexgrid::Center(x,y);const double z=camera.Cell(vw,vh),mx=vw/2+(c.first-camera.x)*z,my=vh/2+(c.second-camera.y)*z;
		CHECK(camera.Tile(mx,my,vw,vh,Width)==Campaign::At(x,y));camera.Zoom(1.2,mx,my,vw,vh);CHECK(camera.Tile(mx,my,vw,vh,Width)==Campaign::At(x,y));
	}
    // Perspective projection must preserve picking and the cursor zoom anchor.
    camera.Fit(96,64,true,true);
    for(int y=20;y<44;y+=6)for(int x=28;x<70;x+=8)
    {
        const auto center=hexgrid::Center(x,y);
        camera.x=48;camera.y=28;camera.zoom=3;
        double cell=camera.Cell(vw,vh);
        const double mx=vw/2+(center.first-camera.x)*cell, my=vh/2+(center.second-camera.y)*cell*camera.Pitch();
        if(mx<0 || my<0 || mx>=vw || my>=vh)continue;
        CHECK(camera.Tile(mx,my,vw,vh,Width)==Campaign::At(x,y));
        camera.Zoom(1.2,mx,my,vw,vh);CHECK(camera.Tile(mx,my,vw,vh,Width)==Campaign::At(x,y));
    }
    camera.x=48;camera.y=28;camera.zoom=4;
    const double beforeX=camera.x,beforeY=camera.y,scale=camera.Cell(vw,vh);
    camera.Pan(20,15,vw,vh);
    CHECK(std::abs(camera.x-(beforeX-20/scale))<1e-8);
    CHECK(std::abs(camera.y-(beforeY-15/(scale*camera.Pitch())))<1e-8);
	camera.Fit(96,64,true);camera.Pan(100000,100000,vw,vh);CHECK(camera.x>0 && camera.y>0);
	const int a=Campaign::At(30,30),b=Campaign::At(32,28);CHECK(g.MapDistance(a,b)==3);
	for(auto& t:g.tiles)t={Terrain::Plain,0};
	const auto path=g.Route(a,b,Arm::Spear,0);CHECK(path.size()==static_cast<size_t>(g.MapDistance(a,b)));
	int previous=a;for(int n:path){CHECK(g.MapDistance(previous,n)==1);previous=n;}
	CHECK(g.ClearShot(a,b));
	bool obstructed=false;for(int n:path)if(n!=b){g.tiles[n].terrain=Terrain::Forest;if(!g.ClearShot(a,b))obstructed=true;g.tiles[n].terrain=Terrain::Plain;}
	CHECK(obstructed);
	Army victim;victim.faction=0;victim.general=0;victim.tile=victim.target=a;victim.troops=3000;g.armies={victim};
	for(int n:g.MapNeighbors(a)){Army enemy=victim;enemy.faction=1;enemy.general=6;enemy.tile=enemy.target=n;g.armies.push_back(enemy);}
	CHECK(g.PressureDirections(0)==6 && g.PressureDamagePercent(0)==45 && g.PressureMoraleLoss(0)==9);
	g.armies.clear();g.ResetLegacy(0);CHECK(!g.hexMap && g.MapNeighbors(Campaign::At(10,10)).size()==4);
	std::cout<<"Hex adjacency, coordinates, picking, zoom, routes, rays, six-sided pressure and legacy geometry passed"<<std::endl;
}
