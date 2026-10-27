#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
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
          + SHorizontalBox::Slot().FillWidth(1).Padding(0, 0, 0, 4)[Command(ETacticalCommand::Hold, [] { return TEXT("固守"); })] ];
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
    AddBattlePanel(Canvas,gunyulayout::CommandOrigin,gunyulayout::CommandSize,Orders);
}
