#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyEpisode.hpp"
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
        if(!Check(PrepareGrandBattlePreview(CoalitionMission,0,order) && State->Journey.HasCoalitionBattle() &&
            State->HUD.MissionStep.Contains(TEXT("曹操")) && State->Units.Num()==11 && State->Units[5].HistoricalSpeaker==CaoCao &&
            BuildJourneyEpisode(*State->BeforeBattle).lines.size()>=6,TEXT("both historical orders deploy eleven real formations and the same council/map goals"))) return false;
        std::vector<std::uint8_t> bytes;
        if(!Check(ExportSession(bytes) && ImportSession(bytes) && State->Journey.HasCoalitionBattle(),TEXT("the new terrain and formations save and reload paused"))) return false;
        if(!Check(PrepareGrandBattlePreview(CoalitionMission,1,order) && State->Journey.stage==JourneyStage::Battle,
            TEXT("the real automatic clock reaches the initial advance or relief phase"))) return false;
        if(order==2 && !Check(State->Journey.battleReview.grand.coalition.rescuedOn && !State->Journey.won && State->HUD.MissionAdvice.Contains(TEXT("通せ")),
            TEXT("rescue starts a real return march and is not an instant victory"))) return false;
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
    UE_LOG(LogTemp,Display,TEXT("GUNYU_COALITION_GRAND: PASS (both orders, eleven formations, actual relief and bridge return, distinct victories, council, survivors, save/reload and Yangren carryover)"));return true;
}
