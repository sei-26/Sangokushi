#include "TacticalTrial.h"
#include "TacticalState.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
bool ATacticalField::BackdropSmokeTest()
{
    const int SavedMission=FirstMission;const bool SavedCouncil=bStartWithCouncil,SavedPlayback=State->bNoPlayback;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_BACKDROP: FAIL %s"),Why);return V; };
    FirstMission=0;bStartWithCouncil=false;State->bNoPlayback=true;Issue(ETacticalCommand::Restart);
    std::vector<std::uint8_t> Before,After;
    if(!Check(ExportSession(Before),TEXT("stable battle export"))) return false;
    BuildBackdropGround();BuildFortressBackdrop();BuildMountainBackdrop();
    if(!Check(ExportSession(After) && Before==After,TEXT("scenery cannot change terrain, armies, objectives or resources"))) return false;
    if(!Check(CityWalls->GetInstanceCount()>200 && CityRoofs->GetInstanceCount()>20,TEXT("actual wall, battlements and layered gate roofs"))) return false;
    for(auto* Part:{CityWalls.Get(),CityTimber.Get(),CityRoofs.Get()})
    {
        if(!Check(Part->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("backdrop cannot add movement blockers"))) return false;
        const FBox Bounds=Part->CalcBounds(Part->GetComponentTransform()).GetBox();
        if(!Check(Bounds.Max.Y<60,TEXT("entire fortress remains behind the first playable hex"))) return false;
        for(int I=0;I<Part->GetInstanceCount();++I)
        { FTransform T;Part->GetInstanceTransform(I,T);if(!Check(!T.ContainsNaN() && T.GetScale3D().GetMin()>0,TEXT("finite, positive building transforms"))) return false; }
    }
    const auto* Mountains=Ground->GetProcMeshSection(4);float Highest=-10000;
    if(!Check(Mountains && Mountains->ProcVertexBuffer.Num()>3000 && Mountains->ProcVertexBuffer.Num()<6000,TEXT("continuous layered mountains are actual geometry"))) return false;
    for(const auto& V:Mountains->ProcVertexBuffer)
    {
        if(!Check(!V.Position.ContainsNaN() && V.Position.Y<60,TEXT("mountains remain finite and outside playable cells"))) return false;
        Highest=FMath::Max(Highest,float(V.Position.Z));
    }
    if(!Check(Highest>900,TEXT("distant peaks have a substantial skyline"))) return false;
    const auto* Surroundings=Ground->GetProcMeshSection(6);
    if(!Check(Surroundings && Surroundings->ProcVertexBuffer.Num()>12000 && Surroundings->ProcVertexBuffer.Num()<26000,TEXT("continuous ground replaces the overlapping underlay"))) return false;
    for(FVector P:{FVector(2200,-300,0),FVector(-300,900,0),FVector(3300,900,0)})
        if(!Check(FMath::IsNearlyEqual(BackdropGroundHeight(P),SurfaceHeight(P),.01f),TEXT("outer field and backdrop heights join without a ledge"))) return false;
    // The playable apron and distant ground must use the same palette/noise.
    // Their shared edge exposed a hard hex silhouette after only one was upgraded.
    TMap<FIntPoint,FColor> ApronColors;
    for(const auto& V:Ground->GetProcMeshSection(0)->ProcVertexBuffer)
        ApronColors.Add(FIntPoint(FMath::RoundToInt(V.Position.X*100),FMath::RoundToInt(V.Position.Y*100)),V.Color);
    int Matched=0;
    for(const auto& V:Surroundings->ProcVertexBuffer)
        if(const FColor* Color=ApronColors.Find(FIntPoint(FMath::RoundToInt(V.Position.X*100),FMath::RoundToInt(V.Position.Y*100))))
        {
            ++Matched;
            if(!Check(FMath::Abs(int(Color->R)-V.Color.R)<=2 && FMath::Abs(int(Color->G)-V.Color.G)<=2 && FMath::Abs(int(Color->B)-V.Color.B)<=2,
                TEXT("near and distant ground colors join at the shared edge"))) return false;
        }
    if(!Check(Matched>30,TEXT("color continuity is checked at actual shared border vertices"))) return false;
    const int Count=CityWalls->GetInstanceCount();const auto Snapshot=State->BackdropSnapshot;
    SelectOfficer(1);Issue(ETacticalCommand::Move);Issue(ETacticalCommand::Cancel);
    if(!Check(CityWalls->GetInstanceCount()==Count && State->BackdropSnapshot==Snapshot,TEXT("selection and reachable overlays retain static scenery"))) return false;
    FirstMission=SavedMission;bStartWithCouncil=SavedCouncil;State->bNoPlayback=SavedPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_BACKDROP: PASS (outside-board fortress, mountain skyline, no simulation mutation, nonblocking geometry, static cache)"));return true;
}
