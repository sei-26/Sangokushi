#pragma once
#include "CoreMinimal.h"
#include "HexGrid.hpp"
#include "JourneyBattlefield.hpp"
#include "StrategistJourney.hpp"
namespace gunyucrossing
{
    inline bool IsPort(const frontline::StrategistJourney& J)
    { return J.Battlefield()==frontline::JourneyMap::Port || J.Battlefield()==frontline::JourneyMap::PortNorth; }
    // Public bridge geometry, in north-to-south order. No armies, AI or ownership.
    inline TArray<int> Bridges(const frontline::StrategistJourney& J)
    {
        TArray<int> Out;if(!IsPort(J) && !J.HasCoalitionBattle()) return Out;
        for(int Y=1;Y<frontline::StrategistJourney::Rows-1;++Y)
            for(int X=1;X<frontline::StrategistJourney::Columns-1;++X)
            {
                const int Tile=frontline::Campaign::At(X,Y);
                const auto T=J.stage==frontline::JourneyStage::Preparation ? J.PlanningTerrain(Tile):J.world.tiles[Tile].terrain;
                if(T==frontline::Terrain::Bridge) Out.Add(Tile);
            }
        return Out;
    }
    inline bool OnBridge(const frontline::StrategistJourney& J,FVector P)
    {
        const auto C=hexgrid::Pick(P.X/200.,P.Y/200.);
        if(C.first<1 || C.first>=frontline::StrategistJourney::Columns-1 || C.second<1 || C.second>=frontline::StrategistJourney::Rows-1) return false;
        return J.world.tiles[frontline::Campaign::At(C.first,C.second)].terrain==frontline::Terrain::Bridge;
    }
}
