#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalMapMesh.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

// Static, instanced scenery. Buildings occupy existing city cells or land
// outside the playable grid. The road and all walkable formation centers stay open.
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
    // A small village and cultivated fields beyond the southern board boundary.
    for (int I=0;I<7;++I)
        House(FVector(470+(I%4)*163,2335+(I/4)*174,0),.68f+(I%3)*.08f,(I%2)*12-6);
    for (int Field=0;Field<3;++Field)
    {
        const FVector Base(1240+Field*220,2390,0);
        for (int Row=0;Row<13;++Row)
        {
            FVector A=Base+FVector(0,Row*13,0),B=A+FVector(178,0,0);
            A.Z=SurfaceHeight(A)+1; B.Z=SurfaceHeight(B)+1;
            Banks.Line(A,B,8,FLinearColor(.105f,.075f,.035f));
            for (int Crop=0;Crop<10;++Crop)
            {
                FVector P=FMath::Lerp(A,B,Crop/9.f); P.Z=SurfaceHeight(P)+12;
                Grass->AddInstance(FTransform(FRotator(0,35,0),P,FVector(.055,.08,.26)));
            }
        }
    }
    for (int Y=1;Y<StrategistJourney::Rows-1;++Y) for (int X=1;X<StrategistJourney::Columns-1;++X)
    {
        const int Tile=Campaign::At(X,Y);
        const auto Terrain=J.world.tiles[Tile].terrain;
        const FVector Center=TileLocation(Tile);
        if (Terrain==Terrain::Bridge)
        {
            // Deck top is exactly the common SurfaceHeight used by soldiers,
            // selection rays and the grid; rails are outside the formation.
            for (int Side : {-1,1}) for (int End : {-1,1})
                Stonework->AddInstance(FTransform(FRotator::ZeroRotator,Center+FVector(End*103,Side*77,-10),FVector(.18,.21,.20)));
            for (int Plank=0;Plank<22;++Plank)
                Wood(Center+FVector(-105+Plank*10,0,-4),FVector(9,158,8));
            for (int Side : {-1,1})
            {
                for (int Post=0;Post<5;++Post)
                    Wood(Center+FVector(-100+Post*50,Side*85,19),FVector(6,6,62));
                for (int Level : {14,36})
                    Wood(Center+FVector(0,Side*85,Level),FVector(218,5,5));
                Wood(Center+FVector(0,Side*55,-13),FVector(228,10,13));
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
