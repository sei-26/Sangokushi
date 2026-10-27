#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
using namespace gunyuui;
void ATacticalController::BuildNovelPlan(TSharedRef<SCanvas> Canvas)
{
    auto Visible=[this] { return bEpisodeOpen && bNovelPlan && Field->Presentation().Journey.bPreparation ? EVisibility::Visible:EVisibility::Collapsed; };
    auto Shade=SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.002f,.004f,.007f,.86f)).Visibility_Lambda(Visible);
    Canvas->AddSlot().Position(FVector2D::ZeroVector).Size(FVector2D(1600,900))[Shade];InputPanels.Add(Shade);
    auto Contents=SNew(SVerticalBox);
    Contents->AddSlot().AutoHeight()[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1)[Label([] { return TEXT("軍師の作戦書"); },34,Gold)]
        + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("軍議に戻る"); },[this] { CloseEpisode(); })]];
    Contents->AddSlot().AutoHeight().Padding(0,4,0,14)[Label([this] { return Field->Presentation().Journey.Episode.Title+TEXT("　／　出陣前の決断"); },14)];
    auto Body=SNew(SVerticalBox);
    Body->AddSlot().AutoHeight()[Label([] { return TEXT("果たすべき役目"); },18,Gold)];
    Body->AddSlot().AutoHeight().Padding(0,7)[Label([this] { return Field->Presentation().Journey.Outcome; },20)];
    Body->AddSlot().AutoHeight()[Label([this] { return Field->Presentation().Journey.ConvoyRouteInfo; },15)];
    Body->AddSlot().AutoHeight().Padding(0,6,0,10)[Label([this] { return Field->Presentation().Journey.PledgeText; },18,Gold)];
    auto Plans=SNew(SHorizontalBox);
    for(int Index=0;Index<2;++Index)
    {
        Plans->AddSlot().FillWidth(1).Padding(Index==0 ? FMargin(0,0,12,0):FMargin(0))
            [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(18)
             .OnClicked_Lambda([this,Index] { SelectNovelPlan(Index);return FReply::Handled(); })
             [SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[Label([this,Index] { const auto& P=Field->Presentation().Journey.Plans;return P.IsValidIndex(Index) ? FString::Printf(TEXT("%d　%s %s"),Index+1,P[Index].bSelected ? TEXT("●"):TEXT("○"),*P[Index].Title):FString(); },24,Gold)]
              + SVerticalBox::Slot().AutoHeight().Padding(0,12)[Label([this,Index] { const auto& P=Field->Presentation().Journey.Plans;return P.IsValidIndex(Index) ? P[Index].Description:FString(); },16)]
              + SVerticalBox::Slot().AutoHeight().Padding(0,6)[Label([this,Index] { const auto& P=Field->Presentation().Journey.Plans;return P.IsValidIndex(Index) ? P[Index].Cost:FString(); },18,Gold)]
              + SVerticalBox::Slot().AutoHeight()[Label([this,Index] { const auto& P=Field->Presentation().Journey.Plans;return P.IsValidIndex(Index) ? P[Index].Remaining:FString(); },16)]]];
    }
    Body->AddSlot().AutoHeight().Padding(0,8,0,18)[Plans];
    Body->AddSlot().AutoHeight()[Label([this] { return Field->Presentation().Journey.PersonalTrust; },15,Gold)];
    Contents->AddSlot().FillHeight(1)[SNew(SScrollBox)+SScrollBox::Slot()[Body]];
    Contents->AddSlot().AutoHeight().Padding(0,18,0,8)[Label([] { return TEXT("皆の配置と帰る道を、この一策で引き受ける。出陣すると兵糧と軍資金を使う。"); },16)];
    Contents->AddSlot().AutoHeight()[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Label([] { return TEXT("1・2：準備案を選ぶ　／　Esc：会話へ戻る"); },14)]
        + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(360).HeightOverride(48)
          [Button([this] { return Field->CanIssue(ETacticalCommand::Deploy) ? TEXT("この作戦で出陣する / D"):TEXT("出陣できない：資源を確認"); },[this] { DeployNovelPlan(); },[this] { return Field->CanIssue(ETacticalCommand::Deploy); })]]];
    auto Panel=SNew(STacticalChrome).Padding(28).Opacity(1.f).Accent(gunyustyle::Brass).Visibility_Lambda(Visible)[Contents];
    Canvas->AddSlot().Position(FVector2D(210,116)).Size(FVector2D(1180,672))[Panel];InputPanels.Add(Panel);
}
