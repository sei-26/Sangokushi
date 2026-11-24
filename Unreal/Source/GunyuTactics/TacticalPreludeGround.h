#pragma once
#include "TacticalMapMesh.h"
#include "JourneyBattlefield.hpp"
#include "StrategistJourney.hpp"
namespace gunyupreludeground
{
    // Leaf litter follows actual forest/plain contacts, never unit positions or orders.
    // Keep irregular patches on the wooded side instead of outlining a hex tile.
    inline void ForestMargins(const frontline::StrategistJourney& J,TacticalMap::FSurface& Land,
        TFunctionRef<FVector(int)> Location,TFunctionRef<float(FVector)> Height)
    {
        using namespace frontline;
        if(J.Battlefield()!=JourneyMap::Retreat && J.Battlefield()!=JourneyMap::YangrenCounter) return;
        for(int Y=1;Y<StrategistJourney::Rows-1;++Y) for(int X=1;X<StrategistJourney::Columns-1;++X)
        {
            const int Tile=Campaign::At(X,Y);
            if(J.world.tiles[Tile].terrain!=Terrain::Forest) continue;
            const FVector Center=Location(Tile);
            for(int Other:J.world.MapNeighbors(Tile))
            {
                if(!StrategistJourney::OnMap(Other) || J.world.tiles[Other].terrain!=Terrain::Plain) continue;
                const FVector Neighbor=Location(Other),Out=(Neighbor-Center).GetSafeNormal2D(),Side(-Out.Y,Out.X,0);
                const FVector Edge=(Center+Neighbor)*.5-Out*19;
                const float Shade=J.intelligence[0].visible[Tile] ? 1.f:.4f;
                for(int Patch=0;Patch<3;++Patch)
                {
                    FVector P=Edge+Side*((Patch-1)*43+FMath::Sin(float(Tile+Patch*5))*7)-Out*(Patch%2*8);
                    P.Z=Height(P)+.8;
                    const FLinearColor Color=FLinearColor(.145f,.12f,.060f)*Shade;
                    for(int I=0;I<9;++I)
                    {
                        auto Rim=[&](int N)
                        {
                            const float Angle=N*2*PI/9,Radius=21+5*FMath::Sin(N*2.7+Tile+Patch);
                            FVector Q=P+Side*(FMath::Cos(Angle)*Radius*1.45)+Out*(FMath::Sin(Angle)*Radius*.70);
                            Q.Z=Height(Q)+.8;return Q;
                        };
                        Land.Triangle(P,Rim(I),Rim(I+1),Color,Color*.82f,Color*.82f);
                    }
                }
            }
        }
    }
}
