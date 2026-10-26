#include "TacticalTrial.h"
#include "TacticalState.h"
#include "ProceduralMeshComponent.h"
#include "ProfilingDebugging/CsvProfiler.h"

// Real curved cloth, edging and original insignia. These are presentation
// identifiers, not reproductions of historical banners or gameplay objects.
void ATacticalField::RenderStandards()
{
    CSV_SCOPED_TIMING_STAT_GLOBAL(GunyuStandards);
    TArray<FVector> Points, Normals;
    TArray<int32> Indices;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> Colors;
    auto Triangle = [&](FVector A, FVector B, FVector C, FLinearColor Tint)
    {
        const int Start = Points.Num();
        Points.Append({A,B,C}); Indices.Append({Start,Start+2,Start+1});
        const FVector Normal = FVector::CrossProduct(B-A,C-A).GetSafeNormal();
        for (int I=0; I<3; ++I) { Normals.Add(Normal); UVs.Add(FVector2D::ZeroVector); Colors.Add(Tint); }
    };
    for (const auto& Unit : State->Units)
    {
        if (Unit.Id < 0 || Unit.Troops <= 0) continue;
        if (Unit.Id == frontline::StrategistJourney::ConvoyArmy)
        {
            FVector Facing=FVector::ForwardVector;
            const auto* Beat=CurrentBeat();
            if (Beat && Beat->Before.IsValidIndex(Unit.Id) && Beat->After.IsValidIndex(Unit.Id))
            {
                const FVector Travel=Beat->After[Unit.Id].Position-Beat->Before[Unit.Id].Position;
                if (Travel.SizeSquared()>1) Facing=Travel.GetSafeNormal();
            }
            const FRotator Heading=Facing.Rotation();
            auto Canopy=[&](float U,float V)
            {
                const float Angle=(V-.5f)*PI;
                return Unit.Position+Heading.RotateVector(FVector(-50+U*100,34*FMath::Sin(Angle),61+27*FMath::Cos(Angle)));
            };
            for (int X=0;X<10;++X) for (int Y=0;Y<12;++Y)
            {
                const float U=X/10.f,V=Y/12.f;
                const FLinearColor Tint=X==0 || X==9 ? FLinearColor(.035f,.14f,.17f) : FLinearColor(.43f,.34f,.20f)*(.94f+.06f*FMath::Sin(X*3.f));
                const FVector A=Canopy(U,V),B=Canopy(U+.1f,V),C=Canopy(U+.1f,V+1/12.f),D=Canopy(U,V+1/12.f);
                Triangle(A,B,C,Tint); Triangle(A,C,D,Tint);
            }
            continue;
        }
        const float Time = State->VisualTime;
        const FLinearColor Cloth = Unit.bEnemy ? FLinearColor(.30f,.035f,.018f)
            : Unit.Id == 1 ? FLinearColor(.018f,.16f,.09f) : Unit.Id == 2 ? FLinearColor(.025f,.09f,.12f) : FLinearColor(.022f,.12f,.21f);
        const FLinearColor Gold(.72f,.46f,.12f);
        auto ClothPoint = [&](FVector2D UV, float Layer = 0.f)
        {
            const float U=UV.X, V=UV.Y;
            const float Wave = U * (5 * FMath::Sin(U*7 - Time*3.4f + Unit.Id) + 2 * FMath::Sin(V*9 + Time*2));
            // Pole-side edge stays pinned; the free hem and split tail move.
            const float Tail = Unit.Id == 2 ? FMath::Max(0.f,1-FMath::Abs(U-.5f)*4) * FMath::Pow(V,8) * 15 : 0;
            return Unit.Position + FVector(-34 + U*88, Wave + Layer, 218 - V*91 + Tail + U*FMath::Sin(Time*2+Unit.Id)*2);
        };
        auto Quad = [&](FVector2D A,FVector2D B,FVector2D C,FVector2D D,FLinearColor Tint,float Layer=0.f)
        {
            Triangle(ClothPoint(A,Layer),ClothPoint(B,Layer),ClothPoint(C,Layer),Tint);
            Triangle(ClothPoint(A,Layer),ClothPoint(C,Layer),ClothPoint(D,Layer),Tint);
        };
        for (int Y=0;Y<10;++Y) for (int X=0;X<12;++X)
        {
            const float U=X/12.f,V=Y/10.f;
            const bool Edge = X==0 || X==11 || Y==0 || Y==9;
            Quad({U,V},{U+1/12.f,V},{U+1/12.f,V+.1f},{U,V+.1f},Edge ? Gold*.65f : Cloth*(.92f+.08f*FMath::Sin(X+Y*3.f)));
        }
        auto Line = [&](FVector2D A,FVector2D B,float Width)
        {
            const FVector2D Direction=(B-A).GetSafeNormal(), Side(-Direction.Y*Width,Direction.X*Width);
            Quad(A-Side,A+Side,B+Side,B-Side,Gold,1.2f);
        };
        auto Ring = [&](FVector2D Center,float Radius,float Width)
        {
            for (int I=0;I<28;++I)
            {
                const float A=I*2*PI/28,B=(I+1)*2*PI/28;
                const FVector2D U(FMath::Cos(A),FMath::Sin(A)),V(FMath::Cos(B),FMath::Sin(B));
                Quad(Center+U*Radius,Center+V*Radius,Center+V*(Radius+Width),Center+U*(Radius+Width),Gold,1.2f);
            }
        };
        if (Unit.bEnemy)
        {
            Ring({.5f,.46f},.17f,.025f);
            for (int I=0;I<8;++I) { const float A=I*PI/4; const FVector2D D(FMath::Cos(A),FMath::Sin(A)); Line(FVector2D(.5f,.46f)+D*.23f,FVector2D(.5f,.46f)+D*.29f,.012f); }
        }
        else if (Unit.Id==0)
        { Ring({.39f,.38f},.12f,.026f); Ring({.61f,.38f},.12f,.026f); Ring({.5f,.58f},.12f,.026f); }
        else if (Unit.Id==1)
        { Line({.37f,.72f},{.62f,.23f},.02f); Ring({.60f,.31f},.105f,.035f); Line({.51f,.40f},{.70f,.30f},.022f); }
        else
        { Line({.31f,.69f},{.68f,.24f},.025f); Line({.69f,.69f},{.32f,.24f},.025f); Line({.25f,.30f},{.43f,.30f},.018f); Line({.57f,.30f},{.75f,.30f},.018f); }
    }
    const auto* Section=Standards->GetProcMeshSection(0);
    // Every triangle owns three consecutive vertices. At a stable vertex count
    // the index layout is identical: update cloth without recreating its proxy.
    if (Section && Section->ProcVertexBuffer.Num()==Points.Num() && Section->ProcIndexBuffer.Num()==Indices.Num())
        Standards->UpdateMeshSection_LinearColor(0,Points,Normals,UVs,Colors,{},false);
    else Standards->CreateMeshSection_LinearColor(0,Points,Indices,Normals,UVs,Colors,{},false);
}
