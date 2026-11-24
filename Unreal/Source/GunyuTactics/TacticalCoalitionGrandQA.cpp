#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyEpisode.hpp"
#include "TacticalCrossingLandscape.h"
#include "Components/InstancedStaticMeshComponent.h"
using namespace frontline;

bool ATacticalField::CoalitionGrandSmokeTest()
{
    auto Check=[](bool value,const TCHAR* why) { if(!value) UE_LOG(LogTemp,Error,TEXT("GUNYU_COALITION_GRAND: FAIL %s"),why);return value; };
    for(int order:{1,2})
    {
        if(!Check(PrepareGrandBattlePreview(CoalitionMission,4,order) && State->HUD.Journey.Map.Sites.ContainsByPredicate(
            [order](const FPlanningSite& Site) { return Site.Name==(order==2 ? TEXT("西岸の帰還地点"):TEXT("曹操と合流する前陣")); }),
            TEXT("the council map names the real operation rather than the old port or beacons"))) return false;
        Issue(ETacticalCommand::PlanExample);
        if(!Check(CanIssue(ETacticalCommand::Deploy),TEXT("the modern planning example can be adopted and deployed"))) return false;
        for(int Tile:gunyucrossing::Bridges(State->Journey))
        {
            const auto& Sites=State->HUD.Journey.Map.Sites;
            const auto Matches=Sites.FilterByPredicate([Tile](const FPlanningSite& S) { return S.Tile==Tile; });
            if(!Check(Matches.Num()==1 && Matches[0].Hints.Num()==3 && !Matches[0].Description.Contains(TEXT("前哨")) &&
                !Matches[0].Hints[0].Contains(TEXT("指揮所")),TEXT("each coalition bridge has one operation-specific council entry"))) return false;
        }
        if(!Check(PrepareGrandBattlePreview(CoalitionMission,0,order) && State->Journey.HasCoalitionBattle() &&
            State->HUD.MissionStep.Contains(TEXT("曹操")) && State->Units.Num()==11 && State->Units[5].HistoricalSpeaker==CaoCao &&
            BuildJourneyEpisode(*State->BeforeBattle).lines.size()>=6,TEXT("both historical orders deploy eleven real formations and the same council/map goals"))) return false;
        std::vector<std::uint8_t> bytes;
        const auto Bridges=gunyucrossing::Bridges(State->Journey);
        if(!Check(Bridges.Num()==2 && State->HUD.Crossings[0].Tile==Bridges[0] && State->HUD.Crossings[1].Tile==Bridges[1] &&
            State->HUD.Crossings[1].Caption.Contains(order==2 ? TEXT("帰路"):TEXT("援軍")),TEXT("both real bridges are named for the chosen operation"))) return false;
        for(int Tile:{Bridges[0],Bridges[1],State->Journey.goal})
        {
            const FVector Offset(400,300,1600);
            if(!Check(RoadDistance(TileLocation(Tile))<.01f && PickSurfaceTile(SurfaceLocation(Tile)+Offset,-Offset.GetSafeNormal())==Tile,
                TEXT("streets join the two bridges and assembly site without changing tile selection"))) return false;
        }
        for(int I=0;I<Buildings->GetInstanceCount();++I)
        {
            FTransform T;Buildings->GetInstanceTransform(I,T,false);
            if(!Check(FVector::Dist2D(T.GetLocation(),SurfaceLocation(State->Journey.goal))>=70 || T.GetLocation().Z<55,
                TEXT("the friendly assembly site has no tall central enemy-command-post proxy"))) return false;
        }
        if(!ExportSession(bytes)) return false;
        State->TerrainSnapshot.Reset();UpdateRoadTopology();BuildTerrain();Refresh();
        std::vector<std::uint8_t> after;
        if(!Check(ExportSession(after) && bytes==after,TEXT("bridge routes, assembly scenery and read-only guidance spend and change nothing"))) return false;
        if(!Check(ExportSession(bytes) && ImportSession(bytes) && State->Journey.HasCoalitionBattle(),TEXT("the new terrain and formations save and reload paused"))) return false;
        if(!Check(PrepareGrandBattlePreview(CoalitionMission,1,order) && State->Journey.stage==JourneyStage::Battle,
            TEXT("the real automatic clock reaches the initial advance or relief phase"))) return false;
        if(order==2 && !Check(State->Journey.battleReview.grand.coalition.rescuedOn && !State->Journey.won && State->HUD.MissionAdvice.Contains(TEXT("通せ")),
            TEXT("rescue starts a real return march and is not an instant victory"))) return false;
        for(const auto& P:State->Journey.PlanCoalitionAllies())
            if(P.army==5 || (P.army==6 && State->Journey.battleReview.grand.coalition.rescuedOn))
            {
                const auto& Route=P.army==5 ? State->HUD.GrandAdvanceRoute:State->HUD.ReliefAdvanceRoute;
                if(!Check(Route.Num()>0 && Route[0]==P.from && (P.next==P.from ? Route.Num()==1:Route.Num()>1 && Route[1]==P.next),
                    TEXT("Cao Cao and relief routes begin with the actual public allied step"))) return false;
            }
        if(order==2)
        {
            if(!Check(State->HUD.GrandOrderStatus.Contains(TEXT("帰還")) && State->HUD.GrandRouteLegend.Contains(TEXT("金")) &&
                State->HUD.MissionFocusTile==State->Journey.world.armies[6].tile,TEXT("after relief, progress and target focus follow the unit that returns first"))) return false;
            if(!ExportSession(bytes)) return false;
            for(auto& E:State->Journey.intelligence[0].contacts) E.current=false;
            Refresh();const auto Cao=State->HUD.GrandAdvanceRoute,Relief=State->HUD.ReliefAdvanceRoute;
            State->Journey.world.armies[10].tile=Campaign::At(14,12);State->Journey.world.armies[10].troops+=999;
            Refresh();
            if(!Check(Cao==State->HUD.GrandAdvanceRoute && Relief==State->HUD.ReliefAdvanceRoute,TEXT("unobserved private enemies cannot change friendly route previews"))) return false;
            if(!ImportSession(bytes)) return false;
        }
        if(!Check(PrepareGrandBattlePreview(CoalitionMission,2,order) && State->Journey.won &&
            State->HUD.Journey.CampaignChange.Contains(TEXT("曹操軍の生還")) && BuildJourneyEpisode(State->Journey).lines.size()>=4,
            TEXT("both operations reach actual outcomes and their character aftermath"))) return false;
        if(order==2 && !Check(State->Journey.battleReview.grand.coalition.returnedOn && State->Journey.world.armies[5].tile%Width<=6 &&
            State->Journey.world.armies[6].tile%Width<=6 && State->Journey.world.armies[10].troops>0,
            TEXT("the relief victory actually brings both allies across and leaves the enemy headquarters alive"))) return false;
        if(!Check(ExportSession(bytes) && ImportSession(bytes),TEXT("the settled survivor record is retained"))) return false;
        Issue(ETacticalCommand::Continue);Issue(ETacticalCommand::Continue);
        if(!Check(State->Journey.mission==YangrenMission && State->Journey.stage==JourneyStage::Preparation &&
            (order!=2 || (State->Journey.HasCoalitionGuide() && State->HUD.Journey.CampaignCarryover.Contains(TEXT("張邈")))),
            TEXT("actual surviving relief troops support the next Sun Jian operation"))) return false;
    }
    UE_LOG(LogTemp,Display,TEXT("GUNYU_COALITION_GRAND: PASS (both orders, unique bridge advice, public two-army routes, open assembly sites, actual relief and bridge return, distinct victories, immutable scene, fog, survivors, save/reload and Yangren carryover)"));return true;
}
