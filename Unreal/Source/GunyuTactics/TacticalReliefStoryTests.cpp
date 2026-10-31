#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
using namespace frontline;
void ATacticalField::PrepareReliefStoryPreview(int Variant)
{
    PrepareReliefPreview(Variant);
    if(Variant==1)
    {
        // Isolate the normal occupied-tile delay from enemy collision in this QA fixture.
        State->Journey.world.armies[3].tile=State->Journey.world.armies[3].target=Campaign::At(13,12);
        State->Journey.world.armies[4].tile=State->Journey.world.armies[4].target=Campaign::At(13,2);
        State->Journey.RefreshIntelligence();Refresh();
    }
    CommitRelief();Issue(ETacticalCommand::Skip);
    if(Variant==1)
    {
        Issue(ETacticalCommand::RealtimeStep);Issue(ETacticalCommand::Skip);
        Issue(ETacticalCommand::RealtimeStep);Issue(ETacticalCommand::Skip);
    }
    else { Issue(ETacticalCommand::EndTurn);Issue(ETacticalCommand::Skip); }
    if(Variant==0)
    {
        SelectOfficer(0);Issue(ETacticalCommand::Hold);Issue(ETacticalCommand::Skip);
        SelectOfficer(1);Issue(ETacticalCommand::Hold);Issue(ETacticalCommand::Skip);
        Issue(ETacticalCommand::EndTurn);Issue(ETacticalCommand::Skip);
    }
    else { Issue(ETacticalCommand::Withdraw);Issue(ETacticalCommand::Withdraw);Issue(ETacticalCommand::Skip); }
}
bool ATacticalController::ReliefStorySmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_RELIEF_STORY: FAIL %s"),Why);return V; };
    CloseEpisode();
    const int Mission=Field->FirstMission,Policy=Field->EnemyPolicy;
    const bool Council=Field->bStartWithCouncil,Realtime=Field->bRealtimeCommand,Legacy=Field->bLegacyJourney;
    auto Read=[this]() { auto S=std::make_unique<JourneySaveData>();std::vector<std::uint8_t> B;if(!Field->ExportSession(B) || !DecodeJourneySave(B,*S)) return std::unique_ptr<JourneySaveData>{};return S; };
    auto Contains=[this](const TCHAR* Text) { return EpisodeSnapshot.Lines.ContainsByPredicate([Text](const FDialoguePresentation& L) { return L.Text.Contains(Text); }); };
    Field->PrepareReliefStoryPreview(0);auto S=Read();
    if(!Check(S && S->journey.won && S->journey.history[2].review.relief.size()==1 && S->journey.history[2].review.relief[0].outcome==JourneyReliefOutcome::Held &&
        Field->Presentation().Journey.StoryFacts.Contains(TEXT("交代直後の更新で守備を確認")),TEXT("actual public handover and subsequent guard commands win with a checked aftermath fact"))) return false;
    Field->Issue(ETacticalCommand::TalkLiu);OpenOfficerNovel();
    if(!Check(bEpisodeOpen && bNovelPersonal && EpisodeSnapshot.Title==TEXT("帰陣の語らい ／ 劉備") && Contains(TEXT("両方に役目")),TEXT("Liu's actual relief reaction reaches the personal novel"))) return false;
    const auto Trust=S->journey.officerTrust;std::vector<std::uint8_t> Before,After;
    Field->ExportSession(Before);BuildInterface();NovelHistoryText(0);VisibleNovelText();Field->ExportSession(After);
    if(!Check(After==Before,TEXT("drawing the conversation and reading its history do not change the saved campaign"))) return false;
    for(int I=0;I<4;++I) { CloseEpisode();Field->Issue(ETacticalCommand::TalkZhang);OpenOfficerNovel(); }
    auto Repeated=Read();
    if(!Check(Repeated && Repeated->journey.officerTrust==Trust && Contains(TEXT("後ろには仲間の旗")),TEXT("Zhang remembers leaving under a comrade's cover; repeated reading grants no trust"))) return false;
    CloseEpisode();Field->Issue(ETacticalCommand::TalkGuan);OpenOfficerNovel();
    if(!Check(!Contains(TEXT("双方の責")),TEXT("an officer outside the explicit operation receives no invented participation"))) return false;
    Field->ExportSession(Before);CloseEpisode();
    if(!Check(Field->ImportSession(Before) && Field->Presentation().Journey.StoryFacts.Contains(TEXT("交代直後")),TEXT("settled evidence survives an actual field save/load"))) return false;
    Field->PrepareReliefStoryPreview(2);S=Read();Field->Issue(ETacticalCommand::TalkLiu);OpenOfficerNovel();
    if(!Check(S && S->journey.history[2].review.relief.size()==1 && S->journey.history[2].review.relief[0].outcome==JourneyReliefOutcome::Lost &&
        Field->Presentation().Journey.StoryFacts.Contains(TEXT("守備条件を満たせなかった")) && Contains(TEXT("人を入れれば守れる")) && !Contains(TEXT("両方に役目")),TEXT("actual exposed arrival gives failure facts and reflection instead of success praise"))) return false;
    CloseEpisode();Field->PrepareReliefStoryPreview(1);S=Read();Field->Issue(ETacticalCommand::TalkZhang);OpenOfficerNovel();
    if(!Check(S && S->journey.history[2].review.relief.size()==1 && S->journey.history[2].review.relief[0].outcome==JourneyReliefOutcome::Recovered && S->journey.history[2].review.relief[0].gapCycles==1 &&
        Field->Presentation().Journey.StoryFacts.Contains(TEXT("1回の空白後")) && Contains(TEXT("待ってる間")),TEXT("real RT occupancy gap and later arrival reach the aftermath and Zhang's conversation"))) return false;
    CloseEpisode();Field->FirstMission=Mission;Field->EnemyPolicy=Policy;Field->bStartWithCouncil=Council;Field->bRealtimeCommand=Realtime;Field->bLegacyJourney=Legacy;
    Field->Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_RELIEF_STORY: PASS (public moves/guard/result, true held/lost/recovered facts, personalized novel, immutable reading and save/load)"));return true;
}
