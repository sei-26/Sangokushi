#include "TacticalTrial.h"
#include "TacticalChainReview.h"
#include "StrategistJourney.hpp"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
void ATacticalController::PrepareChainReviewCapture()
{
    const bool Signature=FParse::Param(FCommandLine::Get(),TEXT("GunyuChainReviewSignature"));
    Field->FirstMission=0;Field->EnemyPolicy=0;Field->bStartWithCouncil=Signature;Field->bRealtimeCommand=false;
    Field->Issue(ETacticalCommand::Restart);
    if(Signature)
    {
        for(int I=0;I<3;++I) Field->Issue(ETacticalCommand::Dialogue);
        Field->Issue(ETacticalCommand::PromiseRoad);Field->Issue(ETacticalCommand::PlanSupplies);Field->Issue(ETacticalCommand::Deploy);
        const int Dest[]={frontline::Campaign::At(9,11),frontline::Campaign::At(9,9),frontline::Campaign::At(9,10)};
        for(int I=0;I<3;++I) { Field->SelectOfficer(I);Field->Issue(ETacticalCommand::Move);Field->CommandAt(Dest[I]);Field->Issue(ETacticalCommand::Skip); }
        const int Actors[]={0,2,1};const frontline::ChainTactic Tactics[]={frontline::ChainTactic::Oath,frontline::ChainTactic::SerpentRoar,frontline::ChainTactic::AzureDragon};
        for(int I=0;I<3;++I) { Field->SelectOfficer(Actors[I]);Field->SelectTactic(int(Tactics[I]));Field->Issue(ETacticalCommand::QueueTactic); }
    }
    else Field->Issue(ETacticalCommand::Sample);
    if(FParse::Param(FCommandLine::Get(),TEXT("GunyuChainReviewStop"))) Field->EditOrder(2,-1);
    Field->Issue(ETacticalCommand::Execute);Field->Issue(ETacticalCommand::Skip);
    if(FParse::Param(FCommandLine::Get(),TEXT("GunyuChainReviewDebrief")))
    { Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip); }
    ToggleChainReview();
}
bool ATacticalController::ChainReviewInterfaceSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_CHAIN_REVIEW_UI: FAIL %s"),Why);return V; };
    const int SavedMission=Field->FirstMission,SavedPolicy=Field->EnemyPolicy;
    const bool SavedCouncil=Field->bStartWithCouncil,SavedRealtime=Field->bRealtimeCommand;
    Field->FirstMission=0;Field->EnemyPolicy=0;Field->bStartWithCouncil=false;Field->bRealtimeCommand=false;
    Field->Issue(ETacticalCommand::Restart);Field->Issue(ETacticalCommand::Sample);Field->Issue(ETacticalCommand::Execute);
    ToggleChainReview();
    if(!Check(!bChainReviewOpen,TEXT("cannot open the completed review during playback"))) return false;
    Field->Issue(ETacticalCommand::Skip);Field->Issue(ETacticalCommand::RealtimeMode);
    if(Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
    if(!Check(!Field->RealtimePaused(),TEXT("fixture is genuinely running before review opens"))) return false;
    bInterfaceVisible=false;ToggleChainReview();
    if(!Check(bChainReviewOpen && bInterfaceVisible && Field->RealtimePaused() && ChainReviewSnapshot.IsValid() && ChainReviewSnapshot->Rows.Num()==3,TEXT("opening shows actual rows and pauses realtime"))) return false;
    std::vector<std::uint8_t> Before,After;
    if(!Check(Field->ExportSession(Before),TEXT("stable paused snapshot"))) return false;
    CloseChainReview();
    if(!Check(Field->RealtimePaused() && !bChainReviewOpen && !ChainReviewSnapshot.IsValid() && Field->ExportSession(After) && Before==After,TEXT("closing discards the view and retains pause without changing the session"))) return false;
    ToggleChainReview();ToggleSaveMenu();
    if(!Check(bSaveMenuOpen && !bChainReviewOpen && !ChainReviewSnapshot.IsValid(),TEXT("save menu supersedes the review"))) return false;
    ToggleSaveMenu();ToggleChainReview();ToggleBattleLog();
    if(!Check(bBattleLogOpen && !bChainReviewOpen,TEXT("report log supersedes the review"))) return false;
    ToggleBattleLog();ToggleChainReview();ToggleMarchPlan();
    if(!Check(bMarchPlanOpen && !bChainReviewOpen,TEXT("march planner supersedes the review"))) return false;
    ToggleMarchPlan();ToggleChainReview();CloseChainReview();
    Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);ToggleChainReview();
    if(!Check(Field->Presentation().Journey.bDebrief && bChainReviewOpen,TEXT("same review opens from debrief"))) return false;
    CloseChainReview();
    Field->FirstMission=SavedMission;Field->EnemyPolicy=SavedPolicy;Field->bStartWithCouncil=SavedCouncil;Field->bRealtimeCommand=SavedRealtime;Field->Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_CHAIN_REVIEW_UI: PASS (playback guard, paused review, close invariance, mutual modals, debrief, cleared snapshot)"));return true;
}
