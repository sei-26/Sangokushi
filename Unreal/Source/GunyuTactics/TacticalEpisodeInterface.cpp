#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
using namespace gunyuui;
void ATacticalController::BuildEpisodeInterface(TSharedRef<SCanvas> Canvas)
{
    auto Visible=[this] { return bEpisodeOpen ? EVisibility::Visible:EVisibility::Collapsed; };
    auto Shade=SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(.006f,.009f,.014f,1.f)).Visibility_Lambda(Visible);
    Canvas->AddSlot().Position(FVector2D::ZeroVector).Size(FVector2D(1600,900))[Shade];InputPanels.Add(Shade);
    auto Title=SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[Label([this] { return EpisodeSnapshot.Place; },15,Gold)]
        + SVerticalBox::Slot().AutoHeight().Padding(0,8,0,0)[Label([this] { return EpisodeSnapshot.Title; },40)];
    auto Header=SNew(SBox).Visibility_Lambda(Visible)[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1)[Title]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
          [Button([] { return TEXT("場面を閉じる / Esc"); },[this] { CloseEpisode(); })]];
    Canvas->AddSlot().Position(FVector2D(80,44)).Size(FVector2D(1440,116))[Header];InputPanels.Add(Header);
    auto Setting=SNew(STacticalChrome).Padding(24).Opacity(.50f).Accent(FLinearColor(.54f,.31f,.13f)).Visibility_Lambda(Visible)
        [SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("この一戦で、何を守るか"); },14,Gold)]
         + SVerticalBox::Slot().AutoHeight().Padding(0,16)[Label([this] { return EpisodeSnapshot.Question; },22)]
         + SVerticalBox::Slot().AutoHeight().Padding(0,12)[Label([this] { return EpisodeSnapshot.Setting; },16,FLinearColor(.60f,.65f,.66f))]];
    Canvas->AddSlot().Position(FVector2D(80,174)).Size(FVector2D(334,354))[Setting];InputPanels.Add(Setting);
    for(int I=0;I<3;++I)
    {
        const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
        const TCHAR* Roles[]={TEXT("志を語る者"),TEXT("采配を問う者"),TEXT("仲間へ駆ける者")};
        auto Figure=SNew(SBorder).Padding(2).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor_Lambda([this,I] { return CurrentEpisodeLine().Speaker==I ? FLinearColor(.042f,.047f,.047f):FLinearColor(.016f,.022f,.028f); })
            .ColorAndOpacity_Lambda([this,I] { const int S=CurrentEpisodeLine().Speaker;return FLinearColor(1,1,1,S==I ? 1.f:S<0 || S>=3 ? .77f:.43f); })
            [SNew(SVerticalBox)
             + SVerticalBox::Slot().FillHeight(1)[CutinPortraitWidget([I] { return I; },310,342)]
             + SVerticalBox::Slot().AutoHeight().Padding(14,6)[Label([Name=FString(Names[I]),Role=FString(Roles[I])] { return Name+TEXT("  /  ")+Role; },16,Gold)]];
        auto Box=SNew(SBox).Visibility_Lambda(Visible)[Figure];
        Canvas->AddSlot().Position(FVector2D(458+I*356,166)).Size(FVector2D(334,380))[Box];InputPanels.Add(Box);
    }
    auto Dialogue=SNew(STacticalChrome).Padding(26).Opacity(.98f).Accent(FLinearColor(.68f,.43f,.19f)).Visibility_Lambda(Visible)
        [SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1)[Label([this] { return CurrentEpisodeLine().Name; },28,Gold)]
            + SHorizontalBox::Slot().AutoWidth()[Label([this] { return FString::Printf(TEXT("%02d / %02d"),EpisodePage+1,EpisodeSnapshot.Lines.Num()); },15)]]
         + SVerticalBox::Slot().FillHeight(1).Padding(0,10)
           [SNew(SScrollBox)+SScrollBox::Slot()[Label([this] { return CurrentEpisodeLine().Text; },27)]]
         + SVerticalBox::Slot().AutoHeight()[Label([this] { return CurrentEpisodeLine().Direction; },16,FLinearColor(.57f,.63f,.64f))]];
    Canvas->AddSlot().Position(FVector2D(80,574)).Size(FVector2D(1440,224))[Dialogue];InputPanels.Add(Dialogue);
    auto Controls=SNew(SBox).Visibility_Lambda(Visible)[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(170)[Button([] { return TEXT("前の台詞 / ←"); },[this] { StepEpisode(-1); },[this] { return EpisodePage>0; })]]
        + SHorizontalBox::Slot().FillWidth(1).Padding(22,0).VAlign(VAlign_Center)[Label([] { return TEXT("自分の速さで読む  /  ←で読み返す  /  場面を閉じた後もHで再読できる"); },13,FLinearColor(.56f,.60f,.60f))]
        + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(320)[Button([this] { return EpisodePage+1<EpisodeSnapshot.Lines.Num() ? TEXT("次の台詞 / Enter・Space・→"):TEXT("場面を終えて采配へ / Enter"); },[this] { StepEpisode(1); })]]];
    Canvas->AddSlot().Position(FVector2D(80,824)).Size(FVector2D(1440,42))[Controls];InputPanels.Add(Controls);
}
