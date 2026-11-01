#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
using namespace frontline;
void ATacticalField::PrepareEnemyDoctrinePreview(int Variant)
{
    auto Save=std::make_unique<JourneySaveData>();auto& J=Save->journey;
    J.ResetCampaign();J.mission=Variant==2 ? 2:1;J.Meet();Save->beforeBattle=J;J.Prepare(1,0);
    if(Variant<2) J.world.armies[0].tile=J.world.armies[0].target=Variant==0 ? Campaign::At(10,8):Campaign::At(12,9);
    else
    {
        J.world.armies[0].tile=J.world.armies[0].target=J.goal;
        J.world.armies[1].tile=J.world.armies[1].target=Campaign::At(7,7);
        J.world.armies[2].tile=J.world.armies[2].target=J.Definition().secondGoal;
        J.world.armies[3].tile=J.world.armies[3].target=Campaign::At(12,10);
        J.world.armies[4].tile=J.world.armies[4].target=Campaign::At(9,6);
    }
    J.RefreshIntelligence();std::vector<std::uint8_t> Bytes;
    if(EncodeJourneySave(*Save,Bytes)) ImportSession(Bytes);
    SelectAt(J.world.armies[4].tile);
}
bool ATacticalField::EnemyDoctrineSmokeTest()
{
    const bool Legacy=bLegacyJourney,NoPlayback=State->bNoPlayback;bLegacyJourney=false;State->bNoPlayback=false;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_ENEMY_DOCTRINE: FAIL %s"),Why);return V; };
    auto Find=[](const std::vector<JourneyEnemyPlan>& Plans,int Id)->JourneyEnemyPlan
    { for(const auto& P:Plans) if(P.army==Id) return P;return {}; };
    PrepareEnemyDoctrinePreview(0);
    auto& Waiting=State->Journey;
    if(!Check(Find(Waiting.ReadEnemyPlans(),4).intent==JourneyEnemyIntent::Reserve && Presentation().EnemyPlanHint.Contains(TEXT("渡河を待つ")),TEXT("observed reserve is readable in simple commands before crossing"))) return false;
    std::vector<std::uint8_t> Before,After;
    if(!Check(ExportSession(Before),TEXT("reserve fixture has a valid saved session"))) return false;
    Refresh();Waiting.ReadEnemyPlans();
    if(!Check(ExportSession(After) && Before==After,TEXT("reading mission roles spends no resources and changes no saved state"))) return false;
    PrepareEnemyDoctrinePreview(1);
    auto& Crossing=State->Journey;
    const auto Counter=Find(Crossing.ReadEnemyPlans(),4);const FString Hint=Presentation().EnemyPlanHint;
    if(!Check(Counter.intent==JourneyEnemyIntent::Counterattack && Counter.next!=Counter.from && Hint.Contains(TEXT("橋を渡る")),TEXT("actual crossing changes role estimate and one-cell movement"))) return false;
    Issue(ETacticalCommand::EndTurn);
    if(!Check(IsPlaying() && Presentation().EnemyPlanHint==Hint,TEXT("counterattack playback retains pre-action role reading"))) return false;
    Issue(ETacticalCommand::Skip);
    if(!Check(Crossing.world.armies[4].tile==Counter.next && Crossing.world.armies[3].tile==Crossing.goal,TEXT("public turn really moves the counterattack and keeps the post defended"))) return false;
    PrepareEnemyDoctrinePreview(2);
    auto& Final=State->Journey;const auto Main=Find(Final.ReadEnemyPlans(),4);
    if(!Check(Main.role==JourneyEnemyRole::MainAttack && Main.target==Final.Definition().secondGoal && Presentation().EnemyPlanHint.Contains(TEXT("南の烽火")),TEXT("observed main attack reads the weaker wing and explains the current estimate"))) return false;
    Issue(ETacticalCommand::EndTurn);Issue(ETacticalCommand::Skip);
    if(!Check(Final.world.armies[4].tile==Main.next && Main.next!=Main.from,TEXT("main attack actually takes its observed first step"))) return false;
    bLegacyJourney=Legacy;State->bNoPlayback=NoPlayback;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_ENEMY_DOCTRINE: PASS (mission roles, observed reserve/crossing/weak wing, actual turn movement, frozen playback, immutable reads)"));return true;
}
