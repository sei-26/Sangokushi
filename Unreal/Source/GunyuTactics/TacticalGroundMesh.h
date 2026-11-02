#pragma once
#include "TacticalMapMesh.h"
namespace TacticalMap
{
    // Sample shared corners once. The simulation's analytic surface and the
    // independent grid remain unchanged; only the rendered tessellation changes.
    struct FGroundMesh
    {
        FSurface Surface;
        TMap<FIntVector,int32> Shared;
        int32 Vertex(FVector P,bool Water,TFunctionRef<float(FVector)> Height,TFunctionRef<FLinearColor(FVector)> Color)
        {
            const FIntVector Key(FMath::RoundToInt(P.X*100),FMath::RoundToInt(P.Y*100),Water ? 1:0);
            if(const int32* Found=Shared.Find(Key)) return *Found;
            P.Z=Water ? -10.f:Height(P);
            const FVector N=Water ? FVector::UpVector:FVector(
                Height(P-FVector(3,0,0))-Height(P+FVector(3,0,0)),
                Height(P-FVector(0,3,0))-Height(P+FVector(0,3,0)),6).GetSafeNormal();
            const int32 I=Surface.Vertices.Num();
            Surface.Vertices.Add(P);Surface.Normals.Add(N);Surface.Colors.Add(Color(P));
            Surface.UVs.Add(FVector2D(P.X,P.Y)/200);Shared.Add(Key,I);return I;
        }
        void Triangle(FVector P,FVector Q,FVector R,bool Water,TFunctionRef<float(FVector)> Height,TFunctionRef<FLinearColor(FVector)> Color)
        {
            int32 A=Vertex(P,Water,Height,Color),B=Vertex(Q,Water,Height,Color),C=Vertex(R,Water,Height,Color);
            if(FVector::CrossProduct(Surface.Vertices[B]-Surface.Vertices[A],Surface.Vertices[C]-Surface.Vertices[A]).Z<0) Swap(B,C);
            Surface.Indices.Append({A,C,B});
        }
    };
}
