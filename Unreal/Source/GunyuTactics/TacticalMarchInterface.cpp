#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
using namespace gunyuui;

void ATacticalController::ToggleMarchPlan()
{
    if (!Field || !Field->bRealtimeCommand || !Field->Presentation().bBattle || Field->IsPlaying()) return;
    bMarchPlanOpen=!bMarchPlanOpen;
    if (bMarchPlanOpen)
    {
        CloseChainReview();
        bInterfaceVisible=true;
        bBattleLogOpen=false;bTacticPickerOpen=false;
        if (!Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
    }
    IgnoreMapInputThroughFrame=Frames+1;
}
void ATacticalController::FocusMarchPoint(int Index)
{
    const auto& Points=Field->Presentation().MarchPoints;
    if (!Points.IsValidIndex(Index)) return;
    Focus=Field->SurfaceLocation(Points[Index].Tile);bMarchPlanOpen=false;IgnoreMapInputThroughFrame=Frames+1;
}
void ATacticalController::BuildMarchInterface(TSharedRef<SCanvas> Canvas)
{
    for (int Index=0;Index<4;++Index)
        Canvas->AddSlot().Position_Lambda([this,Index]
        {
            const auto& H=Field->Presentation();
            return Field->bRealtimeCommand && H.bBattle && !bBattleLogOpen && !bMarchPlanOpen && H.MarchPoints.IsValidIndex(Index) ?
                MapLabelPosition(Field->SurfaceLocation(H.MarchPoints[Index].Tile)+FVector(0,0,40),FVector2D(50,26)) : FVector2D(-2000,-2000);
        }).Size(FVector2D(50,26))
        [SNew(SBorder).Visibility(EVisibility::HitTestInvisible).Padding(FMargin(5,2)).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
         .BorderBackgroundColor(FLinearColor(.13f,.10f,.025f,.97f))
         [Label([this,Index] { const auto& P=Field->Presentation().MarchPoints;return P.IsValidIndex(Index) ? FString::Printf(TEXT("%d%s"),Index+1,Index+1==P.Num() ? TEXT(" 終") : TEXT("")) : FString(); },12,Gold)]];
    auto Content=SNew(SVerticalBox);
    Content->AddSlot().AutoHeight()[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1)[Label([] { return TEXT("行軍計画 / 経由地と到着後の指示"); },20,Gold)]
        + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("閉じる / V"); },[this] { ToggleMarchPlan(); },[] { return true; },true)]];
    Content->AddSlot().AutoHeight().Padding(0,8)[Label([this] { return Field->Presentation().MarchSummary; },13)];
    for (int Index=0;Index<4;++Index)
        Content->AddSlot().AutoHeight().Padding(0,4)[SNew(SBorder).Padding(6).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(.035f,.065f,.065f))
            .Visibility_Lambda([this,Index] { return Field->Presentation().MarchPoints.IsValidIndex(Index) ? EVisibility::Visible : EVisibility::Collapsed; })
            [SNew(SHorizontalBox)
             + SHorizontalBox::Slot().FillWidth(1)[Label([this,Index] { const auto& P=Field->Presentation().MarchPoints;return P.IsValidIndex(Index) ? P[Index].Label : FString(); },14,Gold)]
             + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("地点へ"); },[this,Index] { FocusMarchPoint(Index); },[] { return true; },true)]]];
    Content->AddSlot().FillHeight(1);
    Content->AddSlot().AutoHeight().Padding(0,8)[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,6,0)[Button([this] { return Field->Presentation().ArrivalLabel+TEXT(" / 切替"); },[this] { Field->Issue(ETacticalCommand::RealtimeArrival); },[this] { return Field->CanIssue(ETacticalCommand::RealtimeArrival); },true)]
        + SHorizontalBox::Slot().FillWidth(1)[Button([] { return TEXT("最後の地点を取り消す"); },[this] { Field->Issue(ETacticalCommand::RealtimeUndoWaypoint); },[this] { return Field->CanIssue(ETacticalCommand::RealtimeUndoWaypoint); },true)]];
    Content->AddSlot().AutoHeight()[Label([] { return TEXT("閉じてShift＋右クリックで追加、右クリックで引き直し。最大４地点。\n固守・伏兵準備は到着の次の更新から。伏兵は担当武将と森林が必要。閉じた後、Spaceで再開。"); },12)];
    auto Visible=[this] { return bMarchPlanOpen && Field->bRealtimeCommand && Field->Presentation().bBattle ? EVisibility::Visible : EVisibility::Collapsed; };
    auto Shade=SNew(SBorder).Visibility_Lambda(Visible).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0,0,0,.4f));
    InputPanels.Add(Shade);Canvas->AddSlot().Position(FVector2D::ZeroVector).Size(FVector2D(1600,900))[Shade];
    auto Panel=SNew(STacticalChrome).Visibility_Lambda(Visible).Padding(16).Accent(FLinearColor(.56f,.42f,.18f))[Content];
    InputPanels.Add(Panel);Canvas->AddSlot().Position(FVector2D(254,268)).Size(FVector2D(930,382))[Panel];
}
