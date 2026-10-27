#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalBattlefieldStyle.h"
#include "TacticalMapMesh.h"
using TacticalMap::FSurface;
#include "ProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

namespace
{
    constexpr float HexRadius = 115.470054f;
    float Grain(double X, double Y)
    {
        return FMath::Frac(FMath::Sin(X * 12.9898 + Y * 78.233) * 43758.5453);
    }

}

// Static scenery changes only with the observed board, never once per frame.
// Tile centers, occupancy, LOS and mouse picking retain the canonical hex grid.
void ATacticalField::BuildTerrain()
{
    const auto& J = State->Journey;
    const auto Style = TacticalMap::BattlefieldStyle(J.mission);
    TArray<int32> Snapshot;
    Snapshot.Append({J.mission, J.goal, J.Definition().secondGoal, J.HasConvoy() ? 1 : 0});
    for (int Y = 1; Y < frontline::StrategistJourney::Rows - 1; ++Y)
        for (int X = 1; X < frontline::StrategistJourney::Columns - 1; ++X)
        {
            const int Tile = frontline::Campaign::At(X, Y);
            Snapshot.Add(static_cast<int32>(J.world.tiles[Tile].terrain)
                | (J.intelligence[0].visible[Tile] ? 256 : 0) | (State->Reachable.Contains(Tile) ? 512 : 0));
        }
    for (const auto& City : J.world.cities) Snapshot.Append({City.tile,City.owner});
    // Selecting an officer or reordering a chain must not rebuild the landscape.
    // Exact snapshots avoid hash collisions; construction resets this cache for art edits.
    if (Snapshot == State->TerrainSnapshot) return;
    State->TerrainSnapshot = MoveTemp(Snapshot);
    FSurface Land, Grid;
    Trees->ClearInstances(); Trunks->ClearInstances(); Rocks->ClearInstances();
    Grass->ClearInstances(); Buildings->ClearInstances(); Roofs->ClearInstances();
    Houses->ClearInstances();
    auto Height = [this](FVector P) { return SurfaceHeight(P); };
    auto VisibilityShade = [&](FVector P)
    {
        const auto Cell=hexgrid::Pick(P.X/200.,P.Y/200.);
        float Seen=0,Total=0;
        for (int Y=Cell.second-1;Y<=Cell.second+1;++Y) for (int X=Cell.first-1;X<=Cell.first+1;++X)
        {
            if (X<1 || Y<1 || X>=frontline::StrategistJourney::Columns-1 || Y>=frontline::StrategistJourney::Rows-1) continue;
            const int Tile=frontline::Campaign::At(X,Y);
            const float Weight=FMath::Square(FMath::Max(0.f,1.f-static_cast<float>(FVector::DistSquared2D(P,TileLocation(Tile)))/50000.f));
            Total+=Weight; if (J.intelligence[0].visible[Tile]) Seen+=Weight;
        }
        return Total>0 ? .4f+.6f*Seen/Total : .4f;
    };
    auto ColorAt = [&](FVector P, FLinearColor Base, bool)
    {
        {
            const auto Cell=hexgrid::Pick(P.X/200.,P.Y/200.);
            FLinearColor Sum(0,0,0,0); float Total=0;
            for (int Y=Cell.second-1;Y<=Cell.second+1;++Y) for (int X=Cell.first-1;X<=Cell.first+1;++X)
            {
                if (X<1 || Y<1 || X>=frontline::StrategistJourney::Columns-1 || Y>=frontline::StrategistJourney::Rows-1) continue;
                const auto T=J.world.tiles[frontline::Campaign::At(X,Y)].terrain;
                FLinearColor C=Style.Ground;
                if (T==frontline::Terrain::Forest) C=Style.Forest;
                if (T==frontline::Terrain::Mountain) C=Style.Rock;
                if (T==frontline::Terrain::River) C=FLinearColor(.20f,.16f,.095f);
                const float W=FMath::Square(FMath::Max(0.f,1.f-static_cast<float>(FVector::DistSquared2D(P,TileLocation(frontline::Campaign::At(X,Y))))/65000.f));
                Sum+=C*W; Total+=W;
            }
            if (Total>0) Base=Sum/Total;
        }
        const float V = .93f + .10f * FMath::Sin(P.X * .013 + P.Y * .021);
        Base *= V;
        if (J.HasConvoy())
        {
            const FVector Start = TileLocation(frontline::Campaign::At(6, 11));
            const FVector End = TileLocation(frontline::Campaign::At(12, 11));
            if (P.X > Start.X - 140 && P.X < End.X + 160)
            {
                const float Road = 1 - FMath::SmoothStep(26., 72., FMath::Abs(P.Y - Start.Y));
                Base = FMath::Lerp(Base, Style.Road, Road);
            }
        }
        return Base * VisibilityShade(P);
    };
    for (int Y = 1; Y < frontline::StrategistJourney::Rows - 1; ++Y)
        for (int X = 1; X < frontline::StrategistJourney::Columns - 1; ++X)
        {
            const int Tile = frontline::Campaign::At(X, Y);
            const auto Terrain = J.world.tiles[Tile].terrain;
            const bool bVisible = J.intelligence[0].visible[Tile];
            const FVector Center = TileLocation(Tile);
            const bool bForest = Terrain == frontline::Terrain::Forest;
            const bool bRock = Terrain == frontline::Terrain::Mountain;
            const bool bWater = Terrain == frontline::Terrain::River;
            FSurface& Surface = Land;
            FLinearColor Base=Style.Ground;
            if (bForest) Base = Style.Forest;
            if (bRock) Base = Style.Rock;
            if (bWater) Base = FLinearColor(.20f, .16f, .095f);
            if (Terrain == frontline::Terrain::Bridge) Base = FLinearColor(.31f, .22f, .12f);
            auto Point = [&](FVector P) { P.Z = bWater ? -10.f : Height(P); return P; };
            for (int Side = 0; Side < 6; ++Side)
            {
                const float A = FMath::DegreesToRadians(30.f + Side * 60.f);
                const float B = FMath::DegreesToRadians(90.f + Side * 60.f);
                const FVector U(FMath::Cos(A) * HexRadius, FMath::Sin(A) * HexRadius, 0);
                const FVector V(FMath::Cos(B) * HexRadius, FMath::Sin(B) * HexRadius, 0);
                constexpr int Detail = 4;
                for (int I = 0; I < Detail; ++I) for (int K = 0; K < Detail - I; ++K)
                {
                    const FVector P = Point(Center + U * (I / float(Detail)) + V * (K / float(Detail)));
                    const FVector Q = Point(Center + U * ((I + 1) / float(Detail)) + V * (K / float(Detail)));
                    const FVector R = Point(Center + U * (I / float(Detail)) + V * ((K + 1) / float(Detail)));
                    Surface.Triangle(P, Q, R, ColorAt(P, Base, bVisible), ColorAt(Q, Base, bVisible), ColorAt(R, Base, bVisible));
                    if (I + K < Detail - 1)
                    {
                        const FVector S = Point(Center + U * ((I + 1) / float(Detail)) + V * ((K + 1) / float(Detail)));
                        Surface.Triangle(Q, S, R, ColorAt(Q, Base, bVisible), ColorAt(S, Base, bVisible), ColorAt(R, Base, bVisible));
                    }
                }
                // Thin inlaid grid, continuous ground underneath instead of separated plates.
                const bool bReachable = State->Reachable.Contains(Tile);
                for (int Segment=0;Segment<4;++Segment)
                    Grid.Line(Point(Center+FMath::Lerp(U,V,Segment/4.f))+FVector(0,0,bWater ? 2.5 : 1.5),
                        Point(Center+FMath::Lerp(U,V,(Segment+1)/4.f))+FVector(0,0,bWater ? 2.5 : 1.5), bReachable ? 3.f : .85f,
                        bReachable ? FLinearColor(.22f,.82f,.95f,.92f) : FLinearColor(.48f,.51f,.40f,bVisible ? .21f : .06f));
                if (bReachable)
                    Grid.Triangle(Point(Center) + FVector(0, 0, 1), Point(Center + U) + FVector(0, 0, 1), Point(Center + V) + FVector(0, 0, 1), FLinearColor(.05f, .5f, .7f, .13f), FLinearColor(.05f, .5f, .7f, .13f), FLinearColor(.05f, .5f, .7f, .13f));
            }
            if (bForest)
                for (int Tree = 0; Tree < 2 + int(Grain(X, Y) * 4); ++Tree)
                {
                    const float N = Grain(X * 5 + Tree, Y);
                    const float Angle = Tree * 2.4f + X + Y * 1.8f;
                    const FVector P = Center + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0) * (40 + N * 40);
                    bool bBridgeApproach=false;
                    for (const auto& [NX,NY] : hexgrid::Neighbors(X,Y))
                        if (NX>=1 && NY>=1 && NX<frontline::StrategistJourney::Columns-1 && NY<frontline::StrategistJourney::Rows-1 &&
                            J.world.tiles[frontline::Campaign::At(NX,NY)].terrain==frontline::Terrain::Bridge &&
                            FVector::DistSquared2D(P,TileLocation(frontline::Campaign::At(NX,NY)))<210*210) bBridgeApproach=true;
                    if (bBridgeApproach) continue;
                    const float Size = (.55f + N * .55f) * Style.TreeScale;
                    const FVector Root=P+FVector(0,0,Height(P));
                    const FRotator Twist(0,N*360,0);
                    Trunks->AddInstance(FTransform(Twist,Root+FVector(0,0,62)*Size,FVector(.10,.11,1.3)*Size));
                    // Forks join the same rooted trunk, below the moving leaf crowns.
                    for(int Branch=0;Branch<3;++Branch)
                    {
                        const FVector Start=Root+FVector(0,0,70+Branch*10)*Size;
                        const FVector End=Root+Twist.RotateVector(FVector(FMath::Cos(Branch*2.4)*34,FMath::Sin(Branch*2.4)*34,128+Branch*12)*Size);
                        const FVector Direction=End-Start;
                        Trunks->AddInstance(FTransform(FRotationMatrix::MakeFromZ(Direction).Rotator(),(Start+End)*.5,FVector(.038*Size,.038*Size,Direction.Size()/100)));
                    }
                    const float Breadth=.80f+Grain(X+Tree*7,Y+23)*.35f;
                    for (int Crown = 0; Crown < 3; ++Crown)
                        Trees->AddInstance(FTransform(FRotator(Crown*9+N*8,N*360,Crown*7-7),Root+Twist.RotateVector(FVector((Crown-1)*23,(Crown%2)*18,126+Crown*24)*Size),FVector(.95*Breadth,.85*Breadth,.66+N*.17)*Size));
                }
            if (bRock)
                for (int Rock = 0; Rock < 1; ++Rock)
                {
                    const float N = Grain(X + Rock * 9, Y * 3);
                    FVector P = Center + FVector(FMath::Cos(Rock * 2.4), FMath::Sin(Rock * 2.4), 0) * (45 + Rock * 19);
                    P.Z = Height(P) - 12;
                    Rocks->AddInstance(FTransform(FRotator(20 + N * 55, N * 360, 12), P, FVector(1.1 + N * .4, 1.4 + N * .6, .4 + N * .2)));
                }
            if (!bWater && !bRock)
                for (int Tuft = 0; Tuft < 12; ++Tuft)
                {
                    const float N = Grain(X * 17 + Tuft, Y * 7);
                    const FVector P = Center + FVector(FMath::Cos(Tuft * 2.4), FMath::Sin(Tuft * 2.4), 0) * (35 + N * 61);
                    // Keep the travelled road and the center of every formation clear.
                    const float RoadY = TileLocation(frontline::Campaign::At(6, 11)).Y;
                    if (J.HasConvoy() && FMath::Abs(P.Y - RoadY) < 55) continue;
                    Grass->AddInstance(FTransform(FRotator(0, N * 360, 0), P + FVector(0, 0, Height(P) + 8), FVector(.045, .09, .16 + N * .12)));
                }

        }
    if (J.HasConvoy())
    {
        // Subtle physical wheel ruts follow the shared, levelled road surface.
        const FVector Start=TileLocation(frontline::Campaign::At(6,11));
        const FVector End=TileLocation(frontline::Campaign::At(12,11));
        for (float X=Start.X-80;X<End.X+80;X+=40) for (int Side : {-1,1})
        {
            FVector A(X,Start.Y+Side*21,0), B(X+40,Start.Y+Side*21,0);
            A.Z=Height(A)+.7f; B.Z=Height(B)+.7f;
            const FLinearColor Rut=FLinearColor(.10f,.095f,.065f)*VisibilityShade(A);
            Land.Line(A,B,3.f,Rut);
        }
    }
    // The battlefield sits in a landscape, without exposing the edge of a board.
    // Continue the analytic ground across the actual jagged hex perimeter.
    // These extra hexes are scenery only: no grid, visibility or movement cells.
    for (int Y=-2;Y<frontline::StrategistJourney::Rows+3;++Y)
        for (int X=-2;X<frontline::StrategistJourney::Columns+3;++X)
        {
            if (X>=1 && Y>=1 && X<frontline::StrategistJourney::Columns-1 && Y<frontline::StrategistJourney::Rows-1) continue;
            const auto XY=hexgrid::Center(X,Y);
            const FVector Center(XY.first*200,XY.second*200,0);
            auto Point=[&](FVector P) { P.Z=Height(P); return P; };
            for (int Side=0;Side<6;++Side)
            {
                const float A=(30.f+Side*60.f)*PI/180, B=A+PI/3;
                const FVector U(FMath::Cos(A)*HexRadius,FMath::Sin(A)*HexRadius,0);
                const FVector V(FMath::Cos(B)*HexRadius,FMath::Sin(B)*HexRadius,0);
                for (int I=0;I<4;++I) for (int K=0;K<4-I;++K)
                {
                    const FVector P=Point(Center+U*(I/4.f)+V*(K/4.f)),Q=Point(Center+U*((I+1)/4.f)+V*(K/4.f)),R=Point(Center+U*(I/4.f)+V*((K+1)/4.f));
                    const FLinearColor Base=Style.Ground;
                    Land.Triangle(P,Q,R,ColorAt(P,Base,false),ColorAt(Q,Base,false),ColorAt(R,Base,false));
                    if (I+K<3)
                    {
                        const FVector S=Point(Center+U*((I+1)/4.f)+V*((K+1)/4.f));
                        Land.Triangle(Q,S,R,ColorAt(Q,Base,false),ColorAt(S,Base,false),ColorAt(R,Base,false));
                    }
                }
            }
        }
    auto Gateway = [&](int Tile)
    {
        if (Tile < 0) return;
        const FVector P = SurfaceLocation(Tile);
        for (int Side = 0; Side < 6; ++Side)
        {
            const float A = FMath::DegreesToRadians(30.f + Side * 60.f), B = A + PI / 3;
            Grid.Line(P + FVector(FMath::Cos(A) * 103, FMath::Sin(A) * 103, 3), P + FVector(FMath::Cos(B) * 103, FMath::Sin(B) * 103, 3), 4, FLinearColor(1, .72f, .20f, .9f));
        }
    };
    Gateway(J.goal); Gateway(J.Definition().secondGoal);
    BuildMapScenery();
    // Smooth the ridge lighting without changing its shared selectable surface.
    for (int I=0;I<Land.Vertices.Num();++I)
    {
        const FVector P=Land.Vertices[I];
        if (P.X<200 || P.X>3200 || P.Y<200 || P.Y>2500 || FMath::Abs(P.Z-Height(P))>1) continue;
        const double DX=Height(P+FVector(3,0,0))-Height(P-FVector(3,0,0));
        const double DY=Height(P+FVector(0,3,0))-Height(P-FVector(0,3,0));
        Land.Normals[I]=FVector(-DX,-DY,6).GetSafeNormal();
    }
    Land.Submit(Ground, 0); Grid.Submit(Ground, 1);
    Ground->ClearMeshSection(2);
    BuildMapWater();
    Ground->SetMaterial(0, TerrainMaterial ? TerrainMaterial.Get() : LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Art/Materials/M_Field.M_Field")));
    Ground->SetMaterial(1, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Art/Materials/M_BattleFX.M_BattleFX")));
    Ground->SetMaterial(2, WaterMaterial ? WaterMaterial.Get() : LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Art/Materials/M_BattleWater.M_BattleWater")));
}
