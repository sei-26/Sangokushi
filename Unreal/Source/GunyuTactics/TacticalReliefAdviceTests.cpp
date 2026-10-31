#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include <memory>
using namespace frontline;
void ATacticalController::PrepareReliefAdviceCapture(int Variant)
{
    CloseEpisode();Field->PrepareReliefStoryPreview(Variant==2 ? 2:1);
    Field->Issue(ETacticalCommand::Retry);OpenEpisode();OpenNovelPlan();OpenPlanningExample();
}
bool ATacticalController::ReliefAdviceSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_RELIEF_ADVICE: FAIL %s"),Why);return V; };
    auto Read=[this]() { auto S=std::make_unique<JourneySaveData>();std::vector<std::uint8_t> B;if(!Field->ExportSession(B) || !DecodeJourneySave(B,*S)) return std::unique_ptr<JourneySaveData>{};return S; };
    const int Mission=Field->FirstMission,Policy=Field->EnemyPolicy;
    const bool Council=Field->bStartWithCouncil,Realtime=Field->bRealtimeCommand,Legacy=Field->bLegacyJourney;
    CloseEpisode();Field->PrepareReliefStoryPreview(1);auto S=Read();
    if(!Check(S && S->beforeBattle && S->journey.history[2].review.relief.size()==1,TEXT("actual RT gap and arrival settle before retry"))) return false;
    const auto Camp=S->beforeBattle->world.cities[0];const auto Trust=S->beforeBattle->officerTrust;
    Field->Issue(ETacticalCommand::Retry);S=Read();
    if(!Check(S && S->journey.retryRelief && S->journey.retryRelief->outcome==JourneyReliefOutcome::Recovered &&
        !S->journey.history[2].completed && S->journey.officerTrust==Trust && S->journey.world.cities[0].food==Camp.food &&
        Field->Presentation().Journey.ExampleReason.Contains(TEXT("前回の試み")) && Field->Presentation().Journey.ExampleReason.Contains(TEXT("1回")),TEXT("retry retains a non-canonical lesson and restores unpaid resources and trust"))) return false;
    const auto Duties=Field->Presentation().Journey.ExampleDuties;
    if(!Check(Duties.Num()==3 && Duties[2].Tile==Campaign::At(8,9) && Duties[1].Tile==Campaign::At(7,6) &&
        S->journey.world.MapDistance(Duties[0].Tile,Duties[2].Tile)==1 && Field->Presentation().Journey.ExampleWarning.Contains(TEXT("最短2周期・空白1周期")),TEXT("a legal adjacent waiting role preserves the other beacon and states the real RT cost"))) return false;
    std::vector<std::uint8_t> Before,After;Field->ExportSession(Before);
    OpenEpisode();OpenNovelPlan();OpenPlanningExample();BuildInterface();Field->ExportSession(After);
    if(!Check(bNovelMap && bPlanningExample && Before==After,TEXT("current roles and the purple proposal can be compared without changing the session"))) return false;
    ApplyPlanningExample();S=Read();
    if(!Check(S && !bPlanningExample && S->journey.assignments[0].tile==Duties[0].Tile && S->journey.assignments[2].tile==Duties[2].Tile &&
        S->journey.world.cities[0].food==Camp.food && S->journey.officerTrust==Trust,TEXT("explicit UI adoption applies duties without paying or granting trust"))) return false;
    Field->ExportSession(Before);CloseEpisode();
    if(!Check(Field->ImportSession(Before) && Field->Presentation().Journey.ExampleReason.Contains(TEXT("前回の試み")),TEXT("lesson and adopted duties survive a real field save/load"))) return false;
    const auto Cost=S->journey.PreparationCosts(S->preparation);Field->Issue(ETacticalCommand::Deploy);S=Read();
    if(!Check(S && !S->journey.retryRelief && S->beforeBattle->retryRelief && S->journey.heldTurns==0 &&
        !S->journey.assignments[0].completed && S->realtime.orders[0].destination==Duties[0].Tile && S->realtime.orders[0].arrival==JourneyArrival::Hold &&
        S->journey.world.cities[0].food==Camp.food-Cost.TotalFood(),TEXT("deployment pays the ordinary bill, retains the council snapshot and starts normal arrival orders"))) return false;
    CloseEpisode();PrepareReliefAdviceCapture(2);
    if(!Check(Field->Presentation().Journey.ExampleReason.Contains(TEXT("守備条件を満たせなかった")) && !Field->Presentation().Journey.ExampleReason.Contains(TEXT("1回の守備の空白")),TEXT("actual lost guard yields the corresponding reflection without inventing a timing diagnosis"))) return false;
    CloseEpisode();Field->FirstMission=Mission;Field->EnemyPolicy=Policy;Field->bStartWithCouncil=Council;Field->bRealtimeCommand=Realtime;Field->bLegacyJourney=Legacy;
    Field->Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_RELIEF_ADVICE: PASS (real RT/lost result to retry, unpaid rollback plus saved lesson, immutable proposal, map adoption and normal paid deployment)"));return true;
}
