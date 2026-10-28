#include "TacticalTrial.h"
#include "TacticalPlanningGuide.h"
#include "JourneySave.hpp"
#include "HexGrid.hpp"
#include <memory>
namespace
{
    FString GuideDigest(const FPreparationMap& Map)
    {
        FString Text;
        for(int Officer=0;Officer<3;++Officer) for(const auto& Cell:Map.Cells)
        {
            Text+=planningguide::Place(Map,Cell.Tile);
            for(int P:planningguide::Route(Map,Officer,Cell.Tile)) Text+=FString::FromInt(P)+TEXT("/");
        }
        return Text;
    }
}
bool ATacticalController::PlanningGuideSmokeTest()
{
    using namespace frontline;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_PLANNING_GUIDE: FAIL %s"),Why);return V; };
    CloseEpisode();
    for(int Mission=0;Mission<3;++Mission)
    {
        FString Baseline;
        for(int Policy=0;Policy<3;++Policy)
        {
            auto Save=std::make_unique<JourneySaveData>();Save->journey.Reset();Save->journey.Meet();Save->journey.mission=Mission;Save->journey.enemyPolicy=Policy;
            std::vector<std::uint8_t> Bytes;
            if(!Check(EncodeJourneySave(*Save,Bytes) && Field->ImportSession(Bytes),TEXT("upcoming council imports"))) return false;
            const auto& Map=Field->Presentation().Journey.Map;
            const auto Digest=GuideDigest(Map);
            if(Policy==0) Baseline=Digest;
            else if(!Check(Digest==Baseline,TEXT("private enemy policy does not change names or any preview route"))) return false;
            for(int Officer=0;Officer<3;++Officer) for(const auto& Cell:Map.Cells)
            {
                const auto Path=planningguide::Route(Map,Officer,Cell.Tile);
                const bool Blocked=Cell.Terrain==uint8(Terrain::River) || Cell.Terrain==uint8(Terrain::Sea) || Map.Blocked.Contains(Cell.Tile);
                if(!Check(Blocked ? Path.IsEmpty():!Path.IsEmpty(),TEXT("every public land destination is connected; river/city cannot be a destination"))) return false;
                if(Path.IsEmpty()) continue;
                if(!Check(Path[0]==Map.Starts[Officer].Tile && Path.Last()==Cell.Tile,TEXT("preview belongs to the selected officer and selected target"))) return false;
                for(int I=1;I<Path.Num();++I)
                    if(!Check(hexgrid::Distance(Path[I-1]%Width,Path[I-1]/Width,Path[I]%Width,Path[I]/Width)==1 && !Map.Blocked.Contains(Path[I]),TEXT("every preview edge is a traversable adjacent hex"))) return false;
            }
        }
    }
    OpenEpisode();OpenNovelPlan();TogglePlanningMap();
    if(!Check(bPlanningDuties && !bPlanningLocationChosen && !PlanningDutyReason(1).IsEmpty(),TEXT("map opens the guided editor, requiring an explicit target"))) return false;
    std::vector<std::uint8_t> Before,After;Field->ExportSession(Before);
    SetPlanningDuty(1);Field->ExportSession(After);
    if(!Check(Before==After,TEXT("an unfinished command cannot accidentally assign the initial objective"))) return false;
    SelectPlanningOfficer(1);SelectPlanningTile(Campaign::At(7,5));
    if(!Check(PlanningDutyReason(2).IsEmpty() && PlanningPreviewRoute().Num()==1,TEXT("Guan can prepare at his actual forest departure position"))) return false;
    Field->ExportSession(After);
    if(!Check(Before==After && PlanningOrder(1).Contains(TEXT("未指定")),TEXT("choosing an officer and destination previews without committing"))) return false;
    SetPlanningDuty(2);
    if(!Check(PlanningNotice.Contains(TEXT("命令を確定")) && PlanningOrder(1).Contains(TEXT("伏兵準備")),TEXT("confirmation and readable summary describe the real stored command"))) return false;
    SelectPlanningOfficer(2);SelectPlanningTile(Campaign::At(7,5));
    if(!Check(!PlanningDutyReason(1).IsEmpty(),TEXT("another officer's assigned destination has an explicit rejection reason"))) return false;
    SelectPlanningTile(Campaign::At(9,7));
    if(!Check(!PlanningDutyReason(1).IsEmpty() && PlanningPreviewRoute().IsEmpty(),TEXT("river selection explains the unavailable order and shows no fake route"))) return false;
    SelectPlanningTile(Campaign::At(8,9));
    if(!Check(!PlanningDutyReason(2).IsEmpty(),TEXT("non-ambush officer cannot choose forest preparation"))) return false;
    SelectPlanningOfficer(1);
    if(!Check(PlanningTile==Campaign::At(7,5) && bPlanningLocationChosen,TEXT("clicking a stored order returns to its actual target"))) return false;
    SetPlanningDuty(0);
    if(!Check(PlanningOrder(1).Contains(TEXT("未指定")) && PlanningNotice.Contains(TEXT("解除")),TEXT("clear restores explicit free battlefield command"))) return false;
    CloseEpisode();
    UE_LOG(LogTemp,Display,TEXT("GUNYU_PLANNING_GUIDE: PASS (three-step input, no accidental commit, real names and reasons, editable summaries, 4536 public routes, private-policy isolation)"));return true;
}
