#include "TacticalTrial.h"
#include "StrategistJourney.hpp"
#include "TacticalSceneryMesh.h"
#include "TacticalState.h"
#include "TacticalGroundDetail.h"
#include "TacticalBattlefieldStyle.h"
float ATacticalField::BackdropGroundHeight(FVector P) const
{
    const double Outside=FMath::Max(FMath::Max(200.-P.X,P.X-3200.),FMath::Max(200.-P.Y,P.Y-2500.));
    const double Rise=FMath::SmoothStep(800.,2200.,Outside);
    return SurfaceHeight(P)+Rise*(170+110*FMath::Sin(P.X*.0017)*FMath::Cos(P.Y*.002));
}
float ATacticalField::SceneryFloor(FVector P) const { return BackdropGroundHeight(P); }
void ATacticalField::BuildBackdropGround()
{
    TacticalMap::FIndexedScenery Land;TMap<FIntPoint,int32> Shared;
    const auto Style=TacticalMap::BattlefieldStyle(State->Journey.mission);
    constexpr double Radius=115.470054;
    auto Inside=[](int X,int Y)
    { return X>=-2 && Y>=-2 && X<frontline::StrategistJourney::Columns+3 && Y<frontline::StrategistJourney::Rows+3; };
    auto Vertex=[&](FVector P)
    {
        const FIntPoint Key(FMath::RoundToInt(P.X*100),FMath::RoundToInt(P.Y*100));
        if(const auto* Found=Shared.Find(Key)) return *Found;
        P.Z=BackdropGroundHeight(P);
        const FVector N(BackdropGroundHeight(P-FVector(3,0,0))-BackdropGroundHeight(P+FVector(3,0,0)),BackdropGroundHeight(P-FVector(0,3,0))-BackdropGroundHeight(P+FVector(0,3,0)),6);
        const FLinearColor C=TacticalMap::GroundTint(P,Style.Ground,Style.Road,RoadDistance(P))*.4f;
        const int32 I=Land.Vertex(P,N.GetSafeNormal(),C);Shared.Add(Key,I);return I;
    };
    for(int Y=-35;Y<36;++Y) for(int X=-32;X<53;++X)
    {
        if(Inside(X,Y)) continue;
        const auto XY=hexgrid::Center(X,Y);const FVector Center(XY.first*200,XY.second*200,0);
        const int32 A=Vertex(Center);
        for(int Side=0;Side<6;++Side)
        {
            const double Angle=(30+Side*60)*PI/180;
            const FVector U(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,0),V(FMath::Cos(Angle+PI/3)*Radius,FMath::Sin(Angle+PI/3)*Radius,0);
            const FVector Mid=Center+(U+V)*.5;const FVector Outside=Mid+(Mid-Center).GetSafeNormal2D()*4;
            const auto Cell=hexgrid::Pick(Outside.X/200.,Outside.Y/200.);
            const int Detail=Inside(Cell.first,Cell.second) ? 4:1;
            // Match the fine border edge exactly; coarse exterior edges remain shared.
            for(int I=0;I<Detail;++I)
                Land.Triangle(A,Vertex(Center+FMath::Lerp(U,V,I/double(Detail))),Vertex(Center+FMath::Lerp(U,V,(I+1)/double(Detail))));
        }
    }
    Land.Submit(Ground,6);Ground->SetMaterial(6,TerrainMaterial);
}
