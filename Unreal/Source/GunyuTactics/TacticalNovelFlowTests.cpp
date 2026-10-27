#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include "Engine/Texture2D.h"
#include <memory>
bool ATacticalController::NovelFlowSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_NOVEL_FLOW: FAIL %s"),Why);return V; };
    CloseEpisode();Field->PrepareOpeningPreview(false);bNovelInstant=false;OpenEpisode();
    if(!Check(ConversationTextures.Num()==3 && ConversationTextures[0] && ConversationTextures[1] && ConversationTextures[2],TEXT("three conversation poses are loaded"))) return false;
    for(int I=0;I<3;++I)
        if(!Check(NovelPortraitBrush(I)==ConversationBrushes[I].Get() && NovelPortraitBrush(I)!=CutinBrushes[I].Get(),TEXT("quiet dialogue uses its own artwork while battle cut-ins are retained"))) return false;
    if(!Check(NovelPortraitTone(3).A==0,TEXT("a scene starts with a gentle portrait entrance"))) return false;
    AdvanceNovel(.25f);AdvanceNovel(.25f);
    if(!Check(NovelPortraitTone(3).A==1 && NovelStageTime>0,TEXT("entrance settles without advancing game time"))) return false;
    StepEpisode(1);AdvanceNovel(.25f);
    if(!Check(NovelPortraitTone(4).R>NovelPortraitTone(3).R,TEXT("the current speaker is lit while the protagonist listens"))) return false;
    auto Before=std::make_unique<frontline::JourneySaveData>(),After=std::make_unique<frontline::JourneySaveData>();
    std::vector<std::uint8_t> BeforeBytes,AfterBytes;
    if(!Check(Field->ExportSession(BeforeBytes) && frontline::DecodeJourneySave(BeforeBytes,*Before),TEXT("record actual preparation before reviewing"))) return false;
    while(EpisodePage+1<EpisodeSnapshot.Lines.Num()) StepEpisode(1);
    NovelElapsed=1000;ReadNextNovel();
    if(!Check(bEpisodeOpen && bNovelPlan && Field->ExportSession(AfterBytes) && BeforeBytes==AfterBytes,TEXT("story ending opens a review without spending or changing a promise"))) return false;
    const float Held=NovelElapsed;HandleEpisodeKey(EKeys::Enter);AdvanceNovel(.25f);
    if(!Check(bNovelPlan && NovelElapsed==Held && Field->Presentation().Journey.bPreparation,TEXT("another advance key does not accidentally deploy"))) return false;
    HandleEpisodeKey(EKeys::Two);
    if(!Check(Field->Presentation().Journey.Plans[1].bSelected && Field->ExportSession(AfterBytes) && frontline::DecodeJourneySave(AfterBytes,*After),TEXT("numeric selection changes the real preparation"))) return false;
    const auto Camp=Before->journey.world.cities[0];const auto SelectedCamp=After->journey.world.cities[0];
    if(!Check(Camp.troops==SelectedCamp.troops && Camp.food==SelectedCamp.food && Camp.gold==SelectedCamp.gold,TEXT("review and choosing a plan do not pay deployment costs"))) return false;
    const auto Cost=After->journey.PreparationCosts(1);
    HandleEpisodeKey(EKeys::D);
    if(!Check(!bEpisodeOpen && Field->Presentation().bBattle && Field->ExportSession(AfterBytes) && frontline::DecodeJourneySave(AfterBytes,*After),TEXT("an explicit decision deploys using the current plan"))) return false;
    if(!Check(After->journey.preparation==1 && After->journey.world.cities[0].food==Camp.food-Cost.TotalFood() && After->journey.world.cities[0].gold==Camp.gold-Cost.gold,TEXT("actual resource accounting matches the confirmed plan"))) return false;
    const auto Paid=AfterBytes;HandleEpisodeKey(EKeys::D);DeployNovelPlan();
    if(!Check(Field->ExportSession(AfterBytes) && AfterBytes==Paid,TEXT("a second input cannot pay or deploy again"))) return false;
    Field->PrepareOpeningPreview(false);Field->Issue(ETacticalCommand::TalkGuan);OpenOfficerNovel();
    for(int I=0;I<30;++I) ReadNextNovel();
    if(!Check(!bEpisodeOpen && !bNovelPlan,TEXT("personal conversations close normally without opening an operation review"))) return false;
    Field->PrepareBattleStoryPreview(false);OpenEpisode();
    HandleEpisodeKey(EKeys::Two);if(!Check(Field->Presentation().Journey.bNeedsReply,TEXT("a reply shortcut is inactive before its decision point"))) return false;
    while(EpisodePage+1<EpisodeSnapshot.Lines.Num()) StepEpisode(1);
    NovelElapsed=1000;HandleEpisodeKey(EKeys::Two);
    if(!Check(!bEpisodeOpen && !Field->Presentation().Journey.bNeedsReply && Field->CanIssue(ETacticalCommand::Continue),TEXT("numbered story choices use the real once-only vow"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_NOVEL_FLOW: PASS (conversation portraits, entrance and focus, immutable plan review, numeric plan/vow, explicit deployment, actual costs, one-shot input, personal dialogue)"));return true;
}
