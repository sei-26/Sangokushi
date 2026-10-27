#include "TacticalTrial.h"
FDialoguePresentation ATacticalController::CurrentEpisodeLine() const
{ return EpisodeSnapshot.Lines.IsValidIndex(EpisodePage) ? EpisodeSnapshot.Lines[EpisodePage]:FDialoguePresentation{}; }
void ATacticalController::OpenEpisode()
{
    if(!Field || Field->IsPlaying() || bSaveMenuOpen || Field->Presentation().Journey.Episode.Lines.IsEmpty()) return;
    EpisodeSnapshot=Field->Presentation().Journey.Episode;EpisodeSeenKey=EpisodeSnapshot.Key;
    EpisodePage=0;NovelElapsed=0;bNovelHistory=false;bEpisodeOpen=true;bInterfaceVisible=true;
    bBattleLogOpen=bMarchPlanOpen=bTacticPickerOpen=false;CloseChainReview();
    IgnoreMapInputThroughFrame=Frames+1;
}
void ATacticalController::CloseEpisode()
{ bEpisodeOpen=false;bNovelHistory=false;IgnoreMapInputThroughFrame=Frames+1; }
void ATacticalController::StepEpisode(int Direction)
{
    if(!bEpisodeOpen || bNovelHistory) return;
    NovelElapsed=0;
    if(Direction<0) EpisodePage=FMath::Max(0,EpisodePage-1);
    else if(EpisodePage+1<EpisodeSnapshot.Lines.Num()) ++EpisodePage;
    else CloseEpisode(); // Reading never deploys, changes a promise, advances history or grants trust.
}
void ATacticalController::HandleEpisodeKey(FKey Key)
{
    if(!bEpisodeOpen) return;
    if(Key==EKeys::L) { bNovelHistory=!bNovelHistory;return; }
    if(Key==EKeys::Escape) { if(bNovelHistory) bNovelHistory=false;else CloseEpisode();return; }
    if(bNovelHistory) return;
    if(Key==EKeys::Left) { if(EpisodePage>0) { StepEpisode(-1);NovelElapsed=1000; } }
    else if(Key==EKeys::Enter || Key==EKeys::SpaceBar || Key==EKeys::Right) ReadNextNovel();
}
bool ATacticalController::UpdateEpisodeInput()
{
    const auto& E=Field->Presentation().Journey.Episode;
    const bool WasOpen=bEpisodeOpen;
    if(E.Key.IsEmpty()) { if(bEpisodeOpen) CloseEpisode();EpisodeSeenKey.Reset();return WasOpen; }
    if(bEpisodeOpen && EpisodeSnapshot.Key!=E.Key) CloseEpisode();
    // Show each newly entered council/aftermath once. QA captures opt in explicitly.
    if(!bCapture && !bSmoke && !bSaveMenuOpen && E.Key!=EpisodeSeenKey) OpenEpisode();
    const bool Toggle=WasInputKeyJustPressed(EKeys::H) && !bSaveMenuOpen;
    if(Toggle) { if(bEpisodeOpen) CloseEpisode();else OpenEpisode(); }
    if(!bEpisodeOpen) return WasOpen || Toggle;
    for(const FKey& Key:{EKeys::Escape,EKeys::L,EKeys::Left,EKeys::Enter,EKeys::SpaceBar,EKeys::Right})
        if(WasInputKeyJustPressed(Key)) { HandleEpisodeKey(Key);break; }
    return true;
}
