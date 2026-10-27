#include "TacticalTrial.h"
#include "TacticalState.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "TacticalSceneryLayout.h"

bool ATacticalField::BackdropSmokeTest()
{
    const int SavedMission=FirstMission;const bool SavedCouncil=bStartWithCouncil,SavedPlayback=State->bNoPlayback;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_BACKDROP: FAIL %s"),Why);return V; };
    bStartWithCouncil=false;State->bNoPlayback=true;
    std::array<int,3> WallCounts{},RoofCounts{};std::array<float,3> BuildingTops{},MountainTops{};
    for(int Mission=0;Mission<3;++Mission)
    {
        FirstMission=Mission;Issue(ETacticalCommand::Restart);
        std::vector<std::uint8_t> Before,After;
        if(!Check(ExportSession(Before),TEXT("stable battle export"))) return false;
        BuildBackdropGround();BuildFortressBackdrop();BuildMountainBackdrop();BuildSceneryWater();
        if(!Check(ExportSession(After) && Before==After,TEXT("scenery cannot change terrain, armies, objectives or resources"))) return false;
        WallCounts[Mission]=CityWalls->GetInstanceCount();RoofCounts[Mission]=CityRoofs->GetInstanceCount();
        if(!Check(WallCounts[Mission]>10 && RoofCounts[Mission]>=10,TEXT("actual settlement structures are built"))) return false;
        for(auto* Part:{CityWalls.Get(),CityTimber.Get(),CityRoofs.Get()})
        {
            if(!Check(Part->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("backdrop cannot add movement blockers"))) return false;
            const FBox Bounds=Part->CalcBounds(Part->GetComponentTransform()).GetBox();
            if(!Check(Bounds.Max.Y<60,TEXT("architecture remains behind the first playable hex"))) return false;
            BuildingTops[Mission]=FMath::Max(BuildingTops[Mission],float(Bounds.Max.Z));
            for(int I=0;I<Part->GetInstanceCount();++I)
            { FTransform T;Part->GetInstanceTransform(I,T);if(!Check(!T.ContainsNaN() && T.GetScale3D().GetMin()>0,TEXT("finite, positive building transforms"))) return false; }
        }
        const auto* Mountains=Ground->GetProcMeshSection(4);
        if(!Check(Mountains && Mountains->ProcVertexBuffer.Num()>3000 && Mountains->ProcVertexBuffer.Num()<6000,TEXT("continuous layered mountains are actual geometry"))) return false;
        for(const auto& V:Mountains->ProcVertexBuffer)
        {
            if(!Check(!V.Position.ContainsNaN() && V.Position.Y<60,TEXT("mountains remain finite and outside playable cells"))) return false;
            MountainTops[Mission]=FMath::Max(MountainTops[Mission],float(V.Position.Z));
        }
        if(!Check(MountainTops[Mission]>700,TEXT("each backdrop has an actual skyline"))) return false;
        const auto* Water=Ground->GetProcMeshSection(7);
        if(Mission==1)
        {
            if(!Check(Water && Water->ProcVertexBuffer.Num()>500,TEXT("river continues into the port as actual water geometry"))) return false;
            for(const auto& V:Water->ProcVertexBuffer)
            {
                if(!Check(!V.Position.ContainsNaN() && V.Position.Y<=ATacticalField::TileLocation(frontline::Campaign::At(11,1)).Y+.01 && FMath::IsNearlyEqual(V.Position.Z,-9.f,.01f),TEXT("port water is flat and never extends downstream into a new tactical cell"))) return false;
                const auto Cell=hexgrid::Pick(V.Position.X/200.,V.Position.Y/200.);
                if(Cell.second>=1)
                    if(!Check(State->Journey.world.tiles[frontline::Campaign::At(Cell.first,Cell.second)].terrain==frontline::Terrain::River,TEXT("river join stays in the actual mouth cell"))) return false;
            }
            const FVector Bed(TacticalMap::PortRiverX(-600),-600,0);
            if(!Check(SurfaceHeight(Bed)<-10,TEXT("port water rests in a real scenery riverbed"))) return false;
        }
        else if(!Check(!Water || Water->ProcVertexBuffer.IsEmpty(),TEXT("port water cannot leak into another battle"))) return false;
        const auto* Surroundings=Ground->GetProcMeshSection(6);
        if(!Check(Surroundings && Surroundings->ProcVertexBuffer.Num()>12000 && Surroundings->ProcVertexBuffer.Num()<26000,TEXT("continuous surrounding ground"))) return false;
        for(FVector P:{FVector(2200,-300,0),FVector(-300,900,0),FVector(3300,900,0)})
            if(!Check(FMath::IsNearlyEqual(BackdropGroundHeight(P),SurfaceHeight(P),.01f),TEXT("outer field and backdrop heights join without a ledge"))) return false;
        TMap<FIntPoint,FColor> ApronColors;
        for(const auto& V:Ground->GetProcMeshSection(0)->ProcVertexBuffer)
            ApronColors.Add(FIntPoint(FMath::RoundToInt(V.Position.X*100),FMath::RoundToInt(V.Position.Y*100)),V.Color);
        int Matched=0;
        for(const auto& V:Surroundings->ProcVertexBuffer)
            if(const FColor* Color=ApronColors.Find(FIntPoint(FMath::RoundToInt(V.Position.X*100),FMath::RoundToInt(V.Position.Y*100))))
            {
                ++Matched;
                if(!Check(FMath::Abs(int(Color->R)-V.Color.R)<=2 && FMath::Abs(int(Color->G)-V.Color.G)<=2 && FMath::Abs(int(Color->B)-V.Color.B)<=2,TEXT("near and distant ground colors join"))) return false;
            }
        if(!Check(Matched>30,TEXT("actual shared border vertices are verified"))) return false;
        const auto Snapshot=State->BackdropSnapshot;
        SelectOfficer(1);Issue(ETacticalCommand::Move);Issue(ETacticalCommand::Cancel);
        if(!Check(CityWalls->GetInstanceCount()==WallCounts[Mission] && State->BackdropSnapshot==Snapshot,TEXT("selection retains static scenery"))) return false;
    }
    if(!Check(WallCounts[0]<WallCounts[1]/3 && RoofCounts[1]>RoofCounts[0] && RoofCounts[2]<RoofCounts[1],TEXT("village, port city and military pass have different compositions"))) return false;
    if(!Check(BuildingTops[2]>BuildingTops[0]+120 && MountainTops[2]>MountainTops[0]+250,TEXT("mountain pass has a taller defensive silhouette and mountain frame"))) return false;
    FirstMission=SavedMission;bStartWithCouncil=SavedCouncil;State->bNoPlayback=SavedPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_BACKDROP: PASS (three distinct settlements/skylines, connected port river, nonblocking scenery, immutable session, color seams and cache)"));return true;
}
