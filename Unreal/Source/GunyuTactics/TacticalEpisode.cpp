#include "TacticalTrial.h"
FDialoguePresentation ATacticalController::CurrentEpisodeLine() const
{ return EpisodeSnapshot.Lines.IsValidIndex(EpisodePage) ? EpisodeSnapshot.Lines[EpisodePage]:FDialoguePresentation{}; }
void ATacticalController::OpenEpisode()
{
    if(!Field || Field->IsPlaying() || bSaveMenuOpen || Field->Presentation().Journey.Episode.Lines.IsEmpty()) return;
    EpisodeSnapshot=Field->Presentation().Journey.Episode;EpisodeSeenKey=EpisodeSnapshot.Key;
    EpisodePage=0;NovelElapsed=NovelStageTime=NovelLineTime=0;NovelPreviousSpeaker=-1;bNovelPlan=bNovelPersonal=false;bNovelHistory=false;bEpisodeOpen=true;bInterfaceVisible=true;
    bBattleLogOpen=bMarchPlanOpen=bTacticPickerOpen=false;CloseChainReview();
    IgnoreMapInputThroughFrame=Frames+1;
}
void ATacticalController::CloseEpisode()
{ bEpisodeOpen=false;bNovelPlan=bNovelHistory=false;IgnoreMapInputThroughFrame=Frames+1; }
void ATacticalController::StepEpisode(int Direction)
{
    if(!bEpisodeOpen || bNovelHistory || bNovelPlan) return;
    NovelPreviousSpeaker=CurrentEpisodeLine().Speaker;NovelLineTime=0;NovelElapsed=0;
    if(Direction<0) EpisodePage=FMath::Max(0,EpisodePage-1);
    else if(EpisodePage+1<EpisodeSnapshot.Lines.Num()) ++EpisodePage;
    else CloseEpisode(); // Reading never deploys, changes a promise, advances history or grants trust.
}
void ATacticalController::HandleEpisodeKey(FKey Key)
{
    if(!bEpisodeOpen) return;
    if(bNovelPlan)
    {
        if(Key==EKeys::Escape) bNovelPlan=false;
        else if(Key==EKeys::One) SelectNovelPlan(0);
        else if(Key==EKeys::Two) SelectNovelPlan(1);
        else if(Key==EKeys::D) DeployNovelPlan();
        return;
    }
    if(Key==EKeys::One || Key==EKeys::Two) { ChooseNovelReply(Key==EKeys::One);return; }
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
    if(!bEpisodeOpen && !bSaveMenuOpen && Field->Presentation().Journey.bPreparation && WasInputKeyJustPressed(EKeys::Enter))
    { OpenEpisode();OpenNovelPlan();return true; }
    if(!bEpisodeOpen) return WasOpen || Toggle;
    for(const FKey& Key:{EKeys::Escape,EKeys::L,EKeys::Left,EKeys::Enter,EKeys::SpaceBar,EKeys::Right,EKeys::One,EKeys::Two,EKeys::D})
        if(WasInputKeyJustPressed(Key)) { HandleEpisodeKey(Key);break; }
    return true;
}
