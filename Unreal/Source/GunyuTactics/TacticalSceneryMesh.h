#pragma once
#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
namespace TacticalMap
{
    // Shared vertices for static scenery: no duplicated triangle soup in maps.
    struct FIndexedScenery
    {
        TArray<FVector> Vertices,Normals;
        TArray<FVector2D> UVs;
        TArray<FLinearColor> Colors;
        TArray<int32> Indices;
        int32 Vertex(FVector P,FVector N,FLinearColor C)
        { const int32 I=Vertices.Num();Vertices.Add(P);Normals.Add(N);UVs.Add(FVector2D(P.X,P.Y)/200);Colors.Add(C);return I; }
        void Triangle(int32 A,int32 B,int32 C)
        { if(FVector::CrossProduct(Vertices[B]-Vertices[A],Vertices[C]-Vertices[A]).Z<0) Swap(B,C);Indices.Append({A,C,B}); }
        void Submit(UProceduralMeshComponent* Mesh,int32 Section)
        { Mesh->CreateMeshSection_LinearColor(Section,Vertices,Indices,Normals,UVs,Colors,{},false,false); }
    };
}
