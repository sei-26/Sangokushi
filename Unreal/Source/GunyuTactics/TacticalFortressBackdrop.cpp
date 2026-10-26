#include "TacticalTrial.h"
#include "Components/InstancedStaticMeshComponent.h"

// A visual settlement north of the playable grid. No mission/AI state is read.
// All masonry, roofs and details remain below world Y=50 (first hex starts >70).
void ATacticalField::BuildFortressBackdrop()
{
    for(auto* Part:{CityWalls.Get(),CityTimber.Get(),CityRoofs.Get()}) Part->ClearInstances();
    if(FortressMaterial) CityWalls->SetMaterial(0,FortressMaterial);
    auto Box=[](UInstancedStaticMeshComponent* Mesh,FVector P,FVector Size,FRotator R=FRotator::ZeroRotator)
    { Mesh->AddInstance(FTransform(R,P,Size/100)); };
    auto Floor=[this](double X,double Y) { return SceneryFloor(FVector(X,Y,0)); };
    auto Stone=[&](FVector P,FVector Size) { Box(CityWalls,P,Size); };
    auto Wood=[&](FVector P,FVector Size,FRotator R=FRotator::ZeroRotator) { Box(CityTimber,P,Size,R); };
    auto Roof=[&](FVector P,FVector Size)
    { CityRoofs->AddInstance(FTransform(FRotator(0,90,0),P,Size/100)); };
    auto Wall=[&](FVector A,FVector B)
    {
        const FVector D=B-A;const double Length=D.Size2D();const FVector Dir=D.GetSafeNormal2D();
        const int Count=FMath::CeilToInt(Length/90);const FVector Across(-Dir.Y,Dir.X,0);
        for(int I=0;I<Count;++I)
        {
            FVector P=FMath::Lerp(A,B,(I+.5)/Count);P.Z=Floor(P.X,P.Y);
            const FRotator R(0,Dir.Rotation().Yaw,0);
            Box(CityWalls,P+FVector(0,0,85),FVector(Length/Count+1,52,170),R);
            Box(CityWalls,P+FVector(0,0,20),FVector(Length/Count+2,69,40),R);
            Box(CityWalls,P+FVector(0,0,172),FVector(Length/Count+3,68,14),R);
            for(int Tooth=0;Tooth<2;++Tooth)
            {
                const FVector T=P+Dir*((Tooth-.5)*Length/Count*.5)+Across*9;
                Box(CityWalls,T+FVector(0,0,197),FVector(Length/Count*.28,42,37),R);
            }
        }
    };
    constexpr double West=1160,East=3100,Front=-240,Back=-1180,Gate=2300;
    Wall(FVector(West,Front,0),FVector(Gate-150,Front,0));
    Wall(FVector(Gate+150,Front,0),FVector(East,Front,0));
    Wall(FVector(West,Front,0),FVector(West,Back,0));
    Wall(FVector(East,Front,0),FVector(East,Back,0));
    Wall(FVector(West,Back,0),FVector(East,Back,0));
    auto Pavilion=[&](FVector P,float Size,bool Gatehouse)
    {
        P.Z=Floor(P.X,P.Y);
        // Broad stone plinth and two distinct timber storeys.
        if(!Gatehouse) Stone(P+FVector(0,0,82)*Size,FVector(190,155,164)*Size);
        else
        {
            for(int Side:{-1,1})
            {
                Stone(P+FVector(Side*160,0,82)*Size,FVector(52,150,164)*Size);
                Wood(P+FVector(Side*62,48,65)*Size,FVector(121,12,130)*Size);
                for(int Bar=0;Bar<3;++Bar) Wood(P+FVector(Side*62,39,30+Bar*35)*Size,FVector(112,7,6)*Size);
            }
            Stone(P+FVector(0,0,149)*Size,FVector(375,156,42)*Size);
            Stone(P+FVector(0,0,175)*Size,FVector(395,170,12)*Size);
        }
        for(int Level=0;Level<2;++Level)
        {
            const float Z=(170+Level*108)*Size;
            Stone(P+FVector(0,0,Z-5),FVector(220,184,12)*Size);
            for(int X:{-1,1}) for(int Y:{-1,1})
                Wood(P+FVector(X*81,Y*63,207+Level*108)*Size,FVector(9,9,75)*Size);
            // Dark window recesses and the projecting walk-around balcony.
            for(int Face:{-1,1})
            {
                Wood(P+FVector(0,Face*62,209+Level*108)*Size,FVector(154,7,57)*Size);
                for(int Post=-3;Post<=3;++Post)
                    Wood(P+FVector(Post*26,Face*79,182+Level*108)*Size,FVector(4,4,23)*Size);
                Wood(P+FVector(0,Face*79,194+Level*108)*Size,FVector(184,5,5)*Size);
            }
            Roof(P+FVector(0,0,247+Level*108)*Size,FVector(226,286,83)*Size);
        }
        for(int End:{-1,1})
            Wood(P+FVector(End*152,0,385)*Size,FVector(6,6,29)*Size);
    };
    for(FVector P:{FVector(West,Front-20,0),FVector(East,Front-20,0),FVector(West,Back,0),FVector(East,Back,0)}) Pavilion(P,.82f,false);
    Pavilion(FVector(Gate,Front-18,0),1.04f,true);
    // A second roofline gives the gatehouse a recognizable skyline.
    Pavilion(FVector(2130,-850,0),1.10f,false);
    // Streets and courtyards remain empty between these buildings.
    for(int Row=0;Row<3;++Row) for(int Col=0;Col<5;++Col)
    {
        const double X=1370+Col*330,Y=-455-Row*208;
        if(Col==2 || (Row==2 && Col==3)) continue;
        const FVector P(X,Y,Floor(X,Y));
        const float S=.72f+((Row+Col)%3)*.08f;
        Stone(P+FVector(0,0,25)*S,FVector(214,145,50)*S);
        Wood(P+FVector(0,0,78)*S,FVector(190,124,58)*S);
        for(int Side:{-1,1}) for(int End:{-1,1})
            Wood(P+FVector(End*78,Side*55,78)*S,FVector(7,7,72)*S);
        Roof(P+FVector(0,0,116)*S,FVector(182,260,78)*S);
        // Roof ridge caps are physical rather than texture stripes.
        Wood(P+FVector(0,0,141)*S,FVector(243,8,8)*S);
    }
}
