#pragma once
#include "../NewGame/StrategistJourney.hpp"
#include <queue>
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
    // A player following visible routes can go around a river. Greedy distance
    // alone is not a valid playthrough once the battlefield has real crossings.
    const int from=j.world.armies[army].tile;
    const bool occupied=!TacticChain::Free(j.world,target);
    std::array<int,TileCount> first;first.fill(-1);first[from]=from;
    std::queue<int> open;open.push(from);int destination=-1;
    while(!open.empty() && destination<0)
    {
        const int at=open.front();open.pop();
        if(at==target || (occupied && j.world.MapDistance(at,target)<=1)) { destination=first[at];break; }
        for(int next:j.world.MapNeighbors(at))
            if(StrategistJourney::OnMap(next) && first[next]<0 && j.intelligence[0].visible[next] &&
               TacticChain::Free(j.world,next) && (!avoidRoad || next/Width!=11))
            { first[next]=at==from ? next:first[at];open.push(next); }
    }
    if(destination>=0 && destination!=from) j.Move(army,destination);
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
