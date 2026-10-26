#include "TacticalTrial.h"
#include "TacticalChainReview.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
using namespace gunyuui;
void ATacticalController::BuildChainReviewInterface(TSharedRef<SCanvas> Canvas)
{
    auto Content=SNew(SVerticalBox);
    Content->AddSlot().AutoHeight()[SNew(SHorizontalBox)
      + SHorizontalBox::Slot().FillWidth(1)[Label([] { return TEXT("戦術記録  ―  直近の連鎖を振り返る"); },25,Gold)]
      + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("閉じる / Y"); },[this] { CloseChainReview(); },[] { return true; },true)]];
    Content->AddSlot().AutoHeight().Padding(0,12,0,4)[Label([this] { return ChainReviewSnapshot.IsValid() ? ChainReviewSnapshot->Summary:FString(); },23)];
    Content->AddSlot().AutoHeight().Padding(0,0,0,12)[Label([] { return TEXT("実行した手の役割・成果・つながり。閲覧中は停止し、閉じた後も停止を保つ。"); },13)];
    auto Rows=SNew(SVerticalBox);
    for(int I=0;I<3;++I) Rows->AddSlot().AutoHeight().Padding(0,0,0,8)[ChainReviewRow(I)];
    Content->AddSlot().FillHeight(1)[SNew(SScrollBox)+SScrollBox::Slot()[Rows]];
    Content->AddSlot().AutoHeight().Padding(0,12,0,4)[Label([this] { return ChainReviewSnapshot.IsValid() ? ChainReviewSnapshot->Lesson:FString(); },16,Gold)];
    auto Visibility=[this] { return bChainReviewOpen && ChainReviewSnapshot.IsValid() ? EVisibility::Visible:EVisibility::Collapsed; };
    auto Shade=SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0,0,0,.72f)).Visibility_Lambda(Visibility);
    auto Panel=SNew(STacticalChrome).Padding(22).Accent(Gold).Visibility_Lambda(Visibility)[Content];
    InputPanels.Add(Shade);InputPanels.Add(Panel);
    Canvas->AddSlot().Position(FVector2D::ZeroVector).Size(FVector2D(1600,900))[Shade];
    Canvas->AddSlot().Position(FVector2D(140,150)).Size(FVector2D(1320,620))[Panel];
}
