#pragma once
#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"

namespace TacticalMap
{
    struct FSurface
    {
        TArray<FVector> Vertices, Normals;
        TArray<int32> Indices;
        TArray<FVector2D> UVs;
        TArray<FLinearColor> Colors;
        void Triangle(FVector A, FVector B, FVector C, FLinearColor CA, FLinearColor CB, FLinearColor CC)
        {
            // Lit road ribbons and terrain fans must both face upward. The
            // old line helper happened to be unlit, hiding reversed normals.
            if (FVector::CrossProduct(B-A,C-A).Z<0) { Swap(B,C); Swap(CB,CC); }
            const int I = Vertices.Num();
            Vertices.Append({A, B, C}); Indices.Append({I, I + 2, I + 1});
            const FVector Normal = FVector::CrossProduct(B - A, C - A).GetSafeNormal();
            for (const auto& P : {A, B, C}) { Normals.Add(Normal); UVs.Add(FVector2D(P.X, P.Y) / 200); }
            Colors.Append({CA, CB, CC});
        }
        void Line(FVector A, FVector B, float Width, FLinearColor Color)
        {
            const FVector Side = FVector::CrossProduct((B - A).GetSafeNormal(), FVector::UpVector) * Width * .5;
            Triangle(A - Side, B - Side, B + Side, Color, Color, Color);
            Triangle(A - Side, B + Side, A + Side, Color, Color, Color);
        }
        void Submit(UProceduralMeshComponent* Mesh, int Section)
        {
            Mesh->CreateMeshSection_LinearColor(Section, Vertices, Indices, Normals, UVs, Colors, {}, false);
        }
    };
}
