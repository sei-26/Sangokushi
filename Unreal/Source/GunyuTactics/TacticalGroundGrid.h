#pragma once
#include "TacticalMapMesh.h"
#include "StrategistJourney.hpp"
namespace TacticalMap
{
    inline bool OnlyGroundRangeChanged(const TArray<int32>& Before,const TArray<int32>& After)
    {
        if(Before.IsEmpty() || Before.Num()!=After.Num()) return false;
        constexpr int Cells=(frontline::StrategistJourney::Columns-2)*(frontline::StrategistJourney::Rows-2);
        for(int I=0;I<Before.Num();++I)
        {
            const int Mask=I>=5 && I<5+Cells ? ~512:-1;
            if(((Before[I]^After[I])&Mask)!=0) return false;
        }
        return true;
    }
    inline FSurface GroundGrid(const frontline::StrategistJourney& J,const TArray<int32>& Reachable,
        TFunctionRef<FVector(int)> Location,TFunctionRef<float(FVector)> Height)
    {
        FSurface Grid;constexpr double Radius=115.470054;
        for(int Y=1;Y<frontline::StrategistJourney::Rows-1;++Y) for(int X=1;X<frontline::StrategistJourney::Columns-1;++X)
        {
            const int Tile=frontline::Campaign::At(X,Y);const auto Center=Location(Tile);
            const bool Water=J.world.tiles[Tile].terrain==frontline::Terrain::River;
            const bool Visible=J.intelligence[0].visible[Tile],Allowed=Reachable.Contains(Tile);
            auto Point=[&](FVector P) { P.Z=Water ? -10:Height(P);return P; };
            for(int Side=0;Side<6;++Side)
            {
                const float A=(30.f+Side*60.f)*PI/180,B=A+PI/3;
                const FVector U(FMath::Cos(A)*Radius,FMath::Sin(A)*Radius,0),V(FMath::Cos(B)*Radius,FMath::Sin(B)*Radius,0);
                for(int Segment=0;Segment<4;++Segment)
                    Grid.Line(Point(Center+FMath::Lerp(U,V,Segment/4.f))+FVector(0,0,Water ? 2.5:1.5),
                        Point(Center+FMath::Lerp(U,V,(Segment+1)/4.f))+FVector(0,0,Water ? 2.5:1.5),Allowed ? 3.f:.85f,
                        Allowed ? FLinearColor(.22f,.82f,.95f,.92f):FLinearColor(.48f,.51f,.40f,Visible ? .21f:.06f));
                if(Allowed) Grid.Triangle(Point(Center)+FVector(0,0,1),Point(Center+U)+FVector(0,0,1),Point(Center+V)+FVector(0,0,1),
                    FLinearColor(.05f,.5f,.7f,.13f),FLinearColor(.05f,.5f,.7f,.13f),FLinearColor(.05f,.5f,.7f,.13f));
            }
        }
        for(int Tile:{J.goal,J.Definition().secondGoal})
        {
            if(Tile<0) continue;
            auto P=Location(Tile);P.Z=Height(P);
            for(int Side=0;Side<6;++Side)
            {
                const float A=(30.f+Side*60.f)*PI/180,B=A+PI/3;
                Grid.Line(P+FVector(FMath::Cos(A)*103,FMath::Sin(A)*103,3),P+FVector(FMath::Cos(B)*103,FMath::Sin(B)*103,3),4,FLinearColor(1,.72f,.20f,.9f));
            }
        }
        return Grid;
    }
}
