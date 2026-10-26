#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalChainReview.h"
bool ATacticalField::HasChainReview() const
{
    const auto& J=State->Journey;
    return !IsPlaying() && J.stage!=frontline::JourneyStage::Encounter && J.stage!=frontline::JourneyStage::Preparation && !J.lastChain.prediction && !J.lastChain.events.empty();
}
FChainReview ATacticalField::ReadChainReview() const
{
    if(!HasChainReview()) return {};
    const auto& J=State->Journey;TArray<FString> Names;Names.SetNum(6);
    auto Name=[&](int General)
    { return General>=0 && General<int(J.world.generals.size()) ? JourneyText(J.world.generals[General].name):FString(); };
    for(int I=0;I<3 && I<int(J.world.armies.size());++I) Names[I]=Name(J.world.armies[I].general);
    // Reports may name a past observation. Never inspect an unknown enemy army.
    for(int I=3;I<5;++I) if(const auto* Contact=J.intelligence[0].Find(I)) Names[I]=Name(Contact->observed.general);
    return gunyureview::Build(J.lastChain,Names);
}
