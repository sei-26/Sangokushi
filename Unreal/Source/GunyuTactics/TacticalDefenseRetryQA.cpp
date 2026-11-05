#include "TacticalTrial.h"
#include "TacticalSupportQA.h"
#include "JourneySave.hpp"
#include <memory>
bool PrepareDefenseRetryPreview(ATacticalField* Field,int Variant)
{
    using namespace frontline;
    if(!Field || Variant<0 || Variant>3) return false;
    if(Variant<3)
    {
        const int Battle=Variant==0 ? 1:Variant==1 ? 4:5;
        if(!PrepareDefenseAftermathPreview(Field,Battle)) return false;
    }
    else
    {
        Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);
    }
    Field->Issue(ETacticalCommand::Retry);
    auto Save=std::make_unique<JourneySaveData>();std::vector<std::uint8_t> Bytes;
    if(!Field->ExportSession(Bytes) || !DecodeJourneySave(Bytes,*Save) || Save->journey.stage!=JourneyStage::Preparation) return false;
    if(Variant==3) return !Save->journey.retryDefense;
    const int Actor=Variant==0 ? 1:Variant==1 ? 2:0;
    return Save->journey.retryDefense && Save->journey.retryDefense->failedGuards==(1<<Actor) &&
        Save->journey.history[Save->journey.mission].review.spentGuardFailures==0;
}
