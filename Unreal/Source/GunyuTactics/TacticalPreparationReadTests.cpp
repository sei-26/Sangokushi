#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include <memory>
bool ATacticalController::PreparationReadSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_PREPARATION_READ: FAIL %s"),Why);return V; };
    CloseEpisode();Field->PrepareOpeningPreview(false);
    std::vector<std::uint8_t> Bytes;
    auto Source=std::make_unique<frontline::JourneySaveData>();
    if(!Check(Field->ExportSession(Bytes) && frontline::DecodeJourneySave(Bytes,*Source),TEXT("real council source"))) return false;
    Source->journey.officerTrust={16,-10,20};
    if(!Check(frontline::EncodeJourneySave(*Source,Bytes) && Field->ImportSession(Bytes),TEXT("restore valid mixed trust before deployment"))) return false;
    const auto Plans=Field->Presentation().Journey.Plans;
    for(int Choice=0;Choice<2;++Choice)
    {
        auto Actual=std::make_unique<frontline::StrategistJourney>(Source->journey);
        const auto Cost=Actual->PreparationCosts(Choice);const auto Camp=Actual->world.cities[0];
        if(!Check(Plans[Choice].Food==Cost.TotalFood() && Plans[Choice].Gold==Cost.gold && Plans[Choice].MinimumTroops==Cost.MinimumTroops(),TEXT("typed comparison is built from actual costs including guide/cargo"))) return false;
        if(!Check(Actual->Prepare(Choice,Actual->enemyPolicy),TEXT("either affordable plan can actually deploy"))) return false;
        for(int I=0;I<3;++I)
            if(!Check(Plans[Choice].Morale[I]==Actual->world.armies[I].morale,TEXT("shown morale matches real deployment, including caps and penalties"))) return false;
        if(!Check(Camp.food-Actual->world.cities[0].food==Plans[Choice].Food && Camp.gold-Actual->world.cities[0].gold==Plans[Choice].Gold,TEXT("shown expense equals the actual debit"))) return false;
    }
    if(!Check(Field->Presentation().Journey.PlanDifference.Contains(TEXT("+1800")) && Field->Presentation().Journey.PlanDifference.Contains(TEXT("-120")),TEXT("the two tradeoffs are expressed as actual differences"))) return false;
    OpenEpisode();OpenNovelPlan();std::vector<std::uint8_t> Before,After;
    if(!Check(Field->ExportSession(Before),TEXT("before reading officer perspectives"))) return false;
    HandleEpisodeKey(EKeys::A);HandleEpisodeKey(EKeys::A);
    if(!Check(!bNovelAdvice && Field->ExportSession(After) && Before==After,TEXT("reading advice changes no authority, trust or resources"))) return false;
    const auto Advice=Field->Presentation().Journey.PlanAdvice;CloseEpisode();
    Source->journey.enemyPolicy=2;
    if(!Check(frontline::EncodeJourneySave(*Source,Bytes) && Field->ImportSession(Bytes),TEXT("alter only an unseen enemy policy"))) return false;
    for(int I=0;I<3;++I)
        if(!Check(Advice[I].Text==Field->Presentation().Journey.PlanAdvice[I].Text,TEXT("advice does not reveal hidden enemy policy"))) return false;
    // Only the cheaper-in-food plan fits. The UI must expose the viable alternative.
    auto& Camp=Source->journey.world.cities[0];Camp.food=Source->journey.PreparationCosts(0).TotalFood();
    if(!Check(frontline::EncodeJourneySave(*Source,Bytes) && Field->ImportSession(Bytes),TEXT("restore scarce but usable supplies"))) return false;
    OpenEpisode();OpenNovelPlan();HandleEpisodeKey(EKeys::Two);
    if(!Check(!Field->CanIssue(ETacticalCommand::Deploy) && Field->Presentation().Journey.PlanReadiness.Contains(TEXT("兵糧あと1800")) && Field->Presentation().Journey.PlanReadiness.Contains(TEXT("案1なら")),TEXT("a blocked plan explains shortage and an actually viable alternative"))) return false;
    Field->ExportSession(Before);HandleEpisodeKey(EKeys::D);Field->ExportSession(After);
    if(!Check(bNovelPlan && Field->Presentation().Journey.bPreparation && Before==After,TEXT("trying to deploy a blocked plan cannot spend or advance"))) return false;
    HandleEpisodeKey(EKeys::One);
    if(!Check(Field->CanIssue(ETacticalCommand::Deploy),TEXT("switching to the affordable alternative really enables deployment"))) return false;
    CloseEpisode();Camp.food=0;Camp.gold=0;Camp.troops=0;
    if(!Check(frontline::EncodeJourneySave(*Source,Bytes) && Field->ImportSession(Bytes),TEXT("restore valid zero-resource council"))) return false;
    for(int Choice=0;Choice<2;++Choice)
    {
        const auto& P=Field->Presentation().Journey.Plans[Choice];
        if(!Check(P.MissingFood==P.Food && P.MissingGold==P.Gold && P.MissingTroops==P.MinimumTroops && P.Shortage.Contains(TEXT("兵あと")) && P.Shortage.Contains(TEXT("兵糧あと")) && P.Shortage.Contains(TEXT("軍資金あと")),TEXT("all three deficits use the real minimum requirements"))) return false;
    }
    Field->PrepareBattleStoryPreview(false);Field->Issue(ETacticalCommand::StoryReplyPreserve);Field->Issue(ETacticalCommand::Continue);
    if(!Check(Field->Presentation().Journey.PlanAdvice[1].Text.Contains(TEXT("九割")),TEXT("Guan Yu's advice remembers the actual preservation vow"))) return false;
    Field->PrepareBattleStoryPreview(false);Field->Issue(ETacticalCommand::StoryReplyCoordinate);Field->Issue(ETacticalCommand::Continue);
    if(!Check(Field->Presentation().Journey.PlanAdvice[2].Text.Contains(TEXT("二隊以上")),TEXT("Zhang Fei's advice remembers the actual cooperative vow"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_PREPARATION_READ: PASS (actual costs/morale, caps, deficits, viable alternative, blocked deployment, immutable advice, hidden-policy isolation, real vows)"));return true;
}
