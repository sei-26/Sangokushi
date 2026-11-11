#include "TacticalTrial.h"
bool ATacticalController::CouncilSceneVisible() const
{
    return bEpisodeOpen && bNovelPlan && Field && Field->Presentation().Journey.bPreparation &&
        Field->Presentation().Journey.bCampaignFlow && !bCouncilLedger && !bNovelMap && !bNovelAdvice;
}
bool ATacticalController::IsEnsembleEpisode() const
{
    if(bNovelPersonal || !bEpisodeOpen || NovelOfficer()==4) return false;
    if(bCoalitionEpisode && CurrentEpisodeLine().Speaker==3) return false;
    bool Seen[3]={};for(const auto& L:EpisodeSnapshot.Lines) if(L.Speaker>=0 && L.Speaker<3) Seen[L.Speaker]=true;
    return Seen[0] && Seen[1] && Seen[2];
}
void ATacticalController::OpenCouncilVoice(int Voice,int Decision)
{
    if(!Field || !bEpisodeOpen || !bNovelPlan || !Field->Presentation().Journey.bPreparation || Voice< -1 || Voice>2) return;
    const auto Exchange=Field->ReadCouncilExchange(Voice,Decision);
    if(Exchange.Lines.IsEmpty()) return;
    CouncilExchange=Exchange;CouncilTopic=Voice;CouncilPage=0;CouncilElapsed=CouncilLineTime=0;
    const auto& P=Field->Presentation().Journey;
    CouncilCachePlan=P.Plans.IsValidIndex(1) && P.Plans[1].bSelected ? 1:0;bCouncilCacheRescue=P.bRescue;
}
FDialoguePresentation ATacticalController::CurrentCouncilLine() const
{ return CouncilExchange.Lines.IsValidIndex(CouncilPage) ? CouncilExchange.Lines[CouncilPage].Dialogue:FDialoguePresentation{}; }
FString ATacticalController::VisibleCouncilText() const
{ const auto Text=CurrentCouncilLine().Text;return bNovelInstant ? Text:SafeNovelPrefix(Text,FMath::FloorToInt(CouncilElapsed*48.f)); }
FString ATacticalController::CouncilReaction(int Officer) const
{
    return Officer>=0 && Officer<3 && CouncilExchange.Lines.IsValidIndex(CouncilPage) ? CouncilExchange.Lines[CouncilPage].Reactions[Officer]:FString();
}
void ATacticalController::AdvanceCouncilScene(float Delta)
{
    if(!CouncilSceneVisible() || bSaveMenuOpen) return;
    const auto& P=Field->Presentation().Journey;
    const int Plan=P.Plans.IsValidIndex(1) && P.Plans[1].bSelected ? 1:0;
    if(Plan!=CouncilCachePlan || P.bRescue!=bCouncilCacheRescue) OpenCouncilVoice(CouncilTopic);
    const float Step=FMath::Clamp(Delta,0.f,.25f);
    CouncilElapsed=FMath::Min(1000.f,CouncilElapsed+Step);CouncilStageTime=FMath::Min(1.f,CouncilStageTime+Step);CouncilLineTime+=Step;
    for(int I=0;I<3;++I) CouncilPresence[I]=FMath::FInterpTo(CouncilPresence[I],CurrentCouncilLine().Speaker==I ? 1.f:0.f,Step,9.f);
}
void ATacticalController::NextCouncilLine()
{
    if(!CouncilSceneVisible()) return;
    if(VisibleCouncilText().Len()<CurrentCouncilLine().Text.Len()) { CouncilElapsed=1000;return; }
    if(CouncilPage+1<CouncilExchange.Lines.Num()) { ++CouncilPage;CouncilElapsed=CouncilLineTime=0; }
    // End-of-exchange never deploys or silently adopts a suggestion.
}
void ATacticalController::ChooseCouncilDecision(int Decision)
{
    if(!CouncilSceneVisible()) return;
    const auto& P=Field->Presentation().Journey;
    ETacticalCommand Command;
    if(Decision==1 || Decision==2)
    {
        if(!P.bChoosePromise) return;
        Command=Decision==1 ? ETacticalCommand::PromiseRoad:ETacticalCommand::PromiseRescue;
    }
    else if(Decision==3 || Decision==4) Command=Decision==3 ? ETacticalCommand::PlanAmbush:ETacticalCommand::PlanSupplies;
    else return;
    if(!Field->CanIssue(Command)) return;
    Field->Issue(Command);OpenCouncilVoice(Decision<=2 ? 0:Decision==3 ? 1:2,Decision);
}
FString ATacticalController::CouncilChoiceLabel(int Index) const
{
    const auto& P=Field->Presentation().Journey;
    if(CouncilTopic==0)
    {
        if(P.bChoosePromise) return Index==0 ? TEXT("まず、民の退路を開こう\n先鋒を崩すか、出口を確保する"):TEXT("車が着くまで、俺たちで守る\n護衛600・その兵糧900を追加");
        return Index==0 ? TEXT("そのための配置を、一緒に決めよう"):TEXT("二人の案も、聞かせてくれ");
    }
    if(CouncilTopic<0) return Index==0 ? TEXT("今の配置案を、地図で見せてくれ\n見るだけでは担当を変更しない"):TEXT("準備の案も、聞かせてくれ\n関羽の先手案から聞く");
    if(Index==1) return TEXT("他の意見も聞こう");
    const int Plan=CouncilTopic==1 ? 0:1;
    if(!P.Plans.IsValidIndex(Plan)) return {};
    return (Plan==0 ? FString(TEXT("関羽、先手を頼む")):FString(TEXT("全軍、俺の合図で立て直そう")))+
        FString::Printf(TEXT("\n出陣時：兵糧 %d・軍資金 %d"),P.Plans[Plan].Food,P.Plans[Plan].Gold);
}
void ATacticalController::CouncilChoice(int Index)
{
    if(!CouncilSceneVisible() || Index<0 || Index>1) return;
    if(CouncilTopic<0) { if(Index==0) OpenPlanningExample();else OpenCouncilVoice(1);return; }
    if(CouncilTopic==0)
    {
        if(Field->Presentation().Journey.bChoosePromise) ChooseCouncilDecision(Index==0 ? 1:2);
        else if(Index==0) TogglePlanningMap();else OpenCouncilVoice(-1);
    }
    else if(Index==0) ChooseCouncilDecision(CouncilTopic==1 ? 3:4);
    else OpenCouncilVoice(-1);
}
