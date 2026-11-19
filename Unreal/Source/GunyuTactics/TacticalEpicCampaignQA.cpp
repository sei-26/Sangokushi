#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

bool ATacticalField::PrepareEpicPreludePreview(int Phase)
{
    if(Phase<0 || Phase>2) return false;
    Issue(ETacticalCommand::Skip);bLegacyJourney=false;bGrandHistoricalBattles=true;bStartWithCouncil=false;bRealtimeCommand=true;
    FirstMission=0;bPauseOnBattleAlert=false;State->bNoPlayback=true;Issue(ETacticalCommand::Restart);
    auto& J=State->Journey;
    J.ResetCampaign();J.Meet();J.grandCampaign=J.grandStrategyCampaign=J.yangrenStrategyCampaign=J.shortPrelude=true;
    J.SelectDifficulty(JourneyDifficulty::Story);State->PreparationChoice=1;Refresh();Issue(ETacticalCommand::Deploy);
    if(J.stage!=JourneyStage::Battle || J.HasConvoy()) return false;
    if(Phase>0)
    {
        Issue(ETacticalCommand::AutoBattle);
        for(int cycle=0;cycle<20 && J.stage==JourneyStage::Battle;++cycle)
        { AdvanceRealtime(12.1f);AdvancePresentation(60);AdvanceRealtime(.1f);AdvancePresentation(60); }
        if(IsAutoBattle()) StopAutoBattle();
        if(!J.won) return false;
        if(Phase==2) Issue(ETacticalCommand::Continue);
    }
    State->bNoPlayback=false;Refresh();return true;
}
bool ATacticalField::EpicCampaignSmokeTest()
{
    auto Check=[](bool value,const TCHAR* why) { if(!value) UE_LOG(LogTemp,Error,TEXT("GUNYU_EPIC_CAMPAIGN: FAIL %s"),why);return value; };
    if(!Check(PrepareEpicPreludePreview(1) && CanIssue(ETacticalCommand::Continue) &&
        State->HUD.Journey.PrimaryLabel.Contains(TEXT("黄巾")),TEXT("the first real victory leads directly to the historical battle"))) return false;
    const int food=State->Journey.world.cities[0].food,troops=State->Journey.world.cities[0].troops;
    Issue(ETacticalCommand::Continue);
    if(!Check(State->Journey.mission==YellowTurbanMission && !State->Journey.history[1].won && !State->Journey.history[2].won &&
        State->Journey.history[2].review.resolution==JourneyResolution::JoinedMainForce &&
        State->Journey.world.cities[0].food==food && State->Journey.world.cities[0].troops==troops &&
        State->HUD.Journey.CampaignRoad.Contains(TEXT("【黄巾の乱】")),TEXT("chapter montage preserves resources and never invents side-battle victories"))) return false;
    std::vector<std::uint8_t> bytes;
    if(!Check(ExportSession(bytes) && ImportSession(bytes) && State->Journey.shortPrelude,TEXT("the short campaign saves and reloads"))) return false;
    if(!Check(PrepareEpicPreludePreview(1),TEXT("the optional full prelude starts from another real first victory"))) return false;
    Issue(ETacticalCommand::ContinuePrelude);
    if(!Check(State->Journey.mission==1 && !State->Journey.shortPrelude && CanIssue(ETacticalCommand::Deploy),TEXT("the player can retain the original prelude missions"))) return false;
    if(!Check(PrepareGrandBattlePreview(YangrenMission,3,2) && State->HUD.bYangrenCounter &&
        State->HUD.MissionAdvice.Contains(TEXT("孫堅")) && !State->HUD.Title.Contains(TEXT("本陣")) && State->Journey.world.armies[8].arm==Arm::Cavalry,
        TEXT("Sun Jian's real rendezvous releases the cavalry counterattack and its warning"))) return false;
    if(!Check(ExportSession(bytes) && ImportSession(bytes) && State->HUD.bYangrenCounter,TEXT("the counterattack is retained after a paused load"))) return false;
    if(!Check(PrepareGrandBattlePreview(YangrenMission,2,2) && State->Journey.YangrenCounterRepelled() &&
        State->Journey.world.armies[GrandCommander].troops>0,TEXT("repelling the counterattack wins without a headquarters mop-up"))) return false;
    Issue(ETacticalCommand::Continue);
    if(!Check(CanIssue(ETacticalCommand::LuoyangGate) && State->HUD.Journey.CampaignCarryover.Contains(TEXT("孫堅")),TEXT("the actual result opens the next historical operation"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_EPIC_CAMPAIGN: PASS (short and full prelude, actual resources and history, chapter road, cavalry counterattack, save/reload and next operation)"));
    return true;
}
