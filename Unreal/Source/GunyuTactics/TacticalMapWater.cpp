#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalMapMesh.h"
#include "Components/InstancedStaticMeshComponent.h"

// A continuous channel inside the existing river/bridge cells. The wet banks
// remain part of those same impassable river cells; navigation rules do not move.
void ATacticalField::BuildMapWater()
{
    using namespace frontline;
    TacticalMap::FSurface Water, Shore;
    TArray<int> Tiles;
    const auto& J=State->Journey;
    for (int Y=1;Y<StrategistJourney::Rows-1;++Y) for (int X=1;X<StrategistJourney::Columns-1;++X)
    {
        const int Tile=Campaign::At(X,Y);
        const auto T=J.world.tiles[Tile].terrain;
        if (T==Terrain::River || T==Terrain::Bridge) Tiles.Add(Tile);
    }
    auto ChannelCenter=[&](int Tile)
    {
        FVector P=TileLocation(Tile);
        if (J.mission==2) P.Y+=8*FMath::Sin(P.X*.004);
        return P;
    };
    // Consecutive east/west hexes share a narrower throat at their corners.
    // Leave room for the bend without painting water on a dry neighboring cell.
    auto RadiusAt=[&](FVector P) { return (J.mission==2 ? 44. : 48.)+4*FMath::Sin(P.Y*.008); };
    auto Tint=[&](int Tile)
    { return J.intelligence[0].visible[Tile] ? 1.f : .4f; };
    auto BankTint=[&](float Shade) { return FLinearColor(.19f,.15f,.085f)*Shade; };
    auto Cap=[&](FVector P,float Radius,float Shade,FVector Outward)
    {
        P.Z=-9;
        for (int Side=0;Side<24;++Side)
        {
            const double Facing=FMath::Atan2(Outward.Y,Outward.X);
            const double A=Facing-PI*.5+Side*PI/24,B=A+PI/24;
            const FVector U=P+FVector(FMath::Cos(A)*Radius,FMath::Sin(A)*Radius,0);
            const FVector V=P+FVector(FMath::Cos(B)*Radius,FMath::Sin(B)*Radius,0);
            const FLinearColor Deep=FLinearColor(.021f,.085f,.11f)*Shade,Edge=FLinearColor(.075f,.19f,.17f)*Shade;
            Water.Triangle(P,U,V,Deep,Edge,Edge);
            Shore.Line(U+FVector(0,0,.25),V+FVector(0,0,.25),8,BankTint(Shade));
        }
    };
    for (int I=0;I<Tiles.Num()-1;++I)
    {
        const FVector P=ChannelCenter(Tiles[I]),Q=ChannelCenter(Tiles[I+1]);
        if (FVector::DistSquared2D(P,Q)>250*250) continue;
        const FVector Previous=I>0 ? ChannelCenter(Tiles[I-1]) : P*2-Q;
        const FVector Next=I+2<Tiles.Num() ? ChannelCenter(Tiles[I+2]) : Q*2-P;
        // Average two segments at the ends too: odd-row hex centers zigzag.
        // Pointing an end tangent along just one diagonal spills water onto
        // the dry neighbor. The same averaging supports east/west channels.
        const FVector TangentP=I>0 ? (Q-Previous)*.5 : Tiles.Num()>2 ? (ChannelCenter(Tiles[2])-P)*.5 : Q-P;
        const FVector TangentQ=I+2<Tiles.Num() ? (Next-P)*.5 : Tiles.Num()>2 ? (Q-ChannelCenter(Tiles[Tiles.Num()-3]))*.5 : Q-P;
        auto Channel=[&](double T)
        {
            FVector C=FMath::CubicInterp(P,TangentP,Q,TangentQ,T); C.Z=-9;
            return C;
        };
        auto Edge=[&](double T,int Side)
        {
            const FVector C=Channel(T);
            const FVector D=(Channel(T+.002)-Channel(T-.002)).GetSafeNormal2D();
            const double Radius=RadiusAt(C);
            return C+FVector(-D.Y,D.X,0)*Side*Radius;
        };
        constexpr int Detail=12;
        for (int Step=0;Step<Detail;++Step)
        {
            const double T=Step/double(Detail),U=(Step+1)/double(Detail);
            const FVector A=Edge(T,-1),B=Edge(T,1),C=Edge(U,1),D=Edge(U,-1);
            const float Shade=FMath::Lerp(Tint(Tiles[I]),Tint(Tiles[I+1]),static_cast<float>(T));
            const float EndShade=FMath::Lerp(Tint(Tiles[I]),Tint(Tiles[I+1]),static_cast<float>(U));
            const FVector Mid=Channel(T),EndMid=Channel(U);
            const FLinearColor Shallow=FLinearColor(.075f,.19f,.17f)*Shade,EndShallow=FLinearColor(.075f,.19f,.17f)*EndShade;
            const FLinearColor Deep=FLinearColor(.021f,.085f,.11f)*Shade,EndDeep=FLinearColor(.021f,.085f,.11f)*EndShade;
            Water.Triangle(A,Mid,EndMid,Shallow,Deep,EndDeep);Water.Triangle(A,EndMid,D,Shallow,EndDeep,EndShallow);
            Water.Triangle(Mid,B,C,Deep,Shallow,EndShallow);Water.Triangle(Mid,C,EndMid,Deep,EndShallow,EndDeep);
            for (int Side : {-1,1})
            {
                FVector Start=Edge(T,Side),End=Edge(U,Side);
                Start.Z+=.25;End.Z+=.25;
                Shore.Line(Start,End,11,BankTint(Shade));
                if (J.world.tiles[Tiles[I]].terrain==Terrain::Bridge || J.world.tiles[Tiles[I+1]].terrain==Terrain::Bridge) continue;
                const FVector Across=(Start-Channel(T)).GetSafeNormal2D();
                if ((Step+I*3)%5==0)
                {
                    FVector R=Start+Across*(13+(Step%3)*4);R.Z=SurfaceHeight(R)+14;
                    Grass->AddInstance(FTransform(FRotator(0,I*57+Step*23,0),R,FVector(.065,.10,.31+(Step%3)*.08)));
                }
                // Small, half-buried gravel on the wet side of the bank. Keep
                // every pebble inside an existing river cell, away from bridges.
                if((Step==3 || Step==8) && (I+Side+Step)%2==0)
                    for(int Pebble=0;Pebble<3;++Pebble)
                    {
                        FVector R=Start+Across*(5+Pebble*4)+FVector(Across.Y,-Across.X,0)*(Pebble*5-5);
                        const auto Cell=hexgrid::Pick(R.X/200.,R.Y/200.);
                        if(!StrategistJourney::OnMap(Campaign::At(Cell.first,Cell.second)) || J.world.tiles[Campaign::At(Cell.first,Cell.second)].terrain!=Terrain::River) continue;
                        R.Z=SurfaceHeight(R)+2;
                        Rocks->AddInstance(FTransform(FRotator(9,I*29+Pebble*63,5),R,FVector(.10+Pebble*.02,.08+Pebble*.01,.08)));
                    }
                if (Side==1 && Step==5 && I%2==0)
                {
                    FVector R=Start+Across*7;R.Z=SurfaceHeight(R)+2;
                    Rocks->AddInstance(FTransform(FRotator(0,I*63,0),R,FVector(.24,.18,.15)));
                }
            }
        }
    }
    if (!Tiles.IsEmpty())
    {
        const FVector First=ChannelCenter(Tiles[0]),Last=ChannelCenter(Tiles.Last());
        Cap(First,RadiusAt(First),Tint(Tiles[0]),Tiles.Num()>1 ? First-ChannelCenter(Tiles[FMath::Min(2,Tiles.Num()-1)]) : FVector(0,-1,0));
        Cap(Last,RadiusAt(Last),Tint(Tiles.Last()),Tiles.Num()>1 ? Last-ChannelCenter(Tiles[FMath::Max(0,Tiles.Num()-3)]) : FVector(0,1,0));
    }
    Water.Submit(Ground,2); Shore.Submit(Ground,5);
    Ground->SetMaterial(5,TerrainMaterial);
}
