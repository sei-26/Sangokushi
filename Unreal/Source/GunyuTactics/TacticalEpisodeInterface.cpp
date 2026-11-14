#include "TacticalTrial.h"
#include "JourneyEpisode.hpp"
#include "JourneyMission.hpp"
#include "TacticalWidgets.h"
#include "TacticalNovelLayout.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
using namespace gunyuui;
void ATacticalController::BuildEpisodeInterface(TSharedRef<SCanvas> Canvas)
{
    auto Visible=[this] { return bEpisodeOpen ? EVisibility::Visible:EVisibility::Collapsed; };
    auto DialogueVisible=[this] { return bEpisodeOpen && !bNovelPlan ? EVisibility::Visible:EVisibility::Collapsed; };
    auto Click=[this](const FGeometry&,const FPointerEvent& Event)
    { if(Event.GetEffectingButton()==EKeys::LeftMouseButton) { ReadNextNovel();return FReply::Handled(); }return FReply::Unhandled(); };
    auto Backdrop=SNew(SBorder).Padding(0).BorderImage(FCoreStyle::Get().GetBrush("NoBrush")).Visibility_Lambda(Visible)
        .OnMouseButtonDown_Lambda(Click)[NovelBackdropWidget([this] { return !Field->Presentation().Journey.bPreparation; })];
    Canvas->AddSlot().Position(FVector2D::ZeroVector).Size(FVector2D(1600,900))[Backdrop];InputPanels.Add(Backdrop);
    auto HistoricalSpeaker=[this] { return NovelOfficer()>=frontline::ZhangJiao; };
    // Shared framing keeps full-body faces below the header in both Editor and cooked builds.
    for(int Slot=0;Slot<3;++Slot)
    {
        const bool FullBody=Slot!=1;
        const float W=FullBody ? gunyunovel::FullBodyWidth:650.f,H=FullBody ? gunyunovel::FullBodyHeight:975.f;
        auto Portrait=SNew(SBox).Visibility_Lambda([this,Slot,HistoricalSpeaker]
            { const bool Match=Slot==0 ? CurrentEpisodeLine().Speaker<frontline::ZhangJiao:
                  Slot==1 ? NovelOfficer()!=4 && !HistoricalSpeaker():NovelOfficer()==4;
              return bEpisodeOpen && !bNovelPlan && !IsEnsembleEpisode() && Match && CurrentEpisodeLine().Speaker>=0 ? EVisibility::HitTestInvisible:EVisibility::Collapsed; })
            [SNew(SBorder).Padding(0).BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
             [NovelPortraitWidget([this,Slot] { return Slot==0 ? 3:NovelOfficer(); },W,H)]];
        if(Slot>0) { Portrait->SetRenderTransformPivot(FVector2D(.5,.5));Portrait->SetRenderTransform(FSlateRenderTransform(FScale2D(-1,1))); }
        const FVector2D Position=Slot==0 ? FVector2D(-30,gunyunovel::FullBodyTop):Slot==1 ? FVector2D(850,60):FVector2D(640,gunyunovel::FullBodyTop);
        Canvas->AddSlot().Position(Position).Size(FVector2D(W,H))[Portrait];
    }
    auto Guest=SNew(SBox).Visibility_Lambda([this,HistoricalSpeaker]
        { return bEpisodeOpen && !bNovelPlan && HistoricalSpeaker() && CurrentEpisodeLine().Speaker>=0 ? EVisibility::HitTestInvisible:EVisibility::Collapsed; })
        [NovelPortraitWidget([this] { return NovelOfficer(); },478.f,478.f)];
    Canvas->AddSlot().Position(FVector2D(950,137)).Size(FVector2D(478,478))[Guest];
    BuildEnsemblePortraits(Canvas,false);
    auto Header=SNew(STacticalChrome).Padding(FMargin(22,12)).Opacity(.86f).Visibility_Lambda(DialogueVisible)
        [SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[Label([this] { return EpisodeSnapshot.Place; },12,Gold)]
         + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Label([this] { return EpisodeSnapshot.Title; },25)]];
    Canvas->AddSlot().Position(FVector2D(40,28)).Size(FVector2D(820,88))[Header];InputPanels.Add(Header);
    auto Caption=SNew(SBox).Visibility_Lambda([this] { return bEpisodeOpen && !bNovelPlan && CurrentEpisodeLine().Speaker<0 && !(EpisodePage+1==EpisodeSnapshot.Lines.Num() && Field->Presentation().Journey.bNeedsReply) ? EVisibility::HitTestInvisible:EVisibility::Collapsed; })
        [SNew(STacticalChrome).Padding(24).Opacity(.72f)
         [SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[Label([this] { return EpisodeSnapshot.Question; },27)]
          + SVerticalBox::Slot().AutoHeight().Padding(0,14)[Label([this] { return EpisodeSnapshot.Setting; },17,Ink)]]];
    Canvas->AddSlot().Position(FVector2D(108,280)).Size(FVector2D(620,218))[Caption];
    auto Dialogue=SNew(SBorder).Padding(0).BorderImage(FCoreStyle::Get().GetBrush("NoBrush")).Visibility_Lambda(DialogueVisible).OnMouseButtonDown_Lambda(Click)
        [SNew(STacticalChrome).Padding(FMargin(34,20)).Opacity(.90f).Accent(gunyustyle::Brass)
         [SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
             + SHorizontalBox::Slot().FillWidth(1)[Label([this] { return CurrentEpisodeLine().Name; },29,Gold)]
             + SHorizontalBox::Slot().AutoWidth()[Label([this] { return FString::Printf(TEXT("%02d / %02d"),EpisodePage+1,EpisodeSnapshot.Lines.Num()); },12,Gold)]]
          + SVerticalBox::Slot().FillHeight(1).Padding(0,13)[Label([this] { return VisibleNovelText(); },28)]
          + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
             + SHorizontalBox::Slot().FillWidth(1)[Label([this] { return CurrentEpisodeLine().Direction; },15,FLinearColor(.78f,.72f,.61f))]
             + SHorizontalBox::Slot().AutoWidth().Padding(16,0)[Label([this] { return NovelTextComplete() ? TEXT("▼"):TEXT(""); },17,Gold)]]]];
    Canvas->AddSlot().Position(FVector2D(64,610)).Size(FVector2D(1472,226))[Dialogue];InputPanels.Add(Dialogue);
    auto Controls=SNew(SBox).Visibility_Lambda([this] { return bEpisodeOpen && !bNovelPlan ? EVisibility::Visible:EVisibility::Collapsed; })[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("履歴 / L"); },[this] { bNovelHistory=true; })]
        + SHorizontalBox::Slot().AutoWidth().Padding(12,0)[Button([] { return TEXT("前へ / ←"); },[this] { StepEpisode(-1);NovelElapsed=1000; },[this] { return EpisodePage>0 && !bNovelHistory; })]
        + SHorizontalBox::Slot().AutoWidth()[Button([this] { return bNovelInstant ? TEXT("文字：一括"):TEXT("文字：ゆっくり"); },[this] { bNovelInstant=!bNovelInstant; })]
        + SHorizontalBox::Slot().FillWidth(1)
        + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("閉じる / Esc"); },[this] { CloseEpisode(); })]
        + SHorizontalBox::Slot().AutoWidth().Padding(18,0,0,0)[SNew(SBox).WidthOverride(280)
            [Button([this]
            {
                if(!NovelTextComplete()) return TEXT("全文を表示 / Enter");
                if(EpisodePage+1<EpisodeSnapshot.Lines.Num()) return TEXT("続きを読む / Enter");
                if(CanChooseCoalitionFromNovel()) return TEXT("協力先を選ぶ / 1・2");
                if(CanChooseLuoyangFromNovel()) return TEXT("引き受ける任務を選ぶ");
                if(Field->Presentation().Journey.bComplete && !bNovelPersonal && Field->CanIssue(ETacticalCommand::Continue))
                    return Field->Presentation().Mission==frontline::CoalitionMission ? TEXT("陽人の戦いへ / Enter"):TEXT("黄巾の乱へ / Enter");
                if(CanOpenCoalitionFromEpilogue()) return TEXT("反董卓連合へ / Enter");
                if(Field->Presentation().Journey.bPreparation && !bNovelPersonal) return TEXT("皆と策を決める / Enter");
                return Field->Presentation().Journey.bComplete ? TEXT("章の記録へ戻る / Enter"):TEXT("采配へ戻る / Enter");
            },[this] { ReadNextNovel(); },[this] { return !CanChooseCoalitionFromNovel() && !CanChooseLuoyangFromNovel(); })]]];
    Canvas->AddSlot().Position(FVector2D(64,850)).Size(FVector2D(1472,34))[Controls];InputPanels.Add(Controls);
    auto Choices=SNew(SVerticalBox).Visibility_Lambda([this]
        { return bEpisodeOpen && !bNovelHistory && !bNovelPlan && NovelTextComplete() && EpisodePage+1==EpisodeSnapshot.Lines.Num() && Field->Presentation().Journey.bNeedsReply ? EVisibility::Visible:EVisibility::Collapsed; })
        + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,12)
          [Button([] { return TEXT("1　次は、みんなを連れて帰る\n兵の損失を一割以内に抑え、任務を果たす"); },[this] { ChooseNovelReply(true); })]
        + SVerticalBox::Slot().AutoHeight()[Button([] { return TEXT("2　俺が合図を出して、皆をつなぐ\n二隊以上の戦法をつなぎ、任務を果たす"); },[this] { ChooseNovelReply(false); })];
    Canvas->AddSlot().Position(FVector2D(460,370)).Size(FVector2D(680,180))[Choices];InputPanels.Add(Choices);
    auto CoalitionChoices=SNew(SHorizontalBox).Visibility_Lambda([this]
        { return CanChooseCoalitionFromNovel() || CanChooseLuoyangFromNovel() ? EVisibility::Visible:EVisibility::Collapsed; });
    for(int Index=0;Index<2;++Index)
        CoalitionChoices->AddSlot().FillWidth(1).Padding(Index==0 ? FMargin(0,0,10,0):FMargin(10,0,0,0))
            [Button([this,Index]
                {
                    const auto& J=Field->Presentation().Journey;
                    return CanChooseLuoyangFromNovel() ? J.LuoyangChoices[Index]:J.CoalitionChoices[Index];
                },[this,Index] { ChooseNovelReply(Index==0); },
                [this,Index] { return Index==0 || !CanChooseLuoyangFromNovel() || Field->CanIssue(ETacticalCommand::LuoyangDepot); })];
    Canvas->AddSlot().Position(FVector2D(64,492)).Size(FVector2D(1472,98))[CoalitionChoices];InputPanels.Add(CoalitionChoices);
    BuildNovelHistory(Canvas);
    BuildNovelPlan(Canvas);
    BuildCouncilScene(Canvas);
}
