#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalMapMesh.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "JourneyBattlefield.hpp"

// Static, instanced scenery. Farm buildings sit at the edges of playable cells
// so roads and formation centers stay open.
void ATacticalField::BuildMapScenery()
{
    using namespace frontline;
    TacticalMap::FSurface Banks;
    const auto& J=State->Journey;
    for (auto* Part : {Palisades.Get(),Watchtowers.Get(),Tents.Get(),SupplyCrates.Get(),Stonework.Get()}) Part->ClearInstances();
    auto Wood=[&](FVector P,FVector Size,FRotator R=FRotator::ZeroRotator)
    { Buildings->AddInstance(FTransform(R,P,Size/100)); };
    auto House=[&](FVector P,float Size,float Angle)
    {
        P.Z=SurfaceHeight(P);
        const FRotator R(0,Angle,0);
        Houses->AddInstance(FTransform(R,P+FVector(0,0,38*Size),FVector(1.12,.85,.76)*Size));
        Roofs->AddInstance(FTransform(R,P+FVector(0,0,82*Size),FVector(1.34,1.13,.62)*Size));
        for (float Side : {-1.f,1.f})
        {
            Wood(P+R.RotateVector(FVector(Side*50,-44,39)*Size),FVector(6,5,78)*Size,R);
            Wood(P+R.RotateVector(FVector(Side*30,-44,46)*Size),FVector(18,4,23)*Size,R);
        }
        Wood(P+R.RotateVector(FVector(0,-44,22)*Size),FVector(19,5,44)*Size,R);
    };
    auto Field=[&](FVector Base,int Rows,int Crops,float RowLength,float FurrowWidth)
    {
        for (int Row=0;Row<Rows;++Row)
        {
            FVector A=Base+FVector(0,Row*13,0),B=A+FVector(RowLength,0,0);
            A.Z=SurfaceHeight(A)+1; B.Z=SurfaceHeight(B)+1;
            Banks.Line(A,B,FurrowWidth,FLinearColor(.105f,.075f,.035f));
            for (int Crop=0;Crop<Crops;++Crop)
            {
                FVector P=FMath::Lerp(A,B,Crop/float(Crops-1)); P.Z=SurfaceHeight(P)+12;
                Grass->AddInstance(FTransform(FRotator(0,35,0),P,FVector(.055,.08,.26)));
            }
        }
    };
    const auto Map=J.Battlefield();
    if(Map==JourneyMap::CoalitionRoad || Map==JourneyMap::RefugeeRoad)
    {
        // Field borders and houses beside the road, clear of its formation centers.
        const TArray<FIntPoint> Farms=Map==JourneyMap::CoalitionRoad ?
            TArray<FIntPoint>{{6,10},{7,10},{10,8},{11,8},{10,7}}:TArray<FIntPoint>{{5,10},{6,10},{11,10},{12,10}};
        for(const FIntPoint& Farm:Farms)
        {
            const FVector P=TileLocation(Campaign::At(Farm.X,Farm.Y));
            Field(P+FVector(-79,-74,0),7,9,158,11.f);
        }
        const TArray<FIntPoint> Homesteads=Map==JourneyMap::CoalitionRoad ?
            TArray<FIntPoint>{{4,7},{4,8},{5,8},{10,6},{11,6}}:TArray<FIntPoint>{{4,9},{5,9}};
        for(int I=0;I<Homesteads.Num();++I)
        {
            const FIntPoint Cell=Homesteads[I];
            House(TileLocation(Campaign::At(Cell.X,Cell.Y))+FVector(-46,33,0),.58f+(I%3)*.08f,(I%2)*18-9);
        }
    }
    // The playable board ends before these houses.
    const int HousesCount=Map==JourneyMap::RefugeeWood ? 2:Map==JourneyMap::CoalitionRoad ? 5:J.mission==0 ? 7:J.mission==1 ? 3:0;
    for (int I=0;I<HousesCount;++I)
        House(FVector(470+(I%4)*163,2335+(I/4)*174,0),.68f+(I%3)*.08f,(I%2)*12-6);
    for (int Index=0;Index<(J.mission==0 ? 2 : J.mission==1 ? 1 : 0);++Index)
    {
        Field(FVector(1240+Index*220,2390,0),13,10,178,8.f);
    }
    for (int Y=1;Y<StrategistJourney::Rows-1;++Y) for (int X=1;X<StrategistJourney::Columns-1;++X)
    {
        const int Tile=Campaign::At(X,Y);
        const auto Terrain=J.world.tiles[Tile].terrain;
        const FVector Center=TileLocation(Tile);
        if (Terrain==Terrain::Bridge)
        {
            // Orient the crossing perpendicular to the actual river. The
            // canyon's east/west channel requires north/south bridge decks.
            TArray<FVector> Wet;
            for (int Other : J.world.MapNeighbors(Tile))
            {
                const auto T=J.world.tiles[Other].terrain;
                if (T==Terrain::River || T==Terrain::Bridge) Wet.Add(TileLocation(Other));
            }
            const FVector Along=Wet.Num()>=2 ? Wet[1]-Wet[0] : FVector(0,200,0);
            const FVector Across(Along.Y,-Along.X,0);
            const FRotator Rotation(0,FMath::RadiansToDegrees(FMath::Atan2(Across.Y,Across.X)),0);
            auto Point=[&](FVector Offset) { return Center+Rotation.RotateVector(Offset); };
            auto Beam=[&](FVector Offset,FVector Size) { Wood(Point(Offset),Size,Rotation); };
            // Deck top agrees with SurfaceHeight and leaves formation centers clear.
            for (int Side : {-1,1}) for (int End : {-1,1})
                Stonework->AddInstance(FTransform(Rotation,Point(FVector(End*103,Side*77,-10)),FVector(.18,.21,.20)));
            for (int Plank=0;Plank<22;++Plank)
                Beam(FVector(-105+Plank*10,0,-4),FVector(9,158,8));
            for (int Side : {-1,1})
            {
                for (int Post=0;Post<5;++Post)
                    Beam(FVector(-100+Post*50,Side*85,19),FVector(6,6,62));
                for (int Level : {14,36})
                    Beam(FVector(0,Side*85,Level),FVector(218,5,5));
                Beam(FVector(0,Side*55,-13),FVector(228,10,13));
            }
        }
    }
    BuildFortifications();
    Banks.Submit(Ground,3);
    Ground->SetMaterial(3,TerrainMaterial);
    if(State->BackdropSnapshot!=State->LandscapeSnapshot)
    {
        BuildBackdropGround();BuildFortressBackdrop();BuildMountainBackdrop();
        State->BackdropSnapshot=State->LandscapeSnapshot;
    }
    Ground->SetMaterial(4,DistantMaterial ? DistantMaterial.Get() : TerrainMaterial.Get());
    Ground->SetMaterial(6,TerrainMaterial);
}
