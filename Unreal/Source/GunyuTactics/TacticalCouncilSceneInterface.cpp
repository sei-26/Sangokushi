#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
using namespace gunyuui;
void ATacticalController::BuildCouncilScene(TSharedRef<SCanvas> Canvas)
{
    auto Visible=[this] { return CouncilSceneVisible() ? EVisibility::Visible:EVisibility::Collapsed; };
    auto MultipleLines=[this] { return CouncilExchange.Lines.Num()>1 ? EVisibility::Visible:EVisibility::Collapsed; };
    BuildEnsemblePortraits(Canvas,true);
    auto Header=SNew(STacticalChrome).Padding(FMargin(22,12)).Opacity(.86f).Visibility_Lambda(Visible)
      [SNew(SHorizontalBox)
       + SHorizontalBox::Slot().FillWidth(1)[SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[Label([this] { return Field->Presentation().Journey.PlanObjective; },14,Gold)]
          + SVerticalBox::Slot().AutoHeight().Padding(0,3)[Label([this] { return CouncilExchange.Subject; },25)]
          + SVerticalBox::Slot().AutoHeight()[SNew(SBox).Visibility_Lambda([this]
             { return Field->Presentation().Journey.CampaignCarryover.IsEmpty() ? EVisibility::Collapsed:EVisibility::Visible; })
             [Label([this] { return Field->Presentation().Journey.CampaignCarryover; },13,Gold)]]]
       + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[Button([] { return TEXT("配置・物資を詳しく"); },[this] { bCouncilLedger=true; })]];
    Canvas->AddSlot().Position(FVector2D(40,24)).Size_Lambda([this]
        { return FVector2D(1520,Field->Presentation().Journey.CampaignCarryover.IsEmpty() ? 94:108); })[Header];InputPanels.Add(Header);
    for(int I=0;I<3;++I)
    {
        auto Voice=SNew(SBox).Visibility_Lambda(Visible)
          [SNew(SButton).ButtonStyle(&CommandStyle()).ContentPadding(FMargin(18,8)).IsFocusable(false)
           .OnClicked_Lambda([this,I] { OpenCouncilVoice(I);return FReply::Handled(); })
           [SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[Label([I] { return I==0 ? TEXT("劉備"):I==1 ? TEXT("関羽"):TEXT("張飛"); },26,Gold)]
            + SVerticalBox::Slot().AutoHeight()[Label([this,I]
              {
                  const auto& HUD=Field->Presentation();const auto& Roles=HUD.Journey.Map.CouncilRoles;
                  if(Roles.IsValidIndex(I)) return (HUD.Mission==0 && HUD.Journey.bShortPrelude ? FString(TEXT("相談：")):FString())+Roles[I].Title;
                  return FString(I==0 ? TEXT("任務を確かめる"):I==1 ? TEXT("先遣伏兵"):TEXT("予備糧"));
              },17)]]];
        Canvas->AddSlot().Position(FVector2D(48+I*510,372)).Size(FVector2D(484,72))[Voice];InputPanels.Add(Voice);
        auto Reaction=SNew(STacticalChrome).Padding(FMargin(16,7)).Opacity(.82f)
          .Visibility_Lambda([this,I] { return CouncilSceneVisible() && !CouncilReaction(I).IsEmpty() ? EVisibility::HitTestInvisible:EVisibility::Collapsed; })
          [Label([this,I] { return TEXT("「")+CouncilReaction(I)+TEXT("」"); },16,Gold)];
        Canvas->AddSlot().Position(FVector2D(62+I*510,452)).Size(FVector2D(456,44))[Reaction];
    }
    auto Dialogue=SNew(SBorder).Padding(0).BorderImage(FCoreStyle::Get().GetBrush("NoBrush")).Visibility_Lambda(Visible)
      .OnMouseButtonDown_Lambda([this](const FGeometry&,const FPointerEvent& E)
        { if(E.GetEffectingButton()==EKeys::LeftMouseButton) { NextCouncilLine();return FReply::Handled(); }return FReply::Unhandled(); })
      [SNew(STacticalChrome).Padding(FMargin(28,14)).Opacity(1.f)
       [SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
          + SHorizontalBox::Slot().FillWidth(1)[Label([this] { return CurrentCouncilLine().Name; },26,Gold)]
          + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SBox).Visibility_Lambda(MultipleLines)
            [Label([this] { return FString::Printf(TEXT("%d / %d"),CouncilPage+1,CouncilExchange.Lines.Num()); },15,Gold)]]
          + SHorizontalBox::Slot().AutoWidth().Padding(18,0)[SNew(SBox).Visibility_Lambda(MultipleLines)
            [Button([] { return TEXT("前の台詞 / ←"); },[this] { HandleEpisodeKey(EKeys::Left); },[this] { return CouncilPage>0; })]]
          + SHorizontalBox::Slot().AutoWidth()[Button([this] { return bNovelInstant ? TEXT("文字：一括"):TEXT("文字：順次"); },[this] { bNovelInstant=!bNovelInstant; })]]
        + SVerticalBox::Slot().FillHeight(1).Padding(0,10)[SNew(SScrollBox)+SScrollBox::Slot()
          [SNew(STextBlock).Font(gunyustyle::Font(28)).ColorAndOpacity(FLinearColor(.99f,.98f,.94f))
           .ShadowOffset(FVector2D(1,1)).ShadowColorAndOpacity(FLinearColor::Black).LineHeightPercentage(1.2f)
           .AutoWrapText(true).WrapTextAt(1380.f).Text_Lambda([this] { return FText::FromString(VisibleCouncilText()); })]]
        + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
          + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Label([this] { return CurrentCouncilLine().Direction; },15)]
          + SHorizontalBox::Slot().AutoWidth().Padding(16,0,0,0)[SNew(SBox).Visibility_Lambda([this]
            { return VisibleCouncilText().Len()<CurrentCouncilLine().Text.Len() || CouncilPage+1<CouncilExchange.Lines.Num() ? EVisibility::Visible:EVisibility::Collapsed; })
            [Button([this] { return VisibleCouncilText().Len()<CurrentCouncilLine().Text.Len() ? TEXT("全文を見る"):TEXT("次の言葉へ / Enter"); },
                [this] { NextCouncilLine(); },[] { return true; },true)]]]]];
    Canvas->AddSlot().Position(FVector2D(48,510)).Size(FVector2D(1504,230))[Dialogue];InputPanels.Add(Dialogue);
    auto Choices=SNew(SHorizontalBox).Visibility_Lambda(Visible);
    for(int I=0;I<2;++I)
        Choices->AddSlot().FillWidth(1).Padding(I==0 ? FMargin(0,0,16,0):FMargin(0))
          [Button([this,I] { return FString::Printf(TEXT("%d  "),I+1)+CouncilChoiceLabel(I); },[this,I] { CouncilChoice(I); })];
    Canvas->AddSlot().Position(FVector2D(48,752)).Size(FVector2D(1504,68))[Choices];InputPanels.Add(Choices);
    auto Footer=SNew(SHorizontalBox).Visibility_Lambda(Visible)
      + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Label([this] { const auto& P=Field->Presentation().Journey;const auto* Plan=P.Plans.FindByPredicate([](const FPreparationPresentation& V) { return V.bSelected; });
          if(!Plan || !Plan->bAffordable) return P.PlanReadiness;
          auto Cost=FString::Printf(TEXT("%s ／ 兵糧 %d"),*Plan->Title,Plan->Food);
          if(Plan->Gold) Cost+=FString::Printf(TEXT(" ／ 案内料 %d"),Plan->Gold);
          return Cost; },14,Gold)]
      + SHorizontalBox::Slot().AutoWidth()
        [SNew(SBox).Visibility_Lambda([this]
         {
             const auto& P=Field->Presentation().Journey;
             const bool Needed=CouncilTopic>0 || (CouncilTopic==0 && P.bChoosePromise) || (CouncilTopic<0 && !P.ExampleReason.IsEmpty());
             return Needed && !P.Map.CouncilRoles.IsEmpty() ? EVisibility::Visible:EVisibility::Collapsed;
         })
         [Button([] { return TEXT("作戦図 / M"); },[this] { OpenCouncilMap(); })]]
      + SHorizontalBox::Slot().AutoWidth().Padding(20,0)[Button([] { return TEXT("皆の意見に戻る"); },[this] { OpenCouncilVoice(-1); })]
      + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(320)
        [Button([this]
            { const auto& P=Field->Presentation().Journey;return P.SetbackLabel.IsEmpty() ? FString(TEXT("この策で、出陣だ！ / D")):P.SetbackLabel; },
            [this] { if(Field->CanIssue(ETacticalCommand::AcceptSetback)) ResolveCampaignSetback(); else DeployNovelPlan(); },
            [this] { return Field->CanIssue(ETacticalCommand::Deploy) || Field->CanIssue(ETacticalCommand::AcceptSetback); })]];
    auto FooterPanel=SNew(STacticalChrome).Padding(FMargin(12,3)).Opacity(.94f).Visibility_Lambda(Visible)[Footer];
    Canvas->AddSlot().Position(FVector2D(48,830)).Size(FVector2D(1504,56))[FooterPanel];InputPanels.Add(FooterPanel);
}
