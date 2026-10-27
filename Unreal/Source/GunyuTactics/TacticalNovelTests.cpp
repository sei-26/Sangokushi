#include "TacticalTrial.h"
#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
bool ATacticalController::NovelInterfaceSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_NOVEL_UI: FAIL %s"),Why);return V; };
    CloseEpisode();Field->PrepareOpeningPreview(false);bNovelInstant=false;OpenEpisode();
    if(!Check(NovelBackgrounds.Num()==2 && NovelBackgrounds[0] && NovelBackgrounds[1] && CutinTextures.Num()==5 && CutinTextures[3] && CutinTextures[4],TEXT("both original backgrounds, civilian and protagonist are available"))) return false;
    if(!Check(IFileManager::Get().FileExists(*(FPaths::ProjectContentDir()/TEXT("UI/Fonts/BIZUDMincho-Regular.ttf"))),TEXT("the redistributable literary font ships with its source"))) return false;
    std::vector<std::uint8_t> Before,After;if(!Check(Field->ExportSession(Before),TEXT("council snapshot"))) return false;
    if(!Check(VisibleNovelText().IsEmpty() && NovelHistoryText(1).IsEmpty(),TEXT("typing and history do not reveal unread future lines"))) return false;
    AdvanceNovel(.1f);
    if(!Check(!VisibleNovelText().IsEmpty() && !NovelTextComplete(),TEXT("text reveals gradually"))) return false;
    HandleEpisodeKey(EKeys::L);const float Held=NovelElapsed;AdvanceNovel(.25f);HandleEpisodeKey(EKeys::Enter);
    if(!Check(bNovelHistory && NovelElapsed==Held && EpisodePage==0,TEXT("history pauses typing and owns advance input"))) return false;
    HandleEpisodeKey(EKeys::Escape);HandleEpisodeKey(EKeys::Enter);
    if(!Check(!bNovelHistory && EpisodePage==0 && NovelTextComplete(),TEXT("escape returns to the same line; next click completes it"))) return false;
    HandleEpisodeKey(EKeys::Enter);if(!Check(EpisodePage==1 && NovelOfficer()==4,TEXT("Ji An uses the civilian portrait"))) return false;
    FString Unicode=TEXT("策");Unicode.AppendChar(0xd83d);Unicode.AppendChar(0xde00);Unicode+=TEXT("道");
    if(!Check(SafeNovelPrefix(Unicode,2)==TEXT("策") && SafeNovelPrefix(Unicode,3).Len()==3,TEXT("typing never exposes half a UTF-16 surrogate pair"))) return false;
    CloseEpisode();Field->Issue(ETacticalCommand::TalkGuan);OpenOfficerNovel();
    bool Hero=false;for(const auto& L:EpisodeSnapshot.Lines) Hero|=L.Speaker==3;
    if(!Check(bEpisodeOpen && Hero && EpisodeSnapshot.Lines.Num()>=4 && Field->ExportSession(After),TEXT("officer selection opens its actual cached conversation"))) return false;
    // Officer selection itself is saved UI state; reading must leave that state byte-identical.
    if(!Check(Field->ExportSession(Before),TEXT("officer UI snapshot"))) return false;
    for(int I=0;I<30;++I) ReadNextNovel();
    if(!Check(Field->ExportSession(After) && Before==After,TEXT("typing, replay and log do not alter game data"))) return false;
    Field->PrepareBattleStoryPreview(false);OpenEpisode();
    ChooseNovelReply(true);if(!Check(Field->Presentation().Journey.bNeedsReply,TEXT("reply cannot be chosen before its real decision point"))) return false;
    while(bEpisodeOpen && EpisodePage+1<EpisodeSnapshot.Lines.Num()) StepEpisode(1);
    NovelElapsed=1000;ChooseNovelReply(false);
    if(!Check(!bEpisodeOpen && !Field->Presentation().Journey.bNeedsReply && Field->CanIssue(ETacticalCommand::Continue),TEXT("the novel choice commits the existing protagonist vow"))) return false;
    Field->Issue(ETacticalCommand::Continue);OpenEpisode();
    bool Pledge=false;for(const auto& L:EpisodeSnapshot.Lines) Pledge|=L.Text.Contains(TEXT("二隊以上"));
    if(!Check(Pledge,TEXT("the choice remains part of the actual next council"))) return false;
    CloseEpisode();UE_LOG(LogTemp,Display,TEXT("GUNYU_NOVEL_UI: PASS (original art, hero and civilian, literary font, typing/surrogates, input/history, personal conversations, immutable reads, real vow)"));return true;
}
