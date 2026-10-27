#include "TacticalTrial.h"
bool ATacticalController::EpisodeInterfaceSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_EPISODE_UI: FAIL %s"),Why);return V; };
    CloseEpisode();Field->PrepareOpeningPreview(false);
    std::vector<std::uint8_t> Before,After;
    if(!Check(Field->ExportSession(Before),TEXT("record the actual council before reading"))) return false;
    OpenEpisode();
    if(!Check(bEpisodeOpen && bInterfaceVisible && EpisodeSnapshot.Lines.Num()>=10 && EpisodeSnapshot.Title.Contains(TEXT("泥の道")),TEXT("council opens the staged ensemble, including the civilian perspective"))) return false;
    HandleEpisodeKey(EKeys::Left);if(!Check(EpisodePage==0,TEXT("reading backward is bounded"))) return false;
    HandleEpisodeKey(EKeys::Enter);
    if(!Check(EpisodePage==0 && NovelTextComplete(),TEXT("first advance reveals the current line instead of skipping it"))) return false;
    HandleEpisodeKey(EKeys::Enter);
    if(!Check(EpisodePage==1 && CurrentEpisodeLine().Name==TEXT("季安") && !CurrentEpisodeLine().Direction.IsEmpty(),TEXT("keyboard advances the cached speaker and stage direction"))) return false;
    for(int I=0;I<30;++I) HandleEpisodeKey(EKeys::SpaceBar);
    if(!Check(!bEpisodeOpen && Field->Presentation().Journey.bPreparation && Field->ExportSession(After) && Before==After,TEXT("finishing/extra keys never deploy, change resources or award trust"))) return false;
    OpenEpisode();HandleEpisodeKey(EKeys::Escape);
    if(!Check(!bEpisodeOpen,TEXT("a scene can always be skipped and reread"))) return false;
    OpenEpisode();ToggleSaveMenu();
    if(!Check(!bEpisodeOpen && bSaveMenuOpen,TEXT("saving replaces the scene rather than stacking input"))) return false;
    OpenEpisode();if(!Check(!bEpisodeOpen,TEXT("the save menu retains exclusive input"))) return false;
    ToggleSaveMenu();
    Field->PrepareBattleStoryPreview(true);OpenEpisode();
    bool Cost=false;for(const auto& L:EpisodeSnapshot.Lines) Cost|=L.Text.Contains(TEXT("余るはず"));
    if(!Check(Cost && Field->Presentation().Journey.bNeedsReply,TEXT("actual high losses change the ensemble aftermath"))) return false;
    for(int I=0;I<30;++I) StepEpisode(1);
    if(!Check(!bEpisodeOpen && Field->Presentation().Journey.bNeedsReply && !Field->CanIssue(ETacticalCommand::Continue),TEXT("reading the aftermath cannot choose the protagonist's reply"))) return false;
    Field->Issue(ETacticalCommand::StoryReplyCoordinate);Field->Issue(ETacticalCommand::Continue);OpenEpisode();
    bool Pledge=false;for(const auto& L:EpisodeSnapshot.Lines) Pledge|=L.Text.Contains(TEXT("二隊以上"));
    if(!Check(Pledge && EpisodeSnapshot.Title.Contains(TEXT("米と旗")),TEXT("the next chapter remembers the real committed vow"))) return false;
    CloseEpisode();
    const bool SavedSmoke=bSmoke;bSmoke=false;EpisodeSeenKey.Reset();
    bool Auto=UpdateEpisodeInput() && bEpisodeOpen;CloseEpisode();
    bool Once=!UpdateEpisodeInput() && !bEpisodeOpen;bSmoke=SavedSmoke;
    if(!Check(Auto && Once,TEXT("a new chapter opens automatically only once; skipping leaves command available"))) return false;
    Field->Issue(ETacticalCommand::Deploy);IgnoreMapInputThroughFrame=Frames-10;
    const int Guard=IgnoreMapInputThroughFrame;
    if(!Check(!UpdateEpisodeInput() && IgnoreMapInputThroughFrame==Guard,TEXT("ordinary battle frames do not extend the map input guard"))) return false;
    OpenEpisode();if(!Check(!bEpisodeOpen,TEXT("story UI does not interrupt active battle or reveal its future"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_EPISODE_UI: PASS (ensemble scenes, actual aftermath, saved vow, keyboard/skip/replay, immutable reading, exclusive save, one-time entry, battle input guard)"));return true;
}
