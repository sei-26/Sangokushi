#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyEpisode.hpp"
void ATacticalField::BuildEpisodePresentation()
{
    const auto& J=State->Journey;
    auto& Journey=State->HUD.Journey;
    const auto Copy=[&](const frontline::JourneyEpisode& Episode,FJourneyEpisodePresentation& P)
    {
        if(Episode.lines.empty()) return;
        P.Key=FString::Printf(TEXT("%d:%d"),J.mission,int(J.stage));
        P.Title=JourneyText(Episode.title);P.Place=JourneyText(Episode.place);
        P.Setting=JourneyText(Episode.setting);P.Question=JourneyText(Episode.question);
        for(const auto& Line:Episode.lines)
        {
            FDialoguePresentation D;D.Speaker=Line.speaker;D.Name=JourneyText(frontline::EpisodeSpeakerName(Line.speaker));
            D.Text=JourneyText(Line.text);D.Direction=JourneyText(Line.direction);D.bProtagonist=Line.speaker==3;D.WarmMask=Line.warmMask;
            P.Lines.Add(MoveTemp(D));
        }
    };
    Copy(frontline::BuildJourneyEpisode(J),Journey.Episode);
    if(Journey.bComplete) Copy(frontline::BuildCoalitionEpisode(J),Journey.CoalitionEpisode);
}
