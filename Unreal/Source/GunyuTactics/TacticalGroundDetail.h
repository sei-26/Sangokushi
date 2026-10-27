#pragma once
#include "CoreMinimal.h"

namespace TacticalMap
{
    // Continuous world-space variation: patches join across hex and mesh edges.
    inline float GroundNoise(FVector P,float Scale)
    {
        const double X=P.X/Scale,Y=P.Y/Scale;
        const int IX=FMath::FloorToInt(X),IY=FMath::FloorToInt(Y);
        const double U=FMath::SmoothStep(0.,1.,X-IX),V=FMath::SmoothStep(0.,1.,Y-IY);
        auto Sample=[](int A,int B)
        { return FMath::Frac(FMath::Sin(A*127.1+B*311.7)*43758.5453)*.5+.5; };
        return FMath::Lerp(FMath::Lerp(Sample(IX,IY),Sample(IX+1,IY),U),
            FMath::Lerp(Sample(IX,IY+1),Sample(IX+1,IY+1),U),V);
    }
    inline FLinearColor GroundTint(FVector P,FLinearColor Base,FLinearColor RoadColor,float RoadDistance)
    {
        const float Patch=GroundNoise(P,370.f),Detail=GroundNoise(P,73.f);
        const float Vegetation=FMath::SmoothStep(.30f,.72f,Patch)*(1-FMath::SmoothStep(28.f,110.f,float(P.Z)));
        Base=FMath::Lerp(Base,Base*FLinearColor(.80f,1.28f,.78f),Vegetation*.65f);
        Base*=.84f+.22f*Patch+.12f*Detail;
        const float Edge=58.f+(Patch-.5f)*16.f;
        const float Road=1-FMath::SmoothStep(25.f,Edge,RoadDistance);
        return FMath::Lerp(Base,RoadColor*(.86f+Detail*.20f),Road*.92f);
    }
    inline double SegmentDistance2D(FVector P,FVector A,FVector B)
    {
        const FVector D=B-A;
        const double Length=D.SizeSquared2D();
        if(Length<.001) return FVector::Dist2D(P,A);
        const double T=FMath::Clamp(((P.X-A.X)*D.X+(P.Y-A.Y)*D.Y)/Length,0.,1.);
        return FVector::Dist2D(P,A+D*T);
    }
}
