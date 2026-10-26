#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
using namespace gunyuui;

void ATacticalController::ToggleBattleLog()
{
    if (!Field || !Field->bRealtimeCommand || !Field->Presentation().bBattle) return;
    bBattleLogOpen=!bBattleLogOpen;
    if (bBattleLogOpen) { bMarchPlanOpen=false;bInterfaceVisible=true; }
    if (bBattleLogOpen && !Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
    IgnoreMapInputThroughFrame=Frames+1;
}
void ATacticalController::FocusBattleNotice(int Index)
{
    const auto& Notices=Field->Presentation().BattleNotices;
    if (!Notices.IsValidIndex(Index)) return;
    const int Id=Notices[Index].Army;
    const auto& Units=Field->VisualUnits();
    if (!Units.IsValidIndex(Id) || Units[Id].Id<0 || Units[Id].Troops<=0) return;
    Focus=Units[Id].Position; // Current displayed observation, never the old report's hidden target.
    if (Id<3) Field->SelectOfficer(Id);
    bBattleLogOpen=false;IgnoreMapInputThroughFrame=Frames+1;
}
void ATacticalController::BuildBattleWatchInterface(TSharedRef<SCanvas> Canvas)
{
    auto Content=SNew(SVerticalBox);
    Content->AddSlot().AutoHeight()[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1)[Label([] { return TEXT("伝令帳 / 最新８件"); },20,Gold)]
        + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("閉じる / L"); },[this] { ToggleBattleLog(); },[] { return true; },true)]];
    Content->AddSlot().AutoHeight().Padding(0,6)[Label([] { return TEXT("閲覧中は停止。報告を選ぶと、今見えている該当部隊へ移動する。閉じた後、Spaceで再開。"); },12)];
    auto Entries=SNew(SVerticalBox);
    Entries->AddSlot().AutoHeight()[Label([this] { return Field->Presentation().BattleNotices.IsEmpty() ? TEXT("まだ報告はありません。行軍・交戦の結果から重要な変化を記録します。"):TEXT(""); },13)];
    for (int Index=0;Index<8;++Index)
    {
        auto Entry=SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1)[Label([this,Index] { const auto& Items=Field->Presentation().BattleNotices;return Items.IsValidIndex(Index) ? Items[Index].Title : FString(); },14,Gold)]
                + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("部隊へ"); },[this,Index] { FocusBattleNotice(Index); },[this,Index]
                {
                    const auto& Items=Field->Presentation().BattleNotices;const auto& Units=Field->VisualUnits();
                    if (!Items.IsValidIndex(Index)) return false;
                    const int Id=Items[Index].Army;return Units.IsValidIndex(Id) && Units[Id].Id>=0 && Units[Id].Troops>0;
                },true)]]
            + SVerticalBox::Slot().AutoHeight().Padding(0,3)[Label([this,Index] { const auto& Items=Field->Presentation().BattleNotices;return Items.IsValidIndex(Index) ? Items[Index].Detail : FString(); },12)];
        Entries->AddSlot().AutoHeight().Padding(0,3)[SNew(SBorder).Padding(8)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.035f,.065f,.065f))
            .Visibility_Lambda([this,Index] { return Field->Presentation().BattleNotices.IsValidIndex(Index) ? EVisibility::Visible : EVisibility::Collapsed; })[Entry]];
    }
    Content->AddSlot().FillHeight(1)[SNew(SScrollBox)+SScrollBox::Slot()[Entries]];
    auto Panel=SNew(STacticalChrome).Padding(16).Accent(FLinearColor(.56f,.42f,.18f))
        .Visibility_Lambda([this] { return bBattleLogOpen && Field->bRealtimeCommand && Field->Presentation().bBattle ? EVisibility::Visible : EVisibility::Collapsed; })[Content];
    auto Shade=SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0,0,0,.4f))
        .Visibility_Lambda([this] { return bBattleLogOpen && Field->bRealtimeCommand && Field->Presentation().bBattle ? EVisibility::Visible : EVisibility::Collapsed; });
    InputPanels.Add(Shade);Canvas->AddSlot().Position(FVector2D::ZeroVector).Size(FVector2D(1600,900))[Shade];
    InputPanels.Add(Panel);Canvas->AddSlot().Position(FVector2D(254,268)).Size(FVector2D(930,382))[Panel];
}
