#include "TacticalTrial.h"
#include "TacticalState.h"

bool ATacticalField::MapDetailSmokeTest()
{
    using namespace frontline;
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedPlayback=State->bNoPlayback,SavedRealtime=bRealtimeCommand;
    auto Check=[](bool Value,const TCHAR* Why)
    { if(!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_MAP_DETAIL: FAIL %s"),Why);return Value; };
    FirstMission=0;EnemyPolicy=1;bStartWithCouncil=true;bRealtimeCommand=false;State->bNoPlayback=true;
    Issue(ETacticalCommand::Restart);
    for(int I=0;I<3;++I) Issue(ETacticalCommand::Dialogue);
    Issue(ETacticalCommand::PromiseRescue);Issue(ETacticalCommand::RouteSouthernWood);Issue(ETacticalCommand::Deploy);
    const auto& J=State->Journey;
    if(!Check(J.stage==JourneyStage::Battle && J.convoyRoute==JourneyConvoyRoute::SouthernWood,TEXT("public controls deploy the woodland route"))) return false;
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
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedRealtime;
    State->bNoPlayback=SavedPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_MAP_DETAIL: PASS (real convoy route, woodland detour, ground grading, picking, immutable scenery, cache invalidation)"));return true;
}
