#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalGroundGrid.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "TacticalCrossingLandscape.h"
#include "TacticalPortReadingQA.h"

bool ATacticalField::MapDetailSmokeTest()
{
    using namespace frontline;
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedPlayback=State->bNoPlayback,SavedRealtime=bRealtimeCommand;
    const bool SavedLegacy=bLegacyJourney,SavedGrand=bGrandHistoricalBattles,SavedPause=bPauseOnBattleAlert;
    auto Check=[](bool Value,const TCHAR* Why)
    { if(!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_MAP_DETAIL: FAIL %s"),Why);return Value; };
    FirstMission=0;EnemyPolicy=1;bStartWithCouncil=true;bRealtimeCommand=false;State->bNoPlayback=true;
    Issue(ETacticalCommand::Restart);
    for(int I=0;I<3;++I) Issue(ETacticalCommand::Dialogue);
    Issue(ETacticalCommand::PromiseRescue);Issue(ETacticalCommand::RouteSouthernWood);Issue(ETacticalCommand::Deploy);
    const auto& J=State->Journey;
    if(!Check(J.stage==JourneyStage::Battle && J.convoyRoute==JourneyConvoyRoute::SouthernWood,TEXT("public controls deploy the woodland route"))) return false;
    const auto Stable=State->TerrainSnapshot;
    auto Range=Stable;Range[5]^=512;
    if(!Check(TacticalMap::OnlyGroundRangeChanged(Stable,Range),TEXT("range colors can reuse static terrain"))) return false;
    auto Fog=Range;Fog[5]^=256;
    auto City=Range;City.Last()^=512;
    if(!Check(!TacticalMap::OnlyGroundRangeChanged(Stable,Fog) && !TacticalMap::OnlyGroundRangeChanged(Stable,City),TEXT("fog and city metadata still invalidate the static scene"))) return false;
    // A diagnostic off-board instance proves highlight updates do not clear
    // and recreate the scenery; remove it before the normal geometry checks.
    const int Marker=Trees->AddInstance(FTransform(FVector(0,0,-10000)));
    Issue(ETacticalCommand::Move);
    if(!Check(!State->Reachable.IsEmpty() && Trees->GetInstanceCount()==Marker+1,TEXT("actual move highlights retain existing scenery instances"))) return false;
    Trees->RemoveInstance(Marker);SelectOfficer(0);
    for(int Tile:J.ConvoyPath())
    {
        const FVector P=TileLocation(Tile);
        if(!Check(RoadDistance(P)<.01f && FMath::Abs(SurfaceHeight(P))<.01f,TEXT("road and graded ground pass through every actual convoy checkpoint"))) return false;
        const FVector Offset(900,900,1800);
        if(!Check(PickSurfaceTile(SurfaceLocation(Tile)+Offset,-Offset.GetSafeNormal())==Tile,TEXT("the woodland road remains selectable"))) return false;
    }
    if(!Check(RoadDistance(TileLocation(Campaign::At(9,11)))>85,TEXT("no imaginary straight road replaces the chosen detour"))) return false;
    const auto SouthernSnapshot=State->LandscapeSnapshot;
    const auto SouthernRoad=State->LandscapeRoad;
    std::vector<std::uint8_t> Before,After;
    if(!Check(ExportSession(Before),TEXT("stable battle export"))) return false;
    UpdateRoadTopology();BuildTerrain();SelectOfficer(1);SelectOfficer(0);
    if(!Check(ExportSession(After) && Before==After,TEXT("ground detail and selection cannot alter the session"))) return false;
    if(!Check(State->LandscapeRoad==SouthernRoad && State->LandscapeSnapshot==SouthernSnapshot,TEXT("selection retains the public road cache"))) return false;
    Issue(ETacticalCommand::Withdraw);Issue(ETacticalCommand::Withdraw);
    Issue(ETacticalCommand::Retry);Issue(ETacticalCommand::RouteRoad);Issue(ETacticalCommand::Deploy);
    if(!Check(State->Journey.convoyRoute==JourneyConvoyRoute::Road && State->LandscapeSnapshot!=SouthernSnapshot,
        TEXT("changing only the route invalidates the landscape cache"))) return false;
    if(!Check(RoadDistance(TileLocation(Campaign::At(9,11)))<.01f && RoadDistance(TileLocation(Campaign::At(9,12)))>85,
        TEXT("road, ruts and clearings switch back to the actual street"))) return false;
    // The ordinary breakout must keep its terrain/navigation semantics while gaining readable scenery.
    Issue(ETacticalCommand::Restart);
    for(int I=0;I<3;++I) Issue(ETacticalCommand::Dialogue);
    Issue(ETacticalCommand::PromiseRoad);Issue(ETacticalCommand::Deploy);
    if(!Check(State->Journey.mission==0 && !State->Journey.HasConvoy(),TEXT("ordinary retreat is a separate composition"))) return false;
    if(!Check(RoadDistance(TileLocation(State->Journey.goal))<.01f && RoadDistance(TileLocation(Campaign::At(8,9)))>100,
        TEXT("the street reaches the exit without replacing the ambush wood with a road"))) return false;
    for(int Tile:{Campaign::At(8,9),Campaign::At(9,10),Campaign::At(9,11),State->Journey.goal})
    {
        const FVector Offset(420,380,1500);
        if(!Check(PickSurfaceTile(SurfaceLocation(Tile)+Offset,-Offset.GetSafeNormal())==Tile,TEXT("forest, street and open exit keep their real selectable cells"))) return false;
    }
    if(!Check(ExportSession(Before),TEXT("ordinary retreat exports before visual rebuild"))) return false;
    const auto RetreatRoad=State->LandscapeRoad;
    State->TerrainSnapshot.Reset();UpdateRoadTopology();BuildTerrain();SelectOfficer(1);SelectOfficer(0);
    if(!Check(ExportSession(After) && Before==After && State->LandscapeRoad==RetreatRoad,
        TEXT("retreat art, rebuilt landmarks and selection do not move units or change costs/history"))) return false;
    for(bool North:{false,true})
    {
        if(!Check(PreparePortReadingPreview(this,1,North),TEXT("actual initial victory and reply select each port battlefield"))) return false;
        const auto& Port=State->Journey;const auto Bridges=gunyucrossing::Bridges(Port);
        if(!Check(Bridges.Num()==2,TEXT("port has two real crossings"))) return false;
        for(int Tile:Bridges)
        {
            if(!Check(RoadDistance(TileLocation(Tile))<.01f && FMath::IsNearlyZero(SurfaceHeight(TileLocation(Tile))),
                TEXT("both port roads meet the real level bridge decks"))) return false;
            for(int At:{Tile-1,Tile,Tile+1})
            {
                const FVector Offset(380,340,1500);
                if(!Check(PickSurfaceTile(SurfaceLocation(At)+Offset,-Offset.GetSafeNormal())==At,TEXT("each bank and bridge remains selectable"))) return false;
            }
            const auto* Mesh=Ground->GetProcMeshSection(0);int Samples=0;
            if(!Check(Mesh!=nullptr,TEXT("shared ground geometry exists"))) return false;
            for(const auto& V:Mesh->ProcVertexBuffer)
                if(FVector::DistSquared2D(FVector(V.Position),TileLocation(Tile))<45*45)
                { ++Samples;if(!Check(V.Position.Z< -9.f,TEXT("ground stays below water under the wooden bridge, with no coplanar grass plane"))) return false; }
            if(!Check(Samples>0,TEXT("bridge-bed samples are present"))) return false;
        }
        for(int I=0;I<Grass->GetInstanceCount();++I)
        {
            FTransform T;Grass->GetInstanceTransform(I,T,false);
            if(!Check(!gunyucrossing::OnBridge(Port,T.GetLocation()),TEXT("grass and reeds do not grow through the bridge decks"))) return false;
        }
        for(int Tile:Port.ConvoyPath())
            if(!Check(RoadDistance(TileLocation(Tile))<.01f,TEXT("legacy convoy checkpoints remain on the south street"))) return false;
        if(!Check(Buildings->GetCollisionEnabled()==ECollisionEnabled::NoCollision && Stonework->GetCollisionEnabled()==ECollisionEnabled::NoCollision,
            TEXT("bridge ramps, boat and rails add no physical movement blockers"))) return false;
        if(!ExportSession(Before)) return false;
        State->TerrainSnapshot.Reset();UpdateRoadTopology();BuildTerrain();SelectOfficer(1);SelectOfficer(0);
        if(!Check(ExportSession(After) && Before==After,TEXT("port scenery rebuild changes no troops, budgets, orders or history"))) return false;
    }
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedRealtime;
    bLegacyJourney=SavedLegacy;bGrandHistoricalBattles=SavedGrand;bPauseOnBattleAlert=SavedPause;
    State->bNoPlayback=SavedPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_MAP_DETAIL: PASS (real convoy route, woodland detour, ground grading, picking, immutable scenery, cache invalidation)"));return true;
}
