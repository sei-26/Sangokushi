#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SOverlay.h"
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
        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,12,0)[Button([this] { return bNovelAdvice ? TEXT("案を比較 / A"):TEXT("武将の見立て / A"); },[this] { bNovelMap=false;bNovelAdvice=!bNovelAdvice; })]
        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,12,0)[Button([this] { return bNovelMap ? TEXT("作戦の比較 / M"):TEXT("配置を考える / M"); },[this] { TogglePlanningMap(); })]
        + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("軍議に戻る"); },[this] { CloseEpisode(); })]];
    Contents->AddSlot().AutoHeight().Padding(0,4,0,8)
      [SNew(SHorizontalBox)
       + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
         [Label([this] { return Field->Presentation().Journey.Episode.Title+TEXT(" ／ 出陣前の決断"); },14)]
       + SHorizontalBox::Slot().AutoWidth()[BuildDifficultyChoice()]];
    Contents->AddSlot().AutoHeight().Padding(0,0,0,8)[Label([this] { return TEXT("難易度：")+Field->Presentation().Journey.DifficultyName+TEXT(" ／ ")+Field->Presentation().Journey.DifficultyDetail; },13,Gold)];
    Contents->AddSlot().AutoHeight()[SNew(SBox).Padding(FMargin(0,0,0,12)).Visibility_Lambda([this] { return bNovelMap ? EVisibility::Collapsed:EVisibility::Visible; })[Label([this] { return Field->Presentation().Journey.PlanDifference; },15,Gold)]];
    Contents->AddSlot().AutoHeight().Padding(0,0,0,7)
      [SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Journey.bCampaignFlow && !bNovelMap ? EVisibility::Visible:EVisibility::Collapsed; })
       [SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[Button([] { return TEXT("この局面の物語を読む ／ H"); },[this] { bNovelPlan=false; })]]];
    auto Body=SNew(SVerticalBox);
    Body->AddSlot().AutoHeight()[Label([] { return TEXT("果たすべき役目"); },18,Gold)];
    Body->AddSlot().AutoHeight().Padding(0,7)[Label([this] { return Field->Presentation().Journey.PlanObjective; },20)];
    Body->AddSlot().AutoHeight()[Label([this] { return Field->Presentation().Journey.PlanRoute; },15)];
    Body->AddSlot().AutoHeight().Padding(0,6,0,10)[SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Journey.PledgeText.IsEmpty() ? EVisibility::Collapsed:EVisibility::Visible; })
        [Label([this] { return Field->Presentation().Journey.PledgeText; },18,Gold)]];
    auto Plans=SNew(SHorizontalBox);
    for(int Index=0;Index<2;++Index)
    {
        Plans->AddSlot().FillWidth(1).Padding(Index==0 ? FMargin(0,0,12,0):FMargin(0))
            [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(18)
             .OnClicked_Lambda([this,Index] { SelectNovelPlan(Index);return FReply::Handled(); })
             [SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[Label([this,Index] { const auto& P=Field->Presentation().Journey.Plans;return P.IsValidIndex(Index) ? FString::Printf(TEXT("%d　%s %s"),Index+1,P[Index].bSelected ? TEXT("●"):TEXT("○"),*P[Index].Title):FString(); },24,Gold)]
              + SVerticalBox::Slot().AutoHeight().Padding(0,8)[Label([this,Index] { const auto& P=Field->Presentation().Journey.Plans;return P.IsValidIndex(Index) ? P[Index].Description:FString(); },16)]
              + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Label([this,Index] { const auto& P=Field->Presentation().Journey.Plans;return P.IsValidIndex(Index) ? P[Index].Tradeoff:FString(); },13,Gold)]
              + SVerticalBox::Slot().AutoHeight().Padding(0,5)[Label([this,Index] { const auto& P=Field->Presentation().Journey.Plans;return P.IsValidIndex(Index) ? P[Index].MoraleText:FString(); },12,Gold)]
              + SVerticalBox::Slot().AutoHeight().Padding(0,6)[Label([this,Index] { const auto& P=Field->Presentation().Journey.Plans;return P.IsValidIndex(Index) ? P[Index].Cost:FString(); },15,Gold)]
              + SVerticalBox::Slot().AutoHeight()[Label([this,Index] { const auto& P=Field->Presentation().Journey.Plans;return P.IsValidIndex(Index) ? P[Index].Remaining:FString(); },13)]]];
    }
    Body->AddSlot().AutoHeight().Padding(0,8,0,12)[SNew(SBox).Visibility_Lambda([this] { return bNovelAdvice ? EVisibility::Collapsed:EVisibility::Visible; })[Plans]];
    Body->AddSlot().AutoHeight().Padding(0,0,0,8)
      [SNew(SBox).Visibility_Lambda([this] { const auto& J=Field->Presentation().Journey;return J.bCampaignFlow && !bNovelAdvice && J.Plans.IsValidIndex(0) && J.Plans[0].bSelected ? EVisibility::Visible:EVisibility::Collapsed; })
       [Button([] { return TEXT("関羽の先遣地点を地図で決める ／ 通常位置から陸路２歩以内"); },[this]
        { TogglePlanningMap();SelectPlanningOfficer(1);PlanningNotice=TEXT("森林のマス → 伏兵を指定。２歩以内なら、その位置へ先行して開戦できる。"); })]];
    auto Advice=SNew(SVerticalBox).Visibility_Lambda([this] { return bNovelAdvice ? EVisibility::Visible:EVisibility::Collapsed; });
    for(int I=0;I<3;++I)
    {
        Advice->AddSlot().AutoHeight().Padding(0,8)
          [SNew(SHorizontalBox)
           + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([I] { return I; },42,50)]
           + SHorizontalBox::Slot().FillWidth(1).Padding(14,0)
             [SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[Label([this,I] { const auto& A=Field->Presentation().Journey.PlanAdvice;return A.IsValidIndex(I) ? A[I].Name:FString(); },18,Gold)]
              + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Label([this,I] { const auto& A=Field->Presentation().Journey.PlanAdvice;return A.IsValidIndex(I) ? A[I].Text:FString(); },18)]]];
    }
    Body->AddSlot().AutoHeight()[Advice];
    Contents->AddSlot().FillHeight(1)[SNew(SOverlay)
        + SOverlay::Slot()[SNew(SScrollBox).Visibility_Lambda([this] { return bNovelMap ? EVisibility::Collapsed:EVisibility::Visible; })+SScrollBox::Slot()[Body]]
        + SOverlay::Slot()[BuildPlanningMap()]];
    Contents->AddSlot().AutoHeight().Padding(0,14,0,8)[Label([this] { return Field->Presentation().Journey.PlanReadiness; },16,Gold)];
    Contents->AddSlot().AutoHeight()[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Label([this] { return bNovelMap ? (Field->Presentation().Journey.Map.bChooseRoute ? TEXT("1・2：準備案　M：比較　R：経路　Esc：作戦書"):TEXT("1・2：準備案　M：比較　Esc：作戦書")):TEXT("1・2：準備案　A：助言　M：地図　Esc：会話へ"); },14)]
        + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(360).HeightOverride(48)
          [Button([this] { return Field->CanIssue(ETacticalCommand::Deploy) ? TEXT("この作戦で出陣する / D"):TEXT("出陣できない：資源を確認"); },[this] { DeployNovelPlan(); },[this] { return Field->CanIssue(ETacticalCommand::Deploy); })]]];
    auto Panel=SNew(STacticalChrome).Padding(28).Opacity(1.f).Accent(gunyustyle::Brass).Visibility_Lambda(Visible)[Contents];
    Canvas->AddSlot().Position(FVector2D(150,50)).Size(FVector2D(1300,800))[Panel];InputPanels.Add(Panel);
}
