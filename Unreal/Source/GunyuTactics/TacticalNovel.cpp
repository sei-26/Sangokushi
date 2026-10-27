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
void ATacticalController::AdvanceNovel(float Delta)
{ if(bEpisodeOpen && !bNovelHistory) NovelElapsed=FMath::Min(1000.f,NovelElapsed+FMath::Clamp(Delta,0.f,.25f)); }
void ATacticalController::ReadNextNovel()
{
    if(!bEpisodeOpen || bNovelHistory) return;
    if(!NovelTextComplete()) NovelElapsed=1000;
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
    OpenEpisode();if(!bEpisodeOpen) return;
    const auto& P=Field->Presentation().Journey;
    EpisodeSnapshot.Lines=P.Conversation;EpisodeSnapshot.Title+=TEXT(" / ")+P.Speaker;
}
void ATacticalController::ChooseNovelReply(bool Preserve)
{
    if(!bEpisodeOpen || bNovelHistory || !NovelTextComplete() || EpisodePage+1!=EpisodeSnapshot.Lines.Num()) return;
    const auto Command=Preserve ? ETacticalCommand::StoryReplyPreserve:ETacticalCommand::StoryReplyCoordinate;
    if(!Field->CanIssue(Command)) return;
    Field->Issue(Command);CloseEpisode();
}

FString ATacticalController::NovelHistoryText(int Index) const
{
    if(Index<0 || Index>EpisodePage || !EpisodeSnapshot.Lines.IsValidIndex(Index)) return {};
    return Index==EpisodePage ? VisibleNovelText():EpisodeSnapshot.Lines[Index].Text;
}
