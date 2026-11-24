#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalMapMesh.h"
#include "TacticalGroundDetail.h"
#include "JourneyBattlefield.hpp"
#include "TacticalCrossingLandscape.h"

void ATacticalField::UpdateRoadTopology()
{
    const auto& J=State->Journey;
    auto& Road=State->LandscapeRoad;Road.Reset();
    using namespace frontline;
    const auto Map=J.Battlefield();
    auto Add=[&](std::initializer_list<std::pair<int,int>> Cells)
    { for(const auto& [X,Y]:Cells) Road.Add(TileLocation(Campaign::At(X,Y))); };
    if(Map==JourneyMap::Retreat)
    {
        Add({{2,10},{3,10},{4,11},{6,11},{8,11},{9,11},{10,11},{11,10},{12,10},{14,10},{15,10}});
        return;
    }
    if(gunyucrossing::IsPort(J))
    {
        // One continuous loop joins both real crossings without a diagonal road over water.
        // The south approach still passes every legacy convoy checkpoint.
        Add({{2,10},{3,10},{4,11},{6,11},{9,11},{10,11},{11,11},{12,11},
             {12,10},{12,9},{12,8},{11,8},{10,8},{10,9},{10,10},{10,11}});
        return;
    }
    if(Map==JourneyMap::CoalitionFronts)
    {
        // Join the actual western assembly point, both crossings and Cao Cao's
        // forward position. The east-bank road bends around the southern wood.
        Add({{3,9},{5,9},{6,9},{7,9},{8,9},{9,9},{11,9},{11,8},{11,7},
             {10,6},{10,5},{13,5},{13,4},{10,4},{7,4},{5,4},{5,6},{5,9}});
        return;
    }
    if(Map==JourneyMap::LuoyangGate)
    {
        Add({{3,10},{6,9},{8,8},{10,7},{11,5},{11,3}});
        return;
    }
    if(Map==JourneyMap::LuoyangDepot)
    {
        Add({{3,10},{5,8},{8,7},{10,7},{12,7},{14,6}});
        return;
    }
    if(Map==JourneyMap::Yangren || Map==JourneyMap::YangrenCounter)
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
    if(Map==JourneyMap::GuangzongFronts)
    {
        // Northern relief bypasses the river; the two real bridges join the
        // central and southern approaches on their own banks.
        Add({{4,6},{5,5},{6,3},{8,3},{10,3},{10,5},{11,6},{13,6},
             {12,8},{10,8},{9,8},{8,8},{7,8},{6,8},{6,10},{7,11},
             {8,11},{9,11},{11,11},{13,12}});
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
