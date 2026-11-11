#include "TacticalContributionQA.h"
#include "TacticalTrial.h"
#include "../../../tests/JourneyContributionPlayer.hpp"
bool PrepareContributionPreview(ATacticalField* Field,int Variant)
{
    if(!Field || Variant<0 || Variant>3) return false;
    Field->Issue(ETacticalCommand::Skip);
    auto Session=contributionqa::Play(Variant);if(!Session) return false;
    std::vector<std::uint8_t> Bytes;if(!frontline::EncodeJourneySave(*Session,Bytes)) return false;
    Field->bLegacyJourney=false;return Field->ImportSession(Bytes);
}
bool PrepareCooperationCouncilPreview(ATacticalField* Field,int Variant)
{
    if(!Field || Variant<0 || Variant>3) return false;
    Field->Issue(ETacticalCommand::Skip);auto Session=contributionqa::Play(Variant);
    if(!Session || !Session->journey.Continue()) return false;
    Session->beforeBattle=Session->journey;Session->realtimeMode=true;
    std::vector<std::uint8_t> Bytes;if(!frontline::EncodeJourneySave(*Session,Bytes)) return false;
    Field->bLegacyJourney=false;return Field->ImportSession(Bytes);
}
