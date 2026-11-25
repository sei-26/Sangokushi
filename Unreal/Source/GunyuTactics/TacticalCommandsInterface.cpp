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
    Orders->AddSlot().AutoHeight().Padding(0,0,0,6)[BuildGrandOrderInterface()];
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
    Orders->AddSlot().AutoHeight()[SNew(SBox).HeightOverride(210)[ChainPlannerWidget()]];
    Orders->AddSlot().AutoHeight().Padding(0, 8, 0, 4)[Button([this] { return Field->Presentation().Orders.Num()==1 ? FString(TEXT("単独戦法を今発動 / C")) : FString(TEXT("連鎖を今発動 / C")); },
        [this] { Field->FireManualTactics(); },[this] { return Field->CanFireManualTactics(); },true)];
    Orders->AddSlot().AutoHeight()
        [ SNew(SHorizontalBox)
          + SHorizontalBox::Slot().FillWidth(1).Padding(0, 0, 4, 0)[Command(ETacticalCommand::Sample, [] { return TEXT("連鎖の見本"); })]
          + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::Clear, [] { return TEXT("全取消"); })] ];
    Orders->AddSlot().AutoHeight().Padding(0,6,0,0)
      [SNew(SVerticalBox).Visibility_Lambda([this] { return Field->Presentation().bHasResupplyPlan ? EVisibility::Visible:EVisibility::Collapsed; })
       + SVerticalBox::Slot().AutoHeight()[Command(ETacticalCommand::Resupply,[this] { return Field->Presentation().ResupplyLabel; })]
       + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("主力へ糧1,800を分配・士気＋20。行動権は戻らない"); },11,Gold)]];
    auto Simple=SNew(SVerticalBox);
    Simple->AddSlot().AutoHeight().Padding(0,0,0,6)[BuildGrandOrderInterface()];
    auto PressedMarch=MakeShared<FTacticalPresentation::FMissionMarch>();
    Simple->AddSlot().AutoHeight().Padding(0,0,0,7)
      [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(FMargin(7,6))
       .Visibility_Lambda([this] { return Field->Presentation().MissionMarch.bVisible ? EVisibility::Visible:EVisibility::Collapsed; })
       .IsEnabled_Lambda([this] { return Field->Presentation().MissionMarch.bCanIssue; })
       .ToolTipText_Lambda([this] { return FText::FromString(Field->Presentation().MissionMarch.Detail); })
       .OnPressed_Lambda([this,PressedMarch] { *PressedMarch=Field->Presentation().MissionMarch; })
       .OnClicked_Lambda([this,PressedMarch] { Field->IssueMissionMarch(*PressedMarch);return FReply::Handled(); })
       [Label([this] { return Field->Presentation().MissionMarch.Label; },13,Gold)]];
    Simple->AddSlot().AutoHeight().Padding(0,0,0,8)
      [SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().bDecisionPaused ? EVisibility::Visible:EVisibility::Collapsed; })
       [Command(ETacticalCommand::AutoBattle,[] { return TEXT("ここから自分で指揮"); })]];
    Simple->AddSlot().AutoHeight().Padding(0,0,0,6)[OpeningOrderCard()];
    Simple->AddSlot().AutoHeight().Padding(0,0,0,5)
        [SNew(SBox).HeightOverride(28)
          .Visibility_Lambda([this] { return SelectedDutyTile()>=0 && !Field->IsAutoBattle() ? EVisibility::Visible:EVisibility::Collapsed; })
          .ToolTipText_Lambda([this] { return FText::FromString(Field->Presentation().DutyProgress.Detail); })
          [Button([] { return TEXT("担当地点へ / P"); },[this] { FocusSelectedDuty(); },[] { return true; },true)]];
    Simple->AddSlot().AutoHeight().Padding(0,0,0,4)
      [SNew(SBox).Visibility_Lambda([this] { return Field->IsAutoBattle() ? EVisibility::Visible:EVisibility::Collapsed; })
       [Label([this] { return Field->Presentation().ClockLabel; },13,Gold)]];
    Simple->AddSlot().AutoHeight().Padding(0,2,0,4)[SNew(SBox).HeightOverride(30)
      .Visibility_Lambda([this]
      {
          const auto& Duty=Field->Presentation().DutyProgress;
          return SelectedDutyTile()>=0 && !Field->IsAutoBattle() && (Duty.InspectTile>=0 || Duty.bCanResume) ? EVisibility::Visible:EVisibility::Collapsed;
      })
      .ToolTipText_Lambda([this] { return FText::FromString(Field->Presentation().DutyProgress.Detail); })
      [Button([this]
      {
          const auto& Duty=Field->Presentation().DutyProgress;
          return Duty.InspectTile>=0 ? Duty.InspectLabel:FString(TEXT("担当へ行軍を指示"));
      },[this]
      {
          if(Field->Presentation().DutyProgress.InspectTile>=0) FocusDutyObstacle();
          else Field->Issue(ETacticalCommand::ResumeDuty);
      })]];
    Simple->AddSlot().AutoHeight().Padding(0,6,0,2)[BuildBattleChoicesInterface()];
    Simple->AddSlot().AutoHeight().Padding(0,2,0,4)[BuildSupportChoicesInterface()];
    Simple->AddSlot().AutoHeight().Padding(0,2,0,6)[SNew(SBox).HeightOverride(38)
      .Visibility_Lambda([this] { return Field->Presentation().bDecisionPaused ? EVisibility::Collapsed:EVisibility::Visible; })
      .ToolTipText_Lambda([this] { return FText::FromString(Field->CanIssue(ETacticalCommand::AutoBattle) ? TEXT("味方3隊の移動と戦法を任せる。危険時停止がONなら、重要な戦況の変化で止まる。いつでも指揮を引き取れる。"):TEXT("予約した戦法や合図を実行・解除してから任せよう。") ); })
      [Button([this] { return Field->IsAutoBattle() ? TEXT("自分で指揮する"):Field->Presentation().bConvoyMission ? TEXT("護送を任せる"):TEXT("武将たちに任せる"); },[this] { Field->Issue(ETacticalCommand::AutoBattle); },[this] { return Field->CanIssue(ETacticalCommand::AutoBattle); })]];
    Simple->AddSlot().AutoHeight().Padding(0,0,0,10)[SNew(SBox).HeightOverride(4).Visibility_Lambda([this] { return Field->IsAutoBattle() ? EVisibility::Visible:EVisibility::Collapsed; })
      [SNew(SProgressBar).Style(&ProgressStyle()).Percent_Lambda([this] { return Field->Presentation().ClockProgress; }).FillColorAndOpacity(Gold)]];
    Simple->AddSlot().AutoHeight().Padding(0,0,0,4)[SNew(SHorizontalBox)
      + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::RealtimePause,[this] { return Field->RealtimePaused() ? TEXT("再開する"):TEXT("一時停止"); })]
      + SHorizontalBox::Slot().FillWidth(1).Padding(8,0,0,0)[Command(ETacticalCommand::RealtimeSpeed,[this] { return TEXT("速さ ")+Field->Presentation().SpeedLabel; })]];
    Simple->AddSlot().AutoHeight().Padding(0,4)[SNew(SBox).HeightOverride(36)[SNew(SHorizontalBox)
      + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,4,0)[Command(ETacticalCommand::Move,[] { return TEXT("ここへ進め！"); })]
      + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::Hold,[] { return TEXT("ここを守れ！"); })]]];
    auto Content=SNew(SVerticalBox);
    Content->AddSlot().AutoHeight().Padding(0,0,0,6)
      [SNew(SBox).Visibility_Lambda([this] { return !ShowOpeningOrder() && !Field->Presentation().MissionAdvice.IsEmpty() ? EVisibility::Visible:EVisibility::Collapsed; })
       .ToolTipText_Lambda([this] { const auto& H=Field->Presentation();return FText::FromString(H.MissionDetail+TEXT("\n")+H.GrandOrderStatus); })
       [SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("今の役目"); },11,Gold)]
         + SVerticalBox::Slot().AutoHeight()[Label([this] { return Field->Presentation().MissionAdvice; },14)]]];
    Content->AddSlot().AutoHeight().Padding(0,0,0,6)[ManualTacticsWidget()];
    Content->AddSlot().AutoHeight().Padding(0,0,0,8)[SNew(SHorizontalBox)
      + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,4,0)
        [SNew(SBox).ToolTipText(FText::FromString(TEXT("隣接1歩の移動後に、選んだ戦法が届くか比べる。仮選択だけでは動かない。自動指揮中は采配を引き取る。")))
         [Button([] { return TEXT("移動後の戦法"); },[this] { OpenTacticPositionComparison(); },[this] { return Field->CanCompareTacticPositions(); })]]
      + SHorizontalBox::Slot().FillWidth(1)
        [Button([this] { return bDetailedBattle ? TEXT("通常の采配"):TEXT("詳しい采配"); },[this] { bDetailedBattle=!bDetailedBattle;if(bDetailedBattle) Field->StopAutoBattle(); })]];
    Content->AddSlot().FillHeight(1)
        [SNew(SScrollBox).Visibility_Lambda([this] { return bDetailedBattle ? EVisibility::Collapsed:EVisibility::Visible; })
         + SScrollBox::Slot()[Simple]];
    Content->AddSlot().FillHeight(1)[SNew(SScrollBox).Visibility_Lambda([this] { return bDetailedBattle ? EVisibility::Visible:EVisibility::Collapsed; })+SScrollBox::Slot()[Orders]];
    auto Panel=SNew(STacticalChrome).Padding(12)
        .Visibility_Lambda([this] { return Field->Presentation().Journey.bVisible || (bTacticCinema && Field->ReadTacticCinema().bVisible) ? EVisibility::Collapsed:EVisibility::Visible; })[Content];
    InputPanels.Add(Panel);Canvas->AddSlot().Position(gunyulayout::CommandOrigin).Size(gunyulayout::CommandSize)[Panel];
}
