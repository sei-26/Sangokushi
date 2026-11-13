#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalMapMesh.h"
#include "TacticalGroundDetail.h"
#include "JourneyBattlefield.hpp"

void ATacticalField::UpdateRoadTopology()
{
    const auto& J=State->Journey;
    auto& Road=State->LandscapeRoad;Road.Reset();
    using namespace frontline;
    const auto Map=J.Battlefield();
    auto Add=[&](std::initializer_list<std::pair<int,int>> Cells)
    { for(const auto& [X,Y]:Cells) Road.Add(TileLocation(Campaign::At(X,Y))); };
    if(Map==JourneyMap::Yangren)
    {
        Add({{3,10},{6,10},{8,10},{10,10},{11,9},{11,8},{12,7},{14,6}});
        return;
    }
    if(Map==JourneyMap::CoalitionRoad)
    {
        Add({{3,11},{6,11},{8,11},{9,10},{9,8},{9,6},{10,5},{13,4}});
        return;
    }
    if(Map==JourneyMap::CoalitionFord)
    {
        Add({{4,9},{7,9},{9,8},{10,7},{11,6},{12,6},{14,5}});
        return;
    }
    if(Map==JourneyMap::Guangzong)
    {
        Add({{3,10},{6,10},{9,10},{11,9},{12,8},{14,7}});
        return;
    }
    if(Map==JourneyMap::PortNorth)
    {
        Add({{4,6},{7,7},{9,8},{11,8},{12,8},{13,10},{12,11}});
        return;
    }
    if(J.mission>1) return;
    // Only public terrain/route data. Never infer a road from a hidden army.
    for(int Tile : {frontline::Campaign::At(2,10),frontline::Campaign::At(3,10),
        frontline::Campaign::At(4,11),frontline::Campaign::At(5,11)}) Road.Add(TileLocation(Tile));
    for(int Tile:J.ConvoyPath()) Road.Add(TileLocation(Tile));
    if(J.mission==0 && !J.HasConvoy()) Road.Add(TileLocation(J.Definition().goal));
}

float ATacticalField::RoadDistance(FVector Point) const
{
    double Distance=10000;
    for(int I=1;I<State->LandscapeRoad.Num();++I)
        Distance=FMath::Min(Distance,TacticalMap::SegmentDistance2D(Point,State->LandscapeRoad[I-1],State->LandscapeRoad[I]));
    return Distance;
}

void ATacticalField::BuildRoadRuts(TacticalMap::FSurface& Land)
{
    const auto& J=State->Journey;
    for(int I=1;I<State->LandscapeRoad.Num();++I)
    {
        const FVector A=State->LandscapeRoad[I-1],B=State->LandscapeRoad[I];
        const FVector Along=(B-A).GetSafeNormal2D(),Across(-Along.Y,Along.X,0);
        const int Steps=FMath::Max(1,FMath::CeilToInt(FVector::Dist2D(A,B)/18.));
        for(int Step=0;Step<Steps;++Step) for(int Side:{-1,1})
        {
            FVector P=FMath::Lerp(A,B,Step/double(Steps))+Across*(Side*19);
            FVector Q=FMath::Lerp(A,B,(Step+1)/double(Steps))+Across*(Side*19);
            const auto Cell=hexgrid::Pick(P.X/200.,P.Y/200.);
            if(Cell.first<1 || Cell.first>=frontline::StrategistJourney::Columns-1 ||
                Cell.second<1 || Cell.second>=frontline::StrategistJourney::Rows-1) continue;
            const int Tile=frontline::Campaign::At(Cell.first,Cell.second);
            const auto Terrain=J.world.tiles[Tile].terrain;
            // The actual bridge deck has its own planks; no dirt floats over water.
            if(Terrain==frontline::Terrain::River || Terrain==frontline::Terrain::Bridge) continue;
            P.Z=SurfaceHeight(P)+.65;Q.Z=SurfaceHeight(Q)+.65;
            const float Shade=J.intelligence[0].visible[Tile] ? 1.f:.4f;
            Land.Line(P,Q,2.1f,FLinearColor(.11f,.095f,.058f)*Shade);
        }
    }
}
