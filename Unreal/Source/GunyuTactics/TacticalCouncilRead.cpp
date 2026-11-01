#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyCouncilStage.hpp"
FCouncilStageExchange ATacticalField::ReadCouncilExchange(int Voice,int Decision) const
{
    FCouncilStageExchange Read;
    if(Decision<0 || Decision>4) return Read;
    const auto Exchange=frontline::BuildCouncilExchange(State->Journey,Voice,State->PreparationChoice,static_cast<frontline::CouncilDecision>(Decision));
    Read.Subject=JourneyText(Exchange.subject);
    for(const auto& Beat:Exchange.beats)
    {
        FCouncilStageLine L;const auto& D=Beat.dialogue;
        L.Dialogue.Speaker=D.speaker;L.Dialogue.Name=JourneyText(frontline::EpisodeSpeakerName(D.speaker));
        L.Dialogue.Text=JourneyText(D.text);L.Dialogue.Direction=JourneyText(D.direction);L.Dialogue.bProtagonist=D.speaker==3;L.Dialogue.WarmMask=D.warmMask;
        for(int I=0;I<3;++I) L.Reactions[I]=JourneyText(Beat.reactions[I]);
        Read.Lines.Add(MoveTemp(L));
    }
    return Read;
}
