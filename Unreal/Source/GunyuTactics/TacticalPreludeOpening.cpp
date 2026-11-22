#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyPreludeOpening.hpp"
using namespace frontline;

bool ATacticalField::CanChoosePreludePlan() const
{ return CanChoosePreludeOpening(State->Journey) && !IsPlaying(); }
FString ATacticalField::PreludePlanLabel(int Choice) const
{
    if(Choice<0 || Choice>1 || !CanChoosePreludePlan()) return {};
    const auto& P=PreludeOpening(Choice);
    const bool Selected=State->PreparationChoice==Choice && MatchesPreludeOpening(State->Journey,Choice);
    const auto Cost=State->Journey.PreparationCosts(Choice);
    return (Selected ? FString(TEXT("採用中 ／ ")):FString())+JourneyText(P.name)+TEXT("\n")+
        JourneyText(P.roles)+FString::Printf(TEXT(" ／ 兵糧%d"),Cost.TotalFood());
}
bool ATacticalField::ChoosePreludePlan(int Choice)
{
    if(!CanChoosePreludePlan() || !AdoptPreludeOpening(State->Journey,Choice)) return false;
    State->PreparationChoice=Choice;State->Report=JourneyText(State->Journey.message);Refresh();return true;
}
int ATacticalField::PreludeOpeningOfficer() const
{
    return MatchesPreludeOpening(State->Journey,State->PreparationChoice) ? 0:-1;
}
FCouncilStageExchange ATacticalField::ReadPreludeCouncil(int Choice) const
{
    FCouncilStageExchange R;const auto E=PreludeOpeningCouncil(State->Journey,Choice);R.Subject=JourneyText(E.subject);
    for(const auto& Beat:E.beats)
    {
        FCouncilStageLine L;const auto& D=Beat.dialogue;
        L.Dialogue.Speaker=D.speaker;L.Dialogue.Name=JourneyText(EpisodeSpeakerName(D.speaker));
        L.Dialogue.Text=JourneyText(D.text);L.Dialogue.Direction=JourneyText(D.direction);
        L.Dialogue.bProtagonist=D.speaker==3;L.Dialogue.WarmMask=D.warmMask;
        for(int I=0;I<3;++I) L.Reactions[I]=JourneyText(Beat.reactions[I]);
        R.Lines.Add(MoveTemp(L));
    }
    return R;
}
void ATacticalController::ChoosePreludeOpening(int Choice)
{
    if(!CouncilSceneVisible() || !Field->ChoosePreludePlan(Choice)) return;
    CouncilExchange=Field->ReadPreludeCouncil(Choice);CouncilTopic=-1;CouncilPage=0;
    CouncilElapsed=CouncilLineTime=0;CouncilCachePlan=Choice;
    bCouncilCacheRescue=Field->Presentation().Journey.bRescue;bCouncilCacheSouth=Field->Presentation().Journey.Map.bSouthern;
}
