#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Notifications/SProgressBar.h"
using namespace gunyuui;
void ATacticalController::BuildCommandInterface(TSharedRef<SCanvas> Canvas)
{
    auto Command=[this](ETacticalCommand Action,TFunction<FString()> Name)
    { return Button(Name,[this,Action] { Field->Issue(Action); },[this,Action] { return Field->CanIssue(Action); },true); };
    auto Orders = SNew(SVerticalBox);
    Orders->AddSlot().AutoHeight()[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([this] { return Field->Presentation().Selected; },32,38)]
        + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center).Padding(10,0)
          [Label([this] { const auto& H=Field->Presentation();return H.Officers.IsValidIndex(H.Selected) ? H.Officers[H.Selected].Name+TEXT(" / 采配") : FString(TEXT("采配")); },17,Gold)]];
    Orders->AddSlot().AutoHeight().Padding(0, 5)[Label([this] { return Field->Presentation().Target; }, 13)];
    Orders->AddSlot().AutoHeight()
        [ SNew(SHorizontalBox)
          + SHorizontalBox::Slot().FillWidth(1).Padding(0, 0, 4, 4)[Command(ETacticalCommand::Move, [] { return TEXT("移動"); })]
          + SHorizontalBox::Slot().FillWidth(1).Padding(0, 0, 0, 4)[Command(ETacticalCommand::Strike, [] { return TEXT("攻撃"); })] ];
    Orders->AddSlot().AutoHeight()
        [ SNew(SHorizontalBox)
          + SHorizontalBox::Slot().FillWidth(1).Padding(0, 0, 4, 4)[Command(ETacticalCommand::PrepareAmbush, [] { return TEXT("伏兵"); })]
          + SHorizontalBox::Slot().FillWidth(1).Padding(0, 0, 0, 4)
            [SNew(SBox).ToolTipText_Lambda([this] { return FText::FromString(Field->Presentation().GuardHint); })
             [Button([this] { const auto& H=Field->Presentation();return H.GuardButtonLabel.IsEmpty() ? FString(TEXT("固守")):H.GuardButtonLabel; },
               [this] { Field->Issue(Field->Presentation().bGuardOrder ? ETacticalCommand::ReleaseGuard:ETacticalCommand::Hold); },
               [this] { return Field->CanIssue(Field->Presentation().bGuardOrder ? ETacticalCommand::ReleaseGuard:ETacticalCommand::Hold); },true)]] ];
    Orders->AddSlot().AutoHeight().Padding(0,2,0,4)
      [SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().GuardContext.IsEmpty() ? EVisibility::Collapsed:EVisibility::Visible; })
       .ToolTipText_Lambda([this] { return FText::FromString(Field->Presentation().GuardHint); })
       [Label([this] { return Field->Presentation().GuardContext; },11,Gold)]];
    Orders->AddSlot().AutoHeight()
        [SNew(SHorizontalBox)
         + SHorizontalBox::Slot().FillWidth(3).Padding(0, 0, 4, 0)[SNew(SBox).ToolTipText_Lambda([this] { return FText::FromString(Field->Presentation().Hint); })[TacticPickerButton()]]
         + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::QueueTactic, [] { return TEXT("予約"); })]];
    Orders->AddSlot().AutoHeight().Padding(0, 8, 0, 4)
        [Label([this] { const auto& H = Field->Presentation(); return H.bBusy ? TEXT("予約時の予測 / 行動を再生中") : H.Orders.IsEmpty() ? FString(TEXT("連鎖の段取り")) : FString::Printf(TEXT("連鎖  /  成立見込み %d / %d"), H.Predicted, H.Orders.Num()); }, 16, Gold)];
    Orders->AddSlot().AutoHeight().Padding(0,0,0,5)
        [SNew(SBox).HeightOverride(36).ToolTipText_Lambda([this] { return FText::FromString(Field->Presentation().TacticAvailability+TEXT("\n単独100％ → 2コンボ125％ → 3コンボ150％")); })
         [SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular",11)).AutoWrapText(true)
          .ColorAndOpacity_Lambda([this] { return FSlateColor(Field->Presentation().bTacticUnavailable ? FLinearColor(1,.65f,.32f):FLinearColor(.55f,.83f,.78f)); })
          .Text_Lambda([this] { return FText::FromString(Field->IsPlaying() ? TEXT("行動を再生中。演出完了後に命令できます。"):Field->Presentation().TacticAvailability); })]];
    Orders->AddSlot().FillHeight(1)[ChainPlannerWidget()];
    Orders->AddSlot().AutoHeight().Padding(0, 8, 0, 4)[Command(ETacticalCommand::Execute, [this] { return Field->Presentation().Orders.Num()==1 ? FString(TEXT("単独戦法を発動 / C")) : FString(TEXT("連鎖を発動 / C")); })];
    Orders->AddSlot().AutoHeight()
        [ SNew(SHorizontalBox)
          + SHorizontalBox::Slot().FillWidth(1).Padding(0, 0, 4, 0)[Command(ETacticalCommand::Sample, [] { return TEXT("連鎖の見本"); })]
          + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::Clear, [] { return TEXT("全取消"); })] ];
    Orders->AddSlot().AutoHeight().Padding(0,6,0,0)
      [SNew(SVerticalBox).Visibility_Lambda([this] { return Field->Presentation().bHasResupplyPlan ? EVisibility::Visible:EVisibility::Collapsed; })
       + SVerticalBox::Slot().AutoHeight()[Command(ETacticalCommand::Resupply,[this] { return Field->Presentation().ResupplyLabel; })]
       + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("主力へ糧1,800を分配・士気＋20。行動権は戻らない"); },11,Gold)]];
    auto Simple=SNew(SVerticalBox);
    Simple->AddSlot().AutoHeight()[SNew(SHorizontalBox)
      + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([this] { return Field->Presentation().Selected; },62,74)]
      + SHorizontalBox::Slot().FillWidth(1).Padding(14,8)[SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[Label([this] { const auto& H=Field->Presentation();return H.Officers.IsValidIndex(H.Selected) ? H.Officers[H.Selected].Name:FString(); },25,Gold)]
        + SVerticalBox::Slot().AutoHeight().Padding(0,8)[Label([this] { return Field->IsAutoBattle() ? Field->Presentation().ClockLabel:FString(TEXT("さあ、次の一手だ！")); },15)]]];
    Simple->AddSlot().AutoHeight().Padding(0,6,0,2)[BuildBattleChoicesInterface()];
    Simple->AddSlot().AutoHeight().Padding(0,2,0,6)[SNew(SBox).HeightOverride(38)
      .ToolTipText_Lambda([this] { return FText::FromString(Field->CanIssue(ETacticalCommand::AutoBattle) ? TEXT("味方3隊の移動・戦法・目標の守備を任せる。損耗と戦果は本当の結果。いつでも指揮へ戻れる。"):TEXT("予約した戦法や合図を実行・解除してから任せよう。") ); })
      [Button([this] { return Field->IsAutoBattle() ? TEXT("自分で指揮する"):TEXT("武将たちに任せる"); },[this] { Field->Issue(ETacticalCommand::AutoBattle); },[this] { return Field->CanIssue(ETacticalCommand::AutoBattle); })]];
    Simple->AddSlot().AutoHeight().Padding(0,0,0,10)[SNew(SBox).HeightOverride(4).Visibility_Lambda([this] { return Field->IsAutoBattle() ? EVisibility::Visible:EVisibility::Collapsed; })
      [SNew(SProgressBar).Style(&ProgressStyle()).Percent_Lambda([this] { return Field->Presentation().ClockProgress; }).FillColorAndOpacity(Gold)]];
    Simple->AddSlot().AutoHeight().Padding(0,0,0,4)[SNew(SHorizontalBox)
      + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::RealtimePause,[this] { return Field->RealtimePaused() ? TEXT("再開する"):TEXT("一時停止"); })]
      + SHorizontalBox::Slot().FillWidth(1).Padding(8,0,0,0)[Command(ETacticalCommand::RealtimeSpeed,[this] { return TEXT("速さ ")+Field->Presentation().SpeedLabel; })]];
    Simple->AddSlot().AutoHeight().Padding(0,4)[SNew(SBox).HeightOverride(36)[SNew(SHorizontalBox)
      + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,4,0)[Command(ETacticalCommand::Move,[] { return TEXT("ここへ進め！"); })]
      + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::Hold,[] { return TEXT("ここを守れ！"); })]]];
    auto Content=SNew(SVerticalBox);
    Content->AddSlot().AutoHeight().Padding(0,0,0,8)[Button([this] { return bDetailedBattle ? TEXT("かんたんな采配へ戻る"):TEXT("詳しい采配を開く"); },[this] { bDetailedBattle=!bDetailedBattle;if(bDetailedBattle) Field->StopAutoBattle(); })];
    Content->AddSlot().FillHeight(1)[SNew(SBox).Visibility_Lambda([this] { return bDetailedBattle ? EVisibility::Collapsed:EVisibility::Visible; })[Simple]];
    Content->AddSlot().FillHeight(1)[SNew(SBox).Visibility_Lambda([this] { return bDetailedBattle ? EVisibility::Visible:EVisibility::Collapsed; })[Orders]];
    auto Panel=SNew(STacticalChrome).Padding(12)
        .Visibility_Lambda([this] { return Field->Presentation().Journey.bVisible || (bTacticCinema && Field->ReadTacticCinema().bVisible) ? EVisibility::Collapsed:EVisibility::Visible; })[Content];
    InputPanels.Add(Panel);Canvas->AddSlot().Position(gunyulayout::CommandOrigin).Size(gunyulayout::CommandSize)[Panel];
}
