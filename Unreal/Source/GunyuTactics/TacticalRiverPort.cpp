#include "TacticalTrial.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "TacticalSceneryLayout.h"

void ATacticalField::BuildRiverPort()
{
    auto Wood=[&](FVector P,FVector Size,FRotator R=FRotator::ZeroRotator)
    { CityTimber->AddInstance(FTransform(R,P,Size/100)); };
    for(int Dock=0;Dock<3;++Dock)
    {
        const double Y=-350-Dock*245,River=TacticalMap::PortRiverX(Y);
        const double Outer=River+24,Inner=2645;
        for(double X=Outer;X<Inner;X+=15)
            Wood(FVector(X,Y,8),FVector(14,90,10));
        for(double X:{Outer+12,Inner-18}) for(int Side:{-1,1})
            Wood(FVector(X,Y+Side*39,-9),FVector(10,10,56));
        // Low cargo barges, with a raised bow and open hold.
        const FVector Boat(River-18,Y+107,-6);
        Wood(Boat,FVector(72,153,10));
        for(int Side:{-1,1})
            Wood(Boat+FVector(Side*35,0,12),FVector(7,156,28));
        for(int End:{-1,1})
            Wood(Boat+FVector(0,End*80,13),FVector(67,9,30),FRotator(End*12,0,0));
        for(int Crate=0;Crate<3;++Crate)
            Wood(Boat+FVector((Crate%2)*27-15,(Crate/2)*34-25,14),FVector(23,27,24));
        const FVector Shed(2600,Y-70,SceneryFloor(FVector(2600,Y-70,0)));
        Wood(Shed+FVector(0,0,28),FVector(65,65,56));
        CityRoofs->AddInstance(FTransform(FRotator(0,90,0),Shed+FVector(0,0,63),FVector(.83,.95,.43)));
    }
}
