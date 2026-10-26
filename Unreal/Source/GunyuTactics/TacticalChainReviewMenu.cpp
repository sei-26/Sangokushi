#include "TacticalTrial.h"
#include "TacticalChainReview.h"
void ATacticalController::CloseChainReview()
{
    bChainReviewOpen=false;ChainReviewSnapshot.Reset();IgnoreMapInputThroughFrame=Frames+1;
}
void ATacticalController::ToggleChainReview()
{
    if(bChainReviewOpen) { CloseChainReview();return; }
    if(!Field || bSaveMenuOpen || !Field->HasChainReview()) return;
    if(Field->bRealtimeCommand && !Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
    auto Review=Field->ReadChainReview();if(!Review.bAvailable) return;
    ChainReviewSnapshot=MakeShared<FChainReview>(MoveTemp(Review));bChainReviewOpen=true;
    bInterfaceVisible=true;bBattleLogOpen=bMarchPlanOpen=bTacticPickerOpen=false;
    IgnoreMapInputThroughFrame=Frames+1;
}
void ATacticalController::UpdateChainReviewInput()
{
    if(WasInputKeyJustPressed(EKeys::Y) && !WasInputKeyJustPressed(EKeys::F6) && !bSaveMenuOpen) ToggleChainReview();
    if(bChainReviewOpen && (WasInputKeyJustPressed(EKeys::Escape) || !Field->HasChainReview())) CloseChainReview();
    if(bChainReviewOpen && Field->bRealtimeCommand && !Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
}
