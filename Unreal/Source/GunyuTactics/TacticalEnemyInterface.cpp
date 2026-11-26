#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "TacticalCommandOverlayLayout.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"

using namespace gunyuui;

void ATacticalController::BuildEnemyInterface(TSharedRef<SCanvas> Canvas)
{
    auto Content = SNew(SVerticalBox);
    auto Reads = SNew(SHorizontalBox);
    for (int Index = 0; Index < 2; ++Index)
    {
        Reads->AddSlot().AutoWidth().Padding(0, 0, 8, 0)
        [SNew(SBox).WidthOverride(202)
         .Visibility_Lambda([this, Index] { return Field->Presentation().EnemyReads.IsValidIndex(Index) ? EVisibility::Visible : EVisibility::Collapsed; })
         [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(7)
          .ButtonColorAndOpacity_Lambda([this, Index]
          {
              const auto& R = Field->Presentation().EnemyReads;
              return FSlateColor(R.IsValidIndex(Index) && R[Index].bSelected ? FLinearColor(.28f,.11f,.065f) : FLinearColor(.10f,.065f,.05f));
          })
          .OnClicked_Lambda([this, Index]
          {
              const auto& R = Field->Presentation().EnemyReads;
              if (R.IsValidIndex(Index)) { Field->SelectAt(R[Index].Tile);bEnemyDetailsOpen=true; }
              return FReply::Handled();
          })
          [SNew(SVerticalBox)
           + SVerticalBox::Slot().AutoHeight()[Label([this, Index] { const auto& R = Field->Presentation().EnemyReads; return R.IsValidIndex(Index) ? R[Index].Name : FString(); }, 13, Gold)]
           + SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 0)[Label([this, Index] { const auto& R = Field->Presentation().EnemyReads; return R.IsValidIndex(Index) ? R[Index].Intent : FString(); }, 12, FLinearColor(1.f,.62f,.42f))]]]];
    }
    Reads->AddSlot().AutoWidth().VAlign(VAlign_Center)[Button([this] { return bEnemyDetailsOpen ? TEXT("閉じる I") : TEXT("敵情詳細 I"); },[this] { bEnemyDetailsOpen=!bEnemyDetailsOpen; },[] { return true; },true)];
    Content->AddSlot().AutoHeight()[Reads];
    auto Strip=SNew(STacticalChrome).Padding(FMargin(8,4)).Accent(FLinearColor(.52f,.24f,.12f))
        .Visibility_Lambda([this] { return bDetailedBattle && Field->Presentation().bBattle && !Field->IsPlaying() && !bTacticPickerOpen ? EVisibility::Visible : EVisibility::Collapsed; })[Content];
    InputPanels.Add(Strip);Canvas->AddSlot().Position(FVector2D(16,116)).Size(FVector2D(700,50))[Strip];
    // Keep the currently selected, observed enemy readable in normal command.
    // The complete list and attack overlay remain an optional detail view.
    auto Read=SNew(STacticalChrome).Padding(FMargin(12,8)).Accent(FLinearColor(.52f,.24f,.12f))
      .Visibility_Lambda([this] { return !bDetailedBattle && Field->Presentation().bBattle && !Field->IsPlaying() && !bTacticPickerOpen ? EVisibility::Visible:EVisibility::Collapsed; })
      [SNew(SVerticalBox)
       + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
         + SHorizontalBox::Slot().FillWidth(1)[Label([this]
           {
               const auto& Reads=Field->Presentation().EnemyReads;
               const auto* Enemy=Reads.FindByPredicate([](const auto& R) { return R.bSelected; });
               return Enemy ? TEXT("読み（推定）：")+Enemy->Name+TEXT(" ／ ")+Enemy->Intent:FString(TEXT("読み（推定）：見えている敵の札を選ぼう"));
           },14,Gold)]
         + SHorizontalBox::Slot().AutoWidth().Padding(10,0,0,0)
           [Button([this] { return bEnemyDetailsOpen ? TEXT("閉じる / I"):TEXT("詳しく / I"); },[this] { bEnemyDetailsOpen=!bEnemyDetailsOpen; },[] { return true; },true)]]
       + SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)[Label([this]
         {
             const auto& Reads=Field->Presentation().EnemyReads;
             const auto* Enemy=Reads.FindByPredicate([](const auto& R) { return R.bSelected; });
             if(!Enemy) return FString(TEXT("視界外の状況は不明。点線矢印は現在の情報から読む移動の見込み。"));
             FString First,Rest;
             return Enemy->Counter.Split(TEXT("\n"),&First,&Rest) ? First:Enemy->Counter;
         },12)]];
    InputPanels.Add(Read);Canvas->AddSlot().Position(FVector2D(16,gunyucommandlayout::EnemyTop)).Size(FVector2D(gunyucommandlayout::EnemyWidth,gunyucommandlayout::EnemyHeight))[Read];
    auto Detail=SNew(STacticalChrome).Padding(14).Accent(FLinearColor(.52f,.24f,.12f))
        .Visibility_Lambda([this] { return bEnemyDetailsOpen && Field->Presentation().bBattle && !Field->IsPlaying() && !bTacticPickerOpen ? EVisibility::Visible : EVisibility::Collapsed; })
        [SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("軍師の読み / 見える敵からの推定"); },17,Gold)]
         + SVerticalBox::Slot().AutoHeight().Padding(0,8)[Label([this] { return Field->Presentation().EnemyCounter; },14)]
         + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Label([] { return TEXT("朱＝推定攻撃圏。点線矢印＝移動の見込み。視界外は不明。"); },11)]
            + SHorizontalBox::Slot().AutoWidth()[Button([this] { return Field->Presentation().bThreatOverlay ? TEXT("攻撃圏 ON") : TEXT("攻撃圏 OFF"); },[this] { Field->ToggleThreatOverlay(); },[] { return true; },true)]]];
    InputPanels.Add(Detail);Canvas->AddSlot().Position_Lambda([this] { return FVector2D(16,gunyucommandlayout::EnemyDetailTop(bDetailedBattle)); }).Size(FVector2D(700,132))[Detail];
}
