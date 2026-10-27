#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"

using namespace gunyuui;

void ATacticalController::BuildJourneyInterface(TSharedRef<SCanvas> Canvas)
{
    auto Command = [this](ETacticalCommand Action, TFunction<FString()> Name)
    {
        return Button(Name, [this, Action] { Field->Issue(Action);if(Action==ETacticalCommand::TalkLiu || Action==ETacticalCommand::TalkGuan || Action==ETacticalCommand::TalkZhang) OpenOfficerNovel(); }, [this, Action] { return Field->CanIssue(Action); });
    };
    auto Contents = SNew(SVerticalBox);
    Contents->AddSlot().AutoHeight()[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,16,0)
          [SNew(SBox).WidthOverride(46).HeightOverride(46)[SNew(STacticalChrome).Padding(5).Accent(FLinearColor(.7f,.4f,.18f))
            [Label([] { return TEXT("策"); },28,Gold)]]]
        + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
          [SNew(SVerticalBox)
           + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("黄巾の乱    /    軍師の旅路"); },11,Gold)]
           + SVerticalBox::Slot().AutoHeight()[Label([this] { return Field->Presentation().Journey.Heading; },28)]]];
    Contents->AddSlot().AutoHeight().Padding(0, 12, 0, 18)[Label([this] { return Field->Presentation().Journey.Resources; }, 16, Gold)];
    Contents->AddSlot().AutoHeight().Padding(0,0,0,8)
      [SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Journey.PledgeText.IsEmpty() ? EVisibility::Collapsed:EVisibility::Visible; })
       [Label([this] { return Field->Presentation().Journey.PledgeText; },16,Gold)]];
    auto Text = SNew(SVerticalBox);
    Text->AddSlot().AutoHeight().Padding(0,0,0,12)
        [SNew(STacticalChrome).Visibility_Lambda([this] { return Field->Presentation().Journey.bPreparation ? EVisibility::Visible : EVisibility::Collapsed; })
         .Padding(12).Accent(FLinearColor(.64f,.45f,.16f))
         [Label([this] { return Field->Presentation().Journey.Outcome; },17,Gold)]];
    auto People = SNew(SHorizontalBox).Visibility_Lambda([this] { const auto& P = Field->Presentation().Journey; return P.bPreparation || P.bDebrief ? EVisibility::Visible : EVisibility::Collapsed; });
    const ETacticalCommand Talks[] = {ETacticalCommand::TalkLiu, ETacticalCommand::TalkGuan, ETacticalCommand::TalkZhang};
    const FString Names[] = {TEXT("劉備と話す"), TEXT("関羽と話す"), TEXT("張飛と話す")};
    for (int Index = 0; Index < 3; ++Index)
        People->AddSlot().FillWidth(1).Padding(0, 0, 8, 12)[Command(Talks[Index], [Name = Names[Index]] { return Name; })];
    Text->AddSlot().AutoHeight()
        [SNew(SVerticalBox).Visibility_Lambda([this] { return Field->Presentation().Journey.bChoosePromise ? EVisibility::Visible : EVisibility::Collapsed; })
         + SVerticalBox::Slot().AutoHeight().Padding(0, 8)[Label([] { return TEXT("この戦いで、どこまで守ると約束するか"); }, 17, Gold)]
         + SVerticalBox::Slot().AutoHeight()
           [SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).Padding(0, 0, 8, 0)[Command(ETacticalCommand::PromiseRoad, [this] { return Field->Presentation().Journey.bRescue ? TEXT("○ 退路を開く") : TEXT("● 退路を開く"); })]
            + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::PromiseRescue, [this] { return Field->Presentation().Journey.bRescue ? TEXT("● 避難車まで送り届ける") : TEXT("○ 避難車まで送り届ける"); })] ]
         + SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 12)[Label([this] { return Field->Presentation().Journey.PromiseText; }, 15)] ];
    Text->AddSlot().AutoHeight().Padding(0,0,0,12)
        [SNew(STacticalChrome).Padding(12).Accent(FLinearColor(.58f,.23f,.12f))
         [SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
             + SHorizontalBox::Slot().FillWidth(1)[Label([this] { return Field->Presentation().Journey.Episode.Title; },23,Gold)]
             + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("本幕を読む / H"); },[this] { OpenEpisode(); })]]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Label([this] { return Field->Presentation().Journey.Episode.Setting; },15)]
          + SVerticalBox::Slot().AutoHeight()[Label([this] { return Field->Presentation().Journey.Episode.Question; },16,Gold)]]];
    Text->AddSlot().AutoHeight()[OpeningRouteChoices()];
    Text->AddSlot().AutoHeight()[People];
    Text->AddSlot().AutoHeight()[Label([this] { return Field->Presentation().Journey.Speaker; }, 25, Gold)];
    Text->AddSlot().AutoHeight().Padding(0,10,0,12)[StoryConversationWidget()];
    Text->AddSlot().AutoHeight().Padding(0, 0, 0, 16)
        [SNew(STacticalChrome).Visibility_Lambda([this] { return Field->Presentation().Journey.bPreparation ? EVisibility::Collapsed : EVisibility::Visible; })
         .Padding(14).Accent(FLinearColor(.64f,.45f,.16f))
         [Label([this] { return Field->Presentation().Journey.Outcome; }, 17, Gold)]];
    Text->AddSlot().AutoHeight().Padding(0,0,0,8)[Label([this] { return Field->Presentation().Journey.ReviewFacts; },15)];
    Text->AddSlot().AutoHeight().Padding(0,0,0,8)[Label([this] { return Field->Presentation().Journey.StoryFacts; },15,Gold)];
    Text->AddSlot().AutoHeight().Padding(0,0,0,10)[Label([this] { return Field->Presentation().Journey.ReviewLesson; },16,Gold)];
    Text->AddSlot().AutoHeight().Padding(0,0,0,10)
      [SNew(SBox).Visibility_Lambda([this] { const auto& J=Field->Presentation().Journey;return (J.bDebrief || J.bComplete) && Field->HasChainReview() ? EVisibility::Visible:EVisibility::Collapsed; })
       [Button([] { return TEXT("直近の連鎖を振り返る / Y"); },[this] { ToggleChainReview(); },[this] { return Field->HasChainReview(); })]];
    auto Plans = SNew(SVerticalBox).Visibility_Lambda([this] { return Field->Presentation().Journey.bPreparation ? EVisibility::Visible : EVisibility::Collapsed; });
    for (int Index = 0; Index < 2; ++Index)
    {
        Plans->AddSlot().AutoHeight().Padding(0, 0, 0, 10)
        [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(14)
         .ButtonColorAndOpacity_Lambda([this, Index] { return FSlateColor(Field->Presentation().Journey.Plans.IsValidIndex(Index) && Field->Presentation().Journey.Plans[Index].bSelected ? FLinearColor(2.7f,2.25f,1.3f) : FLinearColor::White); })
         .OnClicked_Lambda([this, Index] { Field->Issue(Index == 0 ? ETacticalCommand::PlanAmbush : ETacticalCommand::PlanSupplies); return FReply::Handled(); })
         [SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[Label([this, Index] { const auto& P = Field->Presentation().Journey.Plans; return P.IsValidIndex(Index) ? (P[Index].bSelected ? TEXT("● ") : TEXT("○ ")) + P[Index].Title : FString(); }, 21, Gold)]
          + SVerticalBox::Slot().AutoHeight().Padding(0, 7)[Label([this, Index] { const auto& P = Field->Presentation().Journey.Plans; return P.IsValidIndex(Index) ? P[Index].Cost : FString(); }, 15, Gold)]
          + SVerticalBox::Slot().AutoHeight()[Label([this, Index] { const auto& P = Field->Presentation().Journey.Plans; return P.IsValidIndex(Index) ? P[Index].Remaining : FString(); }, 14)] ] ];
    }
    Text->AddSlot().AutoHeight()[Label([this] { for (const auto& P : Field->Presentation().Journey.Plans) if (P.bSelected) return TEXT("選んだ準備: ") + P.Description; return FString(); }, 16, Gold)];
    Text->AddSlot().AutoHeight().Padding(0, 16, 0, 0)[Label([this] { return Field->Presentation().Journey.bDebrief ? FString() : Field->Presentation().Report; }, 15)];
    Contents->AddSlot().FillHeight(1)
        [SNew(SHorizontalBox)
         + SHorizontalBox::Slot().FillWidth(1)
           [SNew(SScrollBox)+SScrollBox::Slot()[Text]]
         + SHorizontalBox::Slot().AutoWidth().Padding(26, 0, 0, 0)
           [SNew(SBox).WidthOverride(292)[SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Journey.bPreparation ? EVisibility::Collapsed : EVisibility::Visible; })[SNew(STacticalChrome).Padding(3).Accent(FLinearColor(.62f,.45f,.20f))[SNew(SBox).HeightOverride_Lambda([this] { return Field->Presentation().Journey.bNeedsReply ? 200.f:310.f; })[PortraitWidget([this] { return Field->Presentation().Journey.Portrait; }, 292, 310)]]]]
            + SVerticalBox::Slot().AutoHeight()[SNew(SBox).WidthOverride(292).Visibility_Lambda([this] { return Field->Presentation().Journey.bPreparation ? EVisibility::Visible : EVisibility::Collapsed; })[SNew(SHorizontalBox)+SHorizontalBox::Slot().AutoWidth()[PortraitWidget([this] { return Field->Presentation().Journey.Portrait; }, 104, 110)]]]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 16)[Label([this] { return Field->Presentation().Journey.Speaker; }, 23, Gold)]
            + SVerticalBox::Slot().AutoHeight()[Plans]
            + SVerticalBox::Slot().AutoHeight()[Label([this]
            {
                const auto& J=Field->Presentation().Journey;if(J.bPreparation) return FString();
                if(J.StoryFacts.IsEmpty()) return FString(TEXT("一策を、次の歴史へ。"));
                const int End=J.StoryFacts.Find(TEXT("\n"));return End==INDEX_NONE ? J.StoryFacts:J.StoryFacts.Left(End);
            },14,Gold)] ] ] ];
    Contents->AddSlot().AutoHeight().Padding(0,6,0,4)[Label([this] { return Field->Presentation().Journey.PersonalTrust; },14,Gold)];
    auto Reply=SNew(SVerticalBox).Visibility_Lambda([this] { return Field->Presentation().Journey.bNeedsReply ? EVisibility::Visible:EVisibility::Collapsed; });
    Reply->AddSlot().AutoHeight()[Label([this] { return Field->Presentation().Journey.ReplyPrompt; },16,Gold)];
    Reply->AddSlot().AutoHeight().Padding(0,5)[SNew(SHorizontalBox)
      + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,8,0)[Command(ETacticalCommand::StoryReplyPreserve,[] { return TEXT("1  主人公：次は、みんなを連れて帰る\n次戦の兵損失を一割以内に抑えて勝つ"); })]
      + SHorizontalBox::Slot().FillWidth(1)[Command(ETacticalCommand::StoryReplyCoordinate,[] { return TEXT("2  主人公：俺が合図を出して、皆をつなぐ\n次戦で二隊以上の戦法をつなぎ、任務を果たす"); })]];
    Contents->AddSlot().AutoHeight()[Reply];
    Contents->AddSlot().AutoHeight().Padding(0, 10, 0, 0)[Label([this] { return Field->Presentation().Journey.History; }, 14)];
    Contents->AddSlot().AutoHeight().Padding(0, 10, 0, 10)[Label([this] { return Field->Presentation().Journey.Relationships; }, 16, Gold)];
    Contents->AddSlot().AutoHeight()
        [SNew(SHorizontalBox)
         + SHorizontalBox::Slot().FillWidth(1)
           [SNew(SBox).Visibility_Lambda([this] { return Field->Presentation().Journey.bPreparation ? EVisibility::Visible : EVisibility::Collapsed; })
            [Command(ETacticalCommand::Reinforce, [] { return TEXT("義勇兵1,200を募る / 軍資金250・兵糧500"); })]]
         + SHorizontalBox::Slot().FillWidth(1).Padding(16, 0, 0, 0)
           [Button([this] { return Field->Presentation().Journey.PrimaryLabel; }, [this]
            {
                if (Field->CanIssue(ETacticalCommand::Dialogue)) Field->Issue(ETacticalCommand::Dialogue);
                else if (Field->Presentation().Journey.bPreparation) Field->Issue(ETacticalCommand::Deploy);
                else if (Field->CanIssue(ETacticalCommand::Continue)) Field->Issue(ETacticalCommand::Continue);
                else if (Field->CanIssue(ETacticalCommand::Retry)) Field->Issue(ETacticalCommand::Retry);
                else if (Field->Presentation().Journey.bComplete) Field->Issue(ETacticalCommand::Restart);
            }, [this]
            {
                return Field->CanIssue(ETacticalCommand::Dialogue) || Field->CanIssue(ETacticalCommand::Deploy) || Field->CanIssue(ETacticalCommand::Continue) || Field->CanIssue(ETacticalCommand::Retry) || Field->Presentation().Journey.bComplete;
            })] ];
    Contents->AddSlot().AutoHeight().Padding(0, 12, 0, 0)[Label([this] { return Field->Presentation().Journey.Context; }, 13)];
    auto Panel = SNew(STacticalChrome)
        .Visibility_Lambda([this] { const auto& J=Field->Presentation().Journey;return J.bPreparation || J.bDebrief || J.bComplete ? EVisibility::Visible : EVisibility::Collapsed; })
        .Padding(28).Accent(FLinearColor(.56f,.42f,.18f))[Contents];
    InputPanels.Add(Panel);
    Canvas->AddSlot().Position(FVector2D(16, 116)).Size(FVector2D(1568, 768))[Panel];
}
