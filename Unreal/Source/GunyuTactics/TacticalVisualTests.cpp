#include "TacticalTrial.h"
#include "TacticalState.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"

bool ATacticalField::VisualGeometrySmokeTest()
{
    using namespace frontline;
    const int SavedMission=FirstMission, SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil, SavedNoPlayback=State->bNoPlayback;
    bStartWithCouncil=false; State->bNoPlayback=true; EnemyPolicy=0;
    auto Check=[](bool Value,const TCHAR* Name)
    {
        if (!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_VISUAL_GEOMETRY: FAIL: %s"),Name);
        return Value;
    };
    for (int Mission=0; Mission<3; ++Mission)
    {
        FirstMission=Mission; Issue(ETacticalCommand::Restart);
        if (Mission==1)
        {
            const auto* Channel=Ground->GetProcMeshSection(2);
            if (!Check(Channel && Channel->ProcVertexBuffer.Num()>0,TEXT("escort mission has a continuous river mesh"))) return false;
            uint8 MinGreen=255,MaxGreen=0;
            for(const auto& V:Channel->ProcVertexBuffer) { MinGreen=FMath::Min(MinGreen,V.Color.G);MaxGreen=FMath::Max(MaxGreen,V.Color.G); }
            if(!Check(MaxGreen>=MinGreen+8,TEXT("river mesh carries shallow-to-deep color variation"))) return false;
            for (const auto& Vertex : Channel->ProcVertexBuffer)
            {
                const auto Cell=hexgrid::Pick(Vertex.Position.X/200.,Vertex.Position.Y/200.);
                const auto Terrain=State->Journey.world.tiles[Campaign::At(Cell.first,Cell.second)].terrain;
                if (Terrain!=Terrain::River && Terrain!=Terrain::Bridge)
                    UE_LOG(LogTemp,Error,TEXT("GUNYU_WATER_CONTAINMENT: point=%s cell=(%d,%d)"),*Vertex.Position.ToString(),Cell.first,Cell.second);
                if (!Check(Terrain==Terrain::River || Terrain==Terrain::Bridge,TEXT("curved water stays inside actual river or bridge cells"))) return false;
                if (!Check(FMath::IsNearlyEqual(Vertex.Position.Z,-9.f,.01f),TEXT("water stays at the shared flat river elevation"))) return false;
            }
        }
        if(!Check(Trees->GetInstanceCount()>0 && Trunks->GetInstanceCount()*3==Trees->GetInstanceCount()*4,TEXT("every crown group has a rooted trunk and three branches"))) return false;
        if(!Check(Trunks->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("branch detail cannot obstruct movement"))) return false;
        if (!Check(Spearheads->GetStaticMesh() && Standards->GetMaterial(0),TEXT("real spear and banner assets are loaded"))) return false;
        if (!Check(Palisades->GetStaticMesh() && Watchtowers->GetStaticMesh() && Tents->GetStaticMesh() && SupplyCrates->GetStaticMesh(),
            TEXT("editable fortification mesh references are loaded"))) return false;
        if (!Check(Palisades->GetInstanceCount()>40 && Watchtowers->GetInstanceCount()==2 && Tents->GetInstanceCount()==2,
            TEXT("both public camp sites have actual instanced fortifications"))) return false;
        if (!Check(Palisades->GetCollisionEnabled()==ECollisionEnabled::NoCollision && Stonework->GetCollisionEnabled()==ECollisionEnabled::NoCollision,
            TEXT("decorative buildings cannot introduce hidden movement blockers"))) return false;
        const auto* Banner=Standards->GetProcMeshSection(0);
        if (!Check(Banner && Banner->ProcVertexBuffer.Num()>0,TEXT("visible troops have geometric standards"))) return false;
        auto StableColors=[&](UProceduralMeshComponent* Mesh,TFunction<void()> Draw)
        {
            Mesh->ClearMeshSection(0); Draw();
            TArray<FColor> Before;
            for (const auto& Vertex : Mesh->GetProcMeshSection(0)->ProcVertexBuffer) Before.Add(Vertex.Color);
            Draw();
            const auto& After=Mesh->GetProcMeshSection(0)->ProcVertexBuffer;
            if (Before.Num()!=After.Num()) return false;
            for (int I=0;I<Before.Num();++I) if (Before[I]!=After[I].Color) return false;
            return true;
        };
        if (!Check(StableColors(Standards,[this] { RenderStandards(); }) && StableColors(Effects,[this] { RenderEffects(); }),TEXT("cached updates preserve exact banner and threat colors"))) return false;
        Banner=Standards->GetProcMeshSection(0);
        for (int I=0;I<Banner->ProcIndexBuffer.Num();I+=3)
        {
            const auto& A=Banner->ProcVertexBuffer[Banner->ProcIndexBuffer[I]];
            const auto& B=Banner->ProcVertexBuffer[Banner->ProcIndexBuffer[I+1]];
            const auto& C=Banner->ProcVertexBuffer[Banner->ProcIndexBuffer[I+2]];
            if (!Check(FVector::DotProduct(A.Normal,FVector::CrossProduct(B.Position-A.Position,C.Position-A.Position))<0,TEXT("clockwise cloth faces have matching lit normals"))) return false;
        }
        for (int Y=1;Y<StrategistJourney::Rows-1;++Y) for (int X=1;X<StrategistJourney::Columns-1;++X)
        {
            const int Tile=Campaign::At(X,Y);
            const FVector Point=SurfaceLocation(Tile);
            for (FVector Offset : {FVector(0,1600,2200),FVector(900,900,1800)})
            {
                const FVector Origin=Point+Offset;
                const int Picked=PickSurfaceTile(Origin,-Offset.GetSafeNormal());
                if (Picked!=Tile)
                    UE_LOG(LogTemp,Error,TEXT("GUNYU_SURFACE_PICK: mission=%d tile=(%d,%d) picked=(%d,%d) height=%.2f camera=%s"),Mission,X,Y,Picked%frontline::Width,Picked/frontline::Width,Point.Z,*Offset.ToString());
                if (!Check(Picked==Tile,TEXT("camera ray selects rendered hex center on each terrain"))) return false;
            }
        }
        for (const auto& Unit : State->Units)
            if (Unit.Id>=0 && Unit.Troops>0)
                if (!Check(FMath::IsNearlyEqual(Unit.Position.Z,static_cast<double>(SurfaceHeight(Unit.Position)),.01),TEXT("observed soldiers stand on the actual surface"))) return false;
        if (!Check(PickSurfaceTile(FVector(0,0,1000),FVector::UpVector)==-1,TEXT("upward rays cannot select a tile"))) return false;
    }
    FirstMission=SavedMission; EnemyPolicy=SavedPolicy; bStartWithCouncil=SavedCouncil; State->bNoPlayback=SavedNoPlayback;
    Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_VISUAL_GEOMETRY: PASS (three missions, 1008 surface ray selections, shared unit elevation, real spear mesh and curved banners)"));
    return true;
}
