#include "TacticalTrial.h"
#include "TacticalPlanningMap.h"
#include "JourneySave.hpp"
#include <memory>

namespace
{
    FString MapDigest(const FPreparationMap& Map)
    {
        FString S=Map.Name+Map.Objective;
        for(const auto& C:Map.Cells) S+=FString::Printf(TEXT("|%d:%d"),C.Tile,C.Terrain);
        for(const auto& U:Map.Starts) S+=FString::Printf(TEXT("|%d:%d"),U.Tile,U.Officer);
        for(int Tile:Map.ConvoyRoute) S+=FString::Printf(TEXT("/%d"),Tile);
        for(const auto& Site:Map.Sites) { S+=Site.Name+Site.Description;for(const auto& H:Site.Hints) S+=H; }
        return S;
    }
}
bool ATacticalController::PlanningMapSmokeTest()
{
    using namespace frontline;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_PLANNING_MAP: FAIL %s"),Why);return V; };
    CloseEpisode();
    for(int Mission=0;Mission<3;++Mission)
    {
        FString Baseline;
        for(int Policy=0;Policy<3;++Policy)
        {
            auto Source=std::make_unique<JourneySaveData>();Source->journey.Reset();Source->journey.Meet();Source->journey.mission=Mission;Source->journey.enemyPolicy=Policy;
            if(Mission==0) Source->journey.SelectPromise(JourneyPromise::EscortRefugees);
            std::vector<std::uint8_t> Bytes;
            if(!Check(EncodeJourneySave(*Source,Bytes) && Field->ImportSession(Bytes),TEXT("valid upcoming council fixture"))) return false;
            const auto& Map=Field->Presentation().Journey.Map;
            if(!Check(Map.Cells.Num()==168 && Map.Starts.Num()==(Mission==2 ? 3:4),TEXT("only public terrain and friendly departure slots are present"))) return false;
            if(Policy==0) Baseline=MapDigest(Map);
            else if(!Check(MapDigest(Map)==Baseline,TEXT("private enemy policy cannot alter the map or its advice"))) return false;
            auto Actual=std::make_unique<StrategistJourney>(Source->journey);
            if(!Check(Actual->Prepare(0,Policy),TEXT("the same council can really deploy"))) return false;
            for(const auto& Cell:Map.Cells)
            {
                if(!Check(uint8(Actual->world.tiles[Cell.Tile].terrain)==Cell.Terrain,TEXT("preview terrain matches real deployment, rather than the previous battlefield"))) return false;
                for(FVector2D Size:{FVector2D(650,400),FVector2D(400,280),FVector2D(900,560)})
                    if(!Check(SPlanningMap::TileAt(SPlanningMap::TilePosition(Cell.Tile,Size),Size)==Cell.Tile,TEXT("drawn hex centers select the same tile at three sizes"))) return false;
            }
            for(const auto& Start:Map.Starts)
                if(!Check(Actual->world.armies[Start.Officer<0 ? StrategistJourney::ConvoyArmy:Start.Officer].tile==Start.Tile,TEXT("departure slots match deployed friends and convoy"))) return false;
            if(!Check(!Map.Sites.IsEmpty() && Map.Sites[0].Tile==Actual->goal,TEXT("upcoming objective uses the real mission goal"))) return false;
            if(!Check(SPlanningMap::TileAt(FVector2D(-30,-30),FVector2D(650,400))==-1,TEXT("outside clicks do not select an unrelated hex"))) return false;
        }
        OpenEpisode();OpenNovelPlan();std::vector<std::uint8_t> Before,After;
        if(!Check(Field->ExportSession(Before),TEXT("stable pre-reading session"))) return false;
        HandleEpisodeKey(EKeys::M);
        if(!Check(bNovelMap && bNovelPlan && bEpisodeOpen,TEXT("M opens the map inside the operation review"))) return false;
        const auto& Map=Field->Presentation().Journey.Map;
        if(!Check(PlanningWidget.IsValid(),TEXT("the actual map widget is attached"))) return false;
        const FVector2D WidgetSize(650,400);
        const FGeometry WidgetGeometry=FGeometry::MakeRoot(FVector2f(WidgetSize),FSlateLayoutTransform(.8f,FVector2f(120,40)));
        const FVector2D ScreenPoint=WidgetGeometry.LocalToAbsolute(SPlanningMap::TilePosition(Map.Sites.Last().Tile,WidgetSize));
        TSet<FKey> Pressed;Pressed.Add(EKeys::LeftMouseButton);
        const FPointerEvent Click(0,ScreenPoint,ScreenPoint,Pressed,EKeys::LeftMouseButton,0,FModifierKeysState());
        if(!Check(PlanningWidget->OnMouseButtonDown(WidgetGeometry,Click).IsEventHandled() && PlanningTile==Map.Sites.Last().Tile,TEXT("scaled mouse input reaches the actual site-selection delegate"))) return false;
        if(!Check(!PlanningTitle().IsEmpty() && !PlanningDescription().IsEmpty() && !PlanningHint(0).IsEmpty(),TEXT("selected terrain/site has useful readings"))) return false;
        HandleEpisodeKey(EKeys::Enter);HandleEpisodeKey(EKeys::SpaceBar);
        if(!Check(Field->ExportSession(After) && Before==After,TEXT("reading and advance keys cannot spend, deploy or change progress"))) return false;
        HandleEpisodeKey(EKeys::Escape);
        if(!Check(!bNovelMap && bNovelPlan,TEXT("Esc returns to the preparation comparison"))) return false;
        HandleEpisodeKey(EKeys::M);HandleEpisodeKey(EKeys::A);
        if(!Check(!bNovelMap && bNovelAdvice,TEXT("officer advice and map are exclusive"))) return false;
        CloseEpisode();
    }
    Field->PrepareOpeningPreview(false);OpenEpisode();OpenNovelPlan();HandleEpisodeKey(EKeys::M);
    // The public preview helper starts on the southern route; first select road.
    if(Field->Presentation().Journey.Map.bSouthern) HandleEpisodeKey(EKeys::R);
    std::vector<std::uint8_t> Bytes;auto Before=std::make_unique<JourneySaveData>(),After=std::make_unique<JourneySaveData>();
    Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Before);
    const int RoadGold=Field->Presentation().Journey.Plans[0].Gold;
    HandleEpisodeKey(EKeys::R);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*After);
    if(!Check(Field->Presentation().Journey.Map.bSouthern && Field->Presentation().Journey.Map.ConvoyRoute.Contains(Campaign::At(9,12)),TEXT("route control changes the actual woodland path"))) return false;
    if(!Check(Field->Presentation().Journey.Plans[0].Gold==RoadGold+120 && Before->journey.world.cities[0].gold==After->journey.world.cities[0].gold,TEXT("guide bill updates without spending before deployment"))) return false;
    const auto Camp=After->journey.world.cities[0];const auto Cost=After->journey.PreparationCosts(1);
    HandleEpisodeKey(EKeys::Two);HandleEpisodeKey(EKeys::D);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*After);
    if(!Check(!bEpisodeOpen && !bNovelMap && After->journey.stage==JourneyStage::Battle && After->journey.convoyRoute==JourneyConvoyRoute::SouthernWood && After->journey.preparation==1,
        TEXT("explicit deployment carries the chosen map route and preparation"))) return false;
    if(!Check(After->journey.world.cities[0].gold==Camp.gold-Cost.gold && After->journey.world.cities[0].food==Camp.food-Cost.TotalFood(),TEXT("actual deployment pays the reviewed cost exactly"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_PLANNING_MAP: PASS (three upcoming maps, private-policy isolation, friendly slots, 4536 hex picks, immutable reading, input guards, route bill and real deployment)"));return true;
}
