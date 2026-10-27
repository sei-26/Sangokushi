#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalBattlefieldStyle.h"

void ATacticalField::UpdateLandscapeTopology()
{
    const auto& J=State->Journey;
    TArray<int32> Snapshot;
    Snapshot.Append({J.mission,int(J.convoyRoute),J.HasConvoy() ? 1:0});
    for (int Y=1;Y<frontline::StrategistJourney::Rows-1;++Y)
        for (int X=1;X<frontline::StrategistJourney::Columns-1;++X)
            Snapshot.Add(static_cast<int32>(J.world.tiles[frontline::Campaign::At(X,Y)].terrain));
    if (Snapshot==State->LandscapeSnapshot) return;
    State->LandscapeSnapshot=MoveTemp(Snapshot);
    UpdateRoadTopology();
    State->LandscapePeaks.Reset(); State->LandscapeRidges.Reset(); State->LandscapeRivers.Reset();
    for (int Y=1;Y<frontline::StrategistJourney::Rows-1;++Y)
        for (int X=1;X<frontline::StrategistJourney::Columns-1;++X)
        {
            const int Tile=frontline::Campaign::At(X,Y);
            if (J.world.tiles[Tile].terrain==frontline::Terrain::River)
                State->LandscapeRivers.Add(TileLocation(Tile));
            if (J.world.tiles[Tile].terrain!=frontline::Terrain::Mountain) continue;
            State->LandscapePeaks.Add(TileLocation(Tile));
            for (const auto& [NX,NY] : hexgrid::Neighbors(X,Y))
            {
                if (NX<1 || NY<1 || NX>=frontline::StrategistJourney::Columns-1 || NY>=frontline::StrategistJourney::Rows-1) continue;
                const int Other=frontline::Campaign::At(NX,NY);
                if (Other>Tile && J.world.tiles[Other].terrain==frontline::Terrain::Mountain)
                    State->LandscapeRidges.Emplace(TileLocation(Tile),TileLocation(Other));
            }
        }
}

float ATacticalField::SurfaceHeight(FVector P) const
{
    const auto& J = State->Journey;
    float Height = -2 + 9 * FMath::Sin(P.X * .0031 + P.Y * .0017)
        + 5 * FMath::Cos(P.Y * .0047 - P.X * .0013);
    const auto Cell = hexgrid::Pick(P.X / 200., P.Y / 200.);
    // Connected crest lines share one broad slope instead of separate mounds.
    // The small public-terrain topology cache is rebuilt only when terrain changes.
    const auto Style=TacticalMap::BattlefieldStyle(J.mission);
    // Higher canyon ridges also need broader slopes, so the normal overview
    // can still see and select the playable hex centers behind their crest.
    const float RidgeHeight=Style.RidgeHeight;
    double Mountain=0;
    for (const FVector& Peak : State->LandscapePeaks)
        Mountain=FMath::Max(Mountain,RidgeHeight*FMath::Exp(-FVector::DistSquared2D(P,Peak)/Style.RidgeFalloff));
    for (const auto& Ridge : State->LandscapeRidges)
    {
        const FVector Delta=Ridge.Value-Ridge.Key;
        const double T=FMath::Clamp(FVector::DotProduct(P-Ridge.Key,Delta)/Delta.SizeSquared(),0.,1.);
        Mountain=FMath::Max(Mountain,RidgeHeight*FMath::Exp(-FVector::DistSquared2D(P,Ridge.Key+Delta*T)/Style.RidgeFalloff));
    }
    // Connected ridges retain the same tactical cells, while their crest
    // varies continuously instead of forming one level tabletop.
    const double Crest=.90+.12*FMath::Sin(P.X*.0043+P.Y*.0018)+.07*FMath::Cos(P.Y*.0067-P.X*.0013);
    Height+=Mountain*Crest;
    // Lower the bank continuously to the water along the actual hex outline.
    // An abrupt mountain/water cliff creates multiple ray intersections even
    // for an ordinary overhead camera and also leaves visible mesh cracks.
    double BankDistance=10000;
    for (const FVector& River : State->LandscapeRivers)
    {
        const FVector D=(P-River).GetAbs();
        const double Outside=FMath::Max(D.X,D.X*.5+D.Y*.866025403784)-100;
        BankDistance=FMath::Min(BankDistance,Outside);
    }
    Height=FMath::Lerp(-9.f,Height,static_cast<float>(FMath::SmoothStep(0.,200.,BankDistance)));
    // Flatten only the real public route, including the chosen woodland detour.
    Height *= FMath::SmoothStep(30.f,85.f,RoadDistance(P));
    // The water is a flat, lower surface. This affects art and picking only.
    if (Cell.first >= 1 && Cell.first < frontline::StrategistJourney::Columns - 1 && Cell.second >= 1 && Cell.second < frontline::StrategistJourney::Rows - 1)
    {
        const auto Terrain=J.world.tiles[frontline::Campaign::At(Cell.first,Cell.second)].terrain;
        if (Terrain==frontline::Terrain::River) return -9;
        if (Terrain==frontline::Terrain::Bridge) return 0;
    }
    return Height;
}

FVector ATacticalField::SurfaceLocation(int Tile) const
{
    FVector P = TileLocation(Tile); P.Z = SurfaceHeight(P); return P;
}

int ATacticalField::PickSurfaceTile(FVector Origin, FVector Direction) const
{
    if (Direction.Z >= -.001) return -1;
    // Intersect the same surface used by the grid, units and labels. The
    // modest terrain has no overhangs. Bisection also handles steep hills.
    double Near = FMath::Max(0., (512. - Origin.Z) / Direction.Z);
    double Far = (-128. - Origin.Z) / Direction.Z;
    if (Far <= Near) return -1;
    // Find the first crossing before refinement. Shorelines and bridge decks
    // can have more than one crossing; bisecting the whole depth range may
    // incorrectly choose a bank behind the pointed-at mountain.
    double Previous=Near;
    bool bCrossed=false;
    constexpr int Segments=128;
    for (int Segment=1;Segment<=Segments;++Segment)
    {
        const double T=FMath::Lerp(Near,Far,Segment/double(Segments));
        const FVector P=Origin+Direction*T;
        if (P.Z<=SurfaceHeight(P)) { Near=Previous; Far=T; bCrossed=true; break; }
        Previous=T;
    }
    if (!bCrossed) return -1;
    for (int Step = 0; Step < 22; ++Step)
    {
        const double T = (Near + Far) * .5;
        const FVector P = Origin + Direction * T;
        if (P.Z > SurfaceHeight(P)) Near = T; else Far = T;
    }
    const FVector P = Origin + Direction * ((Near + Far) * .5);
    const auto Cell = hexgrid::Pick(P.X / 200., P.Y / 200.);
    if (Cell.first < 1 || Cell.second < 1 || Cell.first >= frontline::StrategistJourney::Columns - 1 || Cell.second >= frontline::StrategistJourney::Rows - 1) return -1;
    return frontline::Campaign::At(Cell.first, Cell.second);
}
