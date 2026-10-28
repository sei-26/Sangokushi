#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include <memory>
bool ATacticalController::AccessibilitySmokeTest()
{
    using namespace frontline;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_ACCESSIBILITY: FAIL %s"),Why);return V; };
    CloseEpisode();Field->bStartOnStandardDifficulty=false;Field->bStartWithCouncil=true;Field->FirstMission=0;Field->bRealtimeCommand=true;
    Field->Issue(ETacticalCommand::Restart);
    if(!Check(Field->Presentation().Journey.bStoryDifficulty,TEXT("actual fresh Unreal entry defaults to Story difficulty"))) return false;
    Field->Issue(ETacticalCommand::Dialogue);Field->Issue(ETacticalCommand::Dialogue);Field->Issue(ETacticalCommand::Dialogue);
    std::vector<std::uint8_t> Before,Bytes;auto Save=std::make_unique<JourneySaveData>();Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    const auto Camp=Save->journey.world.cities[0];
    if(!Check(Save->journey.stage==JourneyStage::Preparation && Save->journey.TurnBudget()==13,TEXT("rescue mission has three extra command cycles"))) return false;
    Field->Issue(ETacticalCommand::DifficultyStandard);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.TurnBudget()==10 && Save->journey.world.cities[0].food==Camp.food && Save->journey.officerTrust[0]==0,TEXT("difficulty choice is free and uses the real mission deadline"))) return false;
    Field->Issue(ETacticalCommand::DifficultyStory);OpenEpisode();OpenNovelPlan();TogglePlanningMap();
    SelectPlanningTile(Campaign::At(12,11));
    if(!Check(PlanningPreviewRoute().Num()>1,TEXT("an edited route exists before opening the example"))) return false;
    Field->ExportSession(Before);bPlanningExample=true;
    if(!Check(!Field->Presentation().Journey.ExampleManual.IsEmpty() && Field->ExportSession(Bytes) && Before==Bytes && PlanningPreviewRoute().IsEmpty() && PlanningExecution().Contains(TEXT("右の見本")),TEXT("reviewing the example consumes nothing, explains manual work and cannot display a stale route as the example"))) return false;
    ApplyPlanningExample();Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(!bPlanningExample && Save->journey.assignments[0].duty==JourneyDuty::Hold && Save->journey.assignments[1].duty==JourneyDuty::Ambush && Save->journey.assignments[2].duty==JourneyDuty::None && Save->journey.world.cities[0].gold==Camp.gold,
        TEXT("explicit apply changes real assignments while leaving Zhang free to command"))) return false;
    const auto Cost=Save->journey.PreparationCosts(0);HandleEpisodeKey(EKeys::D);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.stage==JourneyStage::Battle && Save->journey.world.armies[3].troops==1440 && Save->journey.world.armies[4].troops==1040 && Save->journey.TurnBudget()==13 && Field->RealtimePaused(),TEXT("deployment really scales enemies and starts paused"))) return false;
    if(!Check(Save->journey.world.cities[0].food==Camp.food-Cost.TotalFood() && Save->beforeBattle && Save->beforeBattle->difficulty==JourneyDifficulty::Story && (Save->realtime.orders[1].arrival==JourneyArrival::Ambush || (Save->journey.assignments[1].completed && Save->journey.chain.concealed[1])) && Save->realtime.orders[2].kind==JourneyDirective::None,TEXT("the reviewed bill, retry difficulty and real role orders agree"))) return false;
    if(!Check(!Field->CanIssue(ETacticalCommand::DifficultyStandard) && !Field->CanIssue(ETacticalCommand::PlanExample),TEXT("battle cannot rewrite difficulty or the initial plan"))) return false;
    if(!Check(Field->ImportSession(Bytes) && Field->Presentation().Difficulty==TEXT("物語重視"),TEXT("load restores actual difficulty in the battle header"))) return false;
    Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);Field->Issue(ETacticalCommand::Retry);
    Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.difficulty==JourneyDifficulty::Story && Save->journey.world.cities[0].food==Camp.food && Save->journey.assignments[1].duty==JourneyDuty::Ambush,TEXT("retry restores difficulty, template and unpaid supplies"))) return false;
    Field->Issue(ETacticalCommand::DifficultyStandard);Field->Issue(ETacticalCommand::Deploy);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.world.armies[3].troops==1800 && Save->journey.world.armies[4].troops==1300 && Save->journey.TurnBudget()==10,TEXT("Standard preserves former opposition and timing"))) return false;
    Field->bStartOnStandardDifficulty=true;CloseEpisode();
    UE_LOG(LogTemp,Display,TEXT("GUNYU_ACCESSIBILITY: PASS (Story default, explicit Standard, free example review, real roles, enemy strength/deadline, locked battle, save/load and retry finance)"));return true;
}
