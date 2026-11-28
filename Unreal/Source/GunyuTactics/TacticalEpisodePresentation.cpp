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
    if(Journey.bDebrief && J.campaignFlow)
    {
        int Preferred=-1;
        if(J.HasCoalitionBattle() && J.won && J.battleReview.grand.coalition.rescuedOn && J.battleReview.grand.coalition.reliefReturned>0)
            Preferred=J.battleReview.grand.coalition.rescuedBy;
        const FDialoguePresentation* Personal=nullptr;
        for(const auto& Line:Journey.Episode.Lines) if(Line.Speaker==Preferred && Preferred>=0) { Personal=&Line;break; }
        int Voices=0;FDialoguePresentation Hero;bool HasHero=false;
        for(const auto& Line:Journey.Episode.Lines)
        {
            if(Line.Speaker<0) continue;
            if(Line.bProtagonist) { if(!HasHero) Hero=Line;HasHero=true;continue; }
            if(Voices==0 || (Voices==1 && !Personal)) { Journey.BattleResponse.Add(Line);++Voices; }
        }
        if(Personal && Journey.BattleResponse.Num()==1) Journey.BattleResponse.Add(*Personal);
        if(HasHero) Journey.BattleResponse.Add(Hero);
    }
    if(Journey.bComplete) Copy(frontline::BuildCoalitionEpisode(J),Journey.CoalitionEpisode);
}
