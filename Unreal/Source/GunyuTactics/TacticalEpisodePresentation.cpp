#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyEpisode.hpp"
void ATacticalField::BuildEpisodePresentation()
{
    const auto& J=State->Journey;const auto E=frontline::BuildJourneyEpisode(J);
    if(E.lines.empty()) return;
    auto& P=State->HUD.Journey.Episode;
    P.Key=FString::Printf(TEXT("%d:%d"),J.mission,int(J.stage));
    P.Title=JourneyText(E.title);P.Place=JourneyText(E.place);P.Setting=JourneyText(E.setting);P.Question=JourneyText(E.question);
    for(const auto& Line:E.lines)
    {
        FDialoguePresentation D;D.Speaker=Line.speaker;D.Name=JourneyText(frontline::EpisodeSpeakerName(Line.speaker));
        D.Text=JourneyText(Line.text);D.Direction=JourneyText(Line.direction);D.bProtagonist=Line.speaker==3;
        P.Lines.Add(MoveTemp(D));
    }
}
