#include "TacticalTrial.h"
FString ATacticalController::SafeNovelPrefix(const FString& Text,int Count)
{
    Count=FMath::Clamp(Count,0,Text.Len());
    if(Count>0 && Count<Text.Len() && Text[Count-1]>=0xd800 && Text[Count-1]<=0xdbff) --Count;
    return Text.Left(Count);
}
FString ATacticalController::VisibleNovelText() const
{ const auto Text=CurrentEpisodeLine().Text;return bNovelInstant ? Text:SafeNovelPrefix(Text,FMath::FloorToInt(NovelElapsed*36.f)); }
bool ATacticalController::NovelTextComplete() const
{ return VisibleNovelText().Len()==CurrentEpisodeLine().Text.Len(); }
bool ATacticalController::CanChooseCoalitionFromNovel() const
{
    return bEpisodeOpen && bCoalitionEpisode && !bNovelHistory && !bNovelPlan &&
        EpisodePage+1==EpisodeSnapshot.Lines.Num() && NovelTextComplete() && Field->CanIssue(ETacticalCommand::CoalitionAdvance);
}
bool ATacticalController::CanChooseLuoyangFromNovel() const
{
    return bEpisodeOpen && !bNovelPersonal && !bNovelHistory && !bNovelPlan &&
        EpisodePage+1==EpisodeSnapshot.Lines.Num() && NovelTextComplete() && Field->CanIssue(ETacticalCommand::LuoyangGate);
}
void ATacticalController::AdvanceNovel(float Delta)
{
    AdvanceCouncilScene(Delta);
    if(bEpisodeOpen && !bNovelHistory && !bNovelPlan)
    { const float Step=FMath::Clamp(Delta,0.f,.25f);NovelElapsed=FMath::Min(1000.f,NovelElapsed+Step);NovelStageTime=FMath::Min(1.f,NovelStageTime+Step);NovelLineTime=FMath::Min(1.f,NovelLineTime+Step); }
}
void ATacticalController::ReadNextNovel()
{
    if(!bEpisodeOpen || bNovelHistory || bNovelPlan) return;
    if(CanChooseCoalitionFromNovel() || CanChooseLuoyangFromNovel()) return;
    if(!NovelTextComplete()) NovelElapsed=1000;
    else if(EpisodePage+1==EpisodeSnapshot.Lines.Num() && !bNovelPersonal &&
        Field->Presentation().Journey.bComplete && Field->CanIssue(ETacticalCommand::Continue))
    {
        Field->Issue(ETacticalCommand::Continue);
        OpenJourneyEntry();
    }
    else if(EpisodePage+1==EpisodeSnapshot.Lines.Num() && CanOpenCoalitionFromEpilogue()) OpenEpisode(true);
    else if(EpisodePage+1==EpisodeSnapshot.Lines.Num() && !bNovelPersonal && Field->Presentation().Journey.bPreparation) OpenNovelPlan();
    else StepEpisode(1);
}
int ATacticalController::NovelOfficer() const
{
    for(int I=EpisodePage;I>=0;--I)
        if(EpisodeSnapshot.Lines.IsValidIndex(I))
        { const int S=EpisodeSnapshot.Lines[I].Speaker;if(S>=0 && S!=3) return S; }
    return 0;
}
void ATacticalController::OpenOfficerNovel()
{
    if(!Field || Field->Presentation().Journey.Conversation.IsEmpty()) return;
    OpenEpisode();if(!bEpisodeOpen) return;bNovelPersonal=true;
    const auto& P=Field->Presentation().Journey;
    EpisodeSnapshot.Lines=P.Conversation;
    EpisodeSnapshot.Title=(P.bPreparation ? FString(TEXT("軍議の語らい ／ ")):FString(TEXT("帰陣の語らい ／ ")))+P.Speaker;
}
void ATacticalController::ChooseNovelReply(bool bFirst)
{
    if(!bEpisodeOpen || bNovelHistory || bNovelPlan || !NovelTextComplete() || EpisodePage+1!=EpisodeSnapshot.Lines.Num()) return;
    if(CanChooseCoalitionFromNovel())
    {
        Field->Issue(bFirst ? ETacticalCommand::CoalitionAdvance:ETacticalCommand::CoalitionProtect);
        if(Field->Presentation().Journey.bPreparation) OpenJourneyEntry();
        return;
    }
    if(CanChooseLuoyangFromNovel())
    {
        const auto Command=bFirst ? ETacticalCommand::LuoyangGate:ETacticalCommand::LuoyangDepot;
        if(Field->CanIssue(Command)) { Field->Issue(Command);OpenJourneyEntry(); }
        return;
    }
    const auto Command=bFirst ? ETacticalCommand::StoryReplyPreserve:ETacticalCommand::StoryReplyCoordinate;
    if(!Field->CanIssue(Command)) return;
    Field->Issue(Command);CloseEpisode();
}

FString ATacticalController::NovelHistoryText(int Index) const
{
    if(Index<0 || Index>EpisodePage || !EpisodeSnapshot.Lines.IsValidIndex(Index)) return {};
    return Index==EpisodePage ? VisibleNovelText():EpisodeSnapshot.Lines[Index].Text;
}
