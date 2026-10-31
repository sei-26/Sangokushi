#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
using namespace gunyuui;
namespace
{
    FLinearColor PositionColor(const FTacticPositionCell& Cell)
    { return !Cell.bMoveAllowed ? FLinearColor(.42f,.42f,.42f):Cell.bReady ? FLinearColor(.77f,.56f,1.f):FLinearColor(.43f,.65f,.73f); }
}
void ATacticalController::OpenTacticPositionComparison()
{
    if(!Field->BeginTacticPositionComparison()) return;
    bEnemyDetailsOpen=bMarchPlanOpen=bBattleLogOpen=bTacticPickerOpen=false;
    const auto& Units=Field->VisualUnits();const int Id=Field->Presentation().Selected;
    if(Units.IsValidIndex(Id) && Units[Id].Id>=0) Focus=Units[Id].Position;
    IgnoreMapInputThroughFrame=Frames+1;LabelLayoutFrame=MAX_uint64;
}
void ATacticalController::BuildTacticPositionInterface(TSharedRef<SCanvas> Canvas)
{
    auto Choices=SNew(SUniformGridPanel).SlotPadding(FMargin(2));
    auto MapMarkers=SNew(SCanvas).Visibility(EVisibility::SelfHitTestInvisible).Clipping(EWidgetClipping::ClipToBounds);
    for(int I=0;I<6;++I)
    {
        auto Text=[this,I]
        {
            const auto& Cells=Field->Presentation().Position.Cells;if(!Cells.IsValidIndex(I)) return FString();
            const auto& C=Cells[I];return FString::Printf(TEXT("%d  %s"),I+1,!C.bMoveAllowed ? TEXT("進めない"):C.bReady ? TEXT("戦法可"):TEXT("戦法不可"));
        };
        auto Pick=[this,I] { const auto& Cells=Field->Presentation().Position.Cells;if(Cells.IsValidIndex(I)) Field->SelectTacticPosition(Cells[I].Tile);IgnoreMapInputThroughFrame=Frames+1; };
        auto Color=[this,I] { const auto& Cells=Field->Presentation().Position.Cells;return FSlateColor(Cells.IsValidIndex(I) ? PositionColor(Cells[I]):FLinearColor::White); };
        Choices->AddSlot(I%3,I/3)
          [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(4)
           .Visibility_Lambda([this,I] { return Field->Presentation().Position.Cells.IsValidIndex(I) ? EVisibility::Visible:EVisibility::Collapsed; })
           .ButtonColorAndOpacity_Lambda([this,I] { const auto& P=Field->Presentation().Position;return FSlateColor(P.Cells.IsValidIndex(I) && P.SelectedTile==P.Cells[I].Tile ? FLinearColor(2.f,1.5f,2.4f):FLinearColor::White); })
           .OnClicked_Lambda([Pick] { Pick();return FReply::Handled(); })[SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular",11)).Text_Lambda([Text] { return FText::FromString(Text()); }).ColorAndOpacity_Lambda(Color)]];
        auto Marker=SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(1)
          .Visibility_Lambda([this,I] { const auto& P=Field->Presentation().Position;return P.bVisible && !Field->IsPlaying() && P.Cells.IsValidIndex(I) ? EVisibility::Visible:EVisibility::Collapsed; })
          .ButtonColorAndOpacity_Lambda([this,I] { const auto& P=Field->Presentation().Position;return FSlateColor(P.Cells.IsValidIndex(I) && P.SelectedTile==P.Cells[I].Tile ? FLinearColor(2.5f,1.7f,3.f):FLinearColor::White); })
          .ToolTipText_Lambda([this,I] { const auto& C=Field->Presentation().Position.Cells;return FText::FromString(C.IsValidIndex(I) ? C[I].Result+TEXT("\n")+C[I].Risk:FString()); })
          .OnClicked_Lambda([Pick] { Pick();return FReply::Handled(); })
          [SNew(STextBlock).Justification(ETextJustify::Center).Font(FCoreStyle::GetDefaultFontStyle("Regular",14))
           .Text(FText::AsNumber(I+1)).ColorAndOpacity_Lambda(Color)];
        InputPanels.Add(Marker);MapMarkers->AddSlot().Position_Lambda([this,I]
        {
            const auto& P=Field->Presentation().Position;
            return P.bVisible && P.Cells.IsValidIndex(I) ? MapLabelPosition(Field->SurfaceLocation(P.Cells[I].Tile)+FVector(0,68,35),FVector2D(28,26))-gunyulayout::MapOrigin:FVector2D(-2000,-2000);
        }).Size(FVector2D(28,26))[Marker];
    }
    Canvas->AddSlot().Position(gunyulayout::MapOrigin).Size(gunyulayout::MapSize)[MapMarkers];
    auto Texts=SNew(SVerticalBox)
      + SVerticalBox::Slot().AutoHeight()[Label([this] { return Field->bRealtimeCommand ? TEXT("配置と戦法の比較 / 停止中"):TEXT("配置と戦法の比較"); },18,Gold)]
      + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Label([this] { return Field->Presentation().Position.Title; },12)]
      + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("紫＝移動後に成立見込み ／ 隣接1歩を比較"); },11,FLinearColor(.77f,.56f,1.f))]
      + SVerticalBox::Slot().AutoHeight().Padding(0,5)[Choices]
      + SVerticalBox::Slot().AutoHeight()[ReliefChoicesWidget()];
    auto Read=SNew(SScrollBox)
      + SScrollBox::Slot()[SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Position.Relief.bVisible ? EVisibility::Visible:EVisibility::Collapsed; })
          [SNew(SVerticalBox)
           + SVerticalBox::Slot().AutoHeight()[Label([this] { return Field->Presentation().Position.Relief.Detail; },12,Gold)]
           + SVerticalBox::Slot().AutoHeight().Padding(0,5)[Label([this] { return Field->Presentation().Position.Relief.Cost; },11)]]]
        + SVerticalBox::Slot().AutoHeight().Padding(0,6,0,0)[Label([this] { return Field->Presentation().Position.Result; },13,Gold)]
        + SVerticalBox::Slot().AutoHeight().Padding(0,7)[Label([this] { return Field->Presentation().Position.Risk; },12,FLinearColor(1.f,.73f,.51f))]
        + SVerticalBox::Slot().AutoHeight()[Label([this] { return Field->Presentation().Position.Timing; },11)]];
    Texts->AddSlot().FillHeight(1)[Read];
    Texts->AddSlot().AutoHeight().Padding(0,6,0,0)
      [SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Position.Relief.bVisible ? EVisibility::Visible:EVisibility::Collapsed; })
       [Button([this] { return Field->Presentation().Position.Relief.ConfirmLabel; },[this] { Field->CommitRelief();IgnoreMapInputThroughFrame=Frames+1; },[this] { return !Field->IsPlaying() && Field->Presentation().Position.Relief.bCanIssue; },true)]];
    Texts->AddSlot().AutoHeight().Padding(0,4,0,0)[Button([this] { return Field->Presentation().Position.ConfirmLabel; },[this] { Field->CommitTacticPosition();IgnoreMapInputThroughFrame=Frames+1; },[this] { return !Field->IsPlaying() && Field->Presentation().Position.bCanCommit; },true)];
    Texts->AddSlot().AutoHeight().Padding(0,4,0,0)[Button([] { return TEXT("比較を閉じる / Esc"); },[this] { Field->Issue(ETacticalCommand::Cancel);IgnoreMapInputThroughFrame=Frames+1; },[] { return true; },true)];
    auto Panel=SNew(STacticalChrome).Padding(12).Accent(FLinearColor(.59f,.35f,.82f))
      .Visibility_Lambda([this] { return Field->Presentation().Position.bVisible && !Field->IsPlaying() ? EVisibility::Visible:EVisibility::Collapsed; })[Texts];
    InputPanels.Add(Panel);Canvas->AddSlot().Position(FVector2D(16,174)).Size_Lambda([this] { return FVector2D(352,Field->Presentation().Position.Relief.bVisible ? 478:360); })[Panel];
}
bool ATacticalController::TacticPositionInterfaceSmokeTest()
{
    const auto SavedFocus=Focus;
    Field->PrepareTacticPositionPreview(0);Field->Issue(ETacticalCommand::Cancel);
    bEnemyDetailsOpen=bMarchPlanOpen=bBattleLogOpen=true;OpenTacticPositionComparison();
    const auto& P=Field->Presentation().Position;
    bool Passed=P.bVisible && !P.bCanCommit && !bEnemyDetailsOpen && !bMarchPlanOpen && !bBattleLogOpen && Focus.Equals(Field->VisualUnits()[2].Position);
    const auto Before=Field->VisualUnits()[2].Position;int Tile=-1;
    for(const auto& C:P.Cells) if(C.bReady) { Tile=C.Tile;break; }
    Field->ClickTile(Tile);
    Passed=Passed && Tile>=0 && Field->Presentation().Position.SelectedTile==Tile && Field->Presentation().Position.bCanCommit && Field->VisualUnits()[2].Position.Equals(Before);
    Field->Issue(ETacticalCommand::Cancel);
    Passed=Passed && !Field->Presentation().Position.bVisible && Field->VisualUnits()[2].Position.Equals(Before);
    Focus=SavedFocus;
    if(Passed) { UE_LOG(LogTemp,Display,TEXT("GUNYU_TACTIC_POSITION_UI: PASS (exclusive comparison, explicit hypothetical selection, normal cancel and unchanged visible army)")); }
    else { UE_LOG(LogTemp,Error,TEXT("GUNYU_TACTIC_POSITION_UI: FAIL")); }
    return Passed;
}
