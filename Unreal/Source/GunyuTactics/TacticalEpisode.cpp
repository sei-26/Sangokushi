#include "TacticalTrial.h"
FDialoguePresentation ATacticalController::CurrentEpisodeLine() const
{ return EpisodeSnapshot.Lines.IsValidIndex(EpisodePage) ? EpisodeSnapshot.Lines[EpisodePage]:FDialoguePresentation{}; }
void ATacticalController::OpenEpisode()
{
    if(!Field || Field->IsPlaying() || bSaveMenuOpen || Field->Presentation().Journey.Episode.Lines.IsEmpty()) return;
    EpisodeSnapshot=Field->Presentation().Journey.Episode;EpisodeSeenKey=EpisodeSnapshot.Key;
    EpisodePage=0;bEpisodeOpen=true;bInterfaceVisible=true;
    bBattleLogOpen=bMarchPlanOpen=bTacticPickerOpen=false;CloseChainReview();
    IgnoreMapInputThroughFrame=Frames+1;
}
void ATacticalController::CloseEpisode()
{ bEpisodeOpen=false;IgnoreMapInputThroughFrame=Frames+1; }
void ATacticalController::StepEpisode(int Direction)
{
    if(!bEpisodeOpen) return;
    if(Direction<0) EpisodePage=FMath::Max(0,EpisodePage-1);
    else if(EpisodePage+1<EpisodeSnapshot.Lines.Num()) ++EpisodePage;
    else CloseEpisode(); // Reading never deploys, changes a promise, advances history or grants trust.
}
void ATacticalController::HandleEpisodeKey(FKey Key)
{
    if(!bEpisodeOpen) return;
    if(Key==EKeys::Escape) CloseEpisode();
    else if(Key==EKeys::Left) StepEpisode(-1);
    else if(Key==EKeys::Enter || Key==EKeys::SpaceBar || Key==EKeys::Right) StepEpisode(1);
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
    for(const FKey& Key:{EKeys::Escape,EKeys::Left,EKeys::Enter,EKeys::SpaceBar,EKeys::Right})
        if(WasInputKeyJustPressed(Key)) { HandleEpisodeKey(Key);break; }
    return true;
}
