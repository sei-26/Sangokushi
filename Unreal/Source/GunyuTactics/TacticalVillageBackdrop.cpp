#include "TacticalTrial.h"
#include "Components/InstancedStaticMeshComponent.h"

void ATacticalField::BuildVillageBackdrop()
{
    auto Box=[](UInstancedStaticMeshComponent* Mesh,FVector P,FVector Size,FRotator R=FRotator::ZeroRotator)
    { Mesh->AddInstance(FTransform(R,P,Size/100)); };
    auto House=[&](FVector P,float Size,float Yaw,bool Hall)
    {
        P.Z=SceneryFloor(P);const FRotator R(0,Yaw,0);
        auto Point=[&](FVector V) { return P+R.RotateVector(V*Size); };
        Box(CityWalls,Point(FVector(0,0,15)),FVector(188,128,30)*Size,R);
        Box(CityTimber,Point(FVector(0,0,63)),FVector(166,112,67)*Size,R);
        for(int X:{-1,1}) for(int Y:{-1,1})
            Box(CityTimber,Point(FVector(X*78,Y*51,67)),FVector(8,8,89)*Size,R);
        CityRoofs->AddInstance(FTransform(FRotator(0,Yaw+90,0),Point(FVector(0,0,105)),FVector(160,224,74)*Size/100));
        if(Hall)
        {
            Box(CityTimber,Point(FVector(0,-71,89)),FVector(201,8,9)*Size,R);
            for(int X:{-1,1}) Box(CityTimber,Point(FVector(X*88,-71,45)),FVector(8,8,90)*Size,R);
            CityRoofs->AddInstance(FTransform(FRotator(0,Yaw+90,0),Point(FVector(0,-48,81)),FVector(140,250,55)*Size/100));
        }
        // Split rails, stacked firewood and a broad empty lane between homes.
        for(int I=0;I<4;++I)
            Box(CityTimber,Point(FVector(110+I*13,12,13+(I%2)*9)),FVector(12,55,12)*Size,R);
    };
    const FVector Homes[]={FVector(770,-250,0),FVector(1090,-290,0),FVector(1790,-230,0),FVector(2090,-340,0),
        FVector(650,-580,0),FVector(1030,-640,0),FVector(1800,-620,0),FVector(2160,-750,0),FVector(1380,-890,0)};
    for(int I=0;I<UE_ARRAY_COUNT(Homes);++I) House(Homes[I],.75f+(I%3)*.15f,(I%3)*11-10,false);
    House(FVector(1430,-520,0),1.30f,0,true);
    // Low boundary walls do not resemble a fortified city or block the field.
    for(int I=0;I<12;++I)
    {
        const double X=650+I*145;
        if(X>1250 && X<1650) continue;
        const FVector P(X,-95,SceneryFloor(FVector(X,-95,0)));
        Box(CityWalls,P+FVector(0,0,21),FVector(138,27,42));
        Box(CityTimber,P+FVector(0,0,48),FVector(7,7,39));
    }
}
