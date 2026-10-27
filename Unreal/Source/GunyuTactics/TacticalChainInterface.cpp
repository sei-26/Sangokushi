#include "TacticalTrial.h"
#include "TacticalChainVisual.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
using namespace gunyuui;
TSharedRef<SWidget> ATacticalController::ChainPlannerWidget()
{
    auto List=SNew(SScrollBox);
    for(int Index=0;Index<3;++Index)
    {
        auto Step=Button([this,Index]
            { const auto& O=Field->Presentation().Orders;return O.IsValidIndex(Index) ? O[Index].TacticName : FString(TEXT("未予約")); },
            [this,Index] { const auto& O=Field->Presentation().Orders;if(O.IsValidIndex(Index)) Field->SelectOfficer(O[Index].Actor); },
            [this,Index] { return !Field->IsPlaying() && Field->Presentation().Orders.IsValidIndex(Index); },true);
        auto Edit=[this,Index](int Direction,const TCHAR* Text)
        {
            return SNew(SBox).WidthOverride(Direction==0 ? 28:24)
                [Button([Text] { return FString(Text); },[this,Index,Direction] { Field->EditOrder(Index,Direction); },
                [this,Index,Direction]
                { const auto& H=Field->Presentation();return !Field->IsPlaying() && H.bBattle && H.Orders.IsValidIndex(Index) && (Direction!=-1 || Index>0) && (Direction!=1 || H.Orders.IsValidIndex(Index+1)); },true)];
        };
        auto Content=SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[Label([Index] { return gunyuchainui::Number(Index); },19,Gold)]
                + SHorizontalBox::Slot().AutoWidth().Padding(5,0)[PortraitWidget([this,Index] { const auto& O=Field->Presentation().Orders;return O.IsValidIndex(Index) ? O[Index].Actor : -1; },26,32)]
                + SHorizontalBox::Slot().FillWidth(1)[SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[Label([this,Index] { const auto& O=Field->Presentation().Orders;return O.IsValidIndex(Index) ? O[Index].ActorName : TEXT("武将を選択"); },11)]
                    + SVerticalBox::Slot().AutoHeight()[Step]]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom)[Edit(-1,TEXT("↑"))]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom)[Edit(1,TEXT("↓"))]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom)[Edit(0,TEXT("×"))]]
            + SVerticalBox::Slot().AutoHeight().Padding(0,3,0,0)
                [SNew(SBox).HeightOverride(30).Clipping(EWidgetClipping::ClipToBounds)
                 [SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular",11))
                 .ColorAndOpacity_Lambda([this,Index] { const auto& O=Field->Presentation().Orders;return FSlateColor(O.IsValidIndex(Index) ? gunyuchainui::Color(O[Index]) : FLinearColor(.5f,.59f,.60f)); })
                 .Text_Lambda([this,Index] { const auto& O=Field->Presentation().Orders;return FText::FromString(O.IsValidIndex(Index) ? gunyuchainui::Status(O[Index])+TEXT(" / ")+O[Index].Detail.Replace(TEXT("成立 / "),TEXT("")) : TEXT("戦法を選び「予約」で追加")); })
                 .AutoWrapText(true).OverflowPolicy(ETextOverflowPolicy::MultilineEllipsis)]];
        List->AddSlot().Padding(0,0,0,5)
            [SNew(SBox).HeightOverride(82)
             [SNew(SBorder).Padding(6).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
              .BorderBackgroundColor_Lambda([this,Index] { const auto& H=Field->Presentation();return H.Orders.IsValidIndex(Index) && H.Orders[Index].Actor==H.Selected ? FLinearColor(.045f,.095f,.09f,.98f) : FLinearColor(.014f,.029f,.035f,.95f); })
              .ToolTipText_Lambda([this,Index]
              { const auto& H=Field->Presentation();return FText::FromString(H.Orders.IsValidIndex(Index) ? H.Orders[Index].Title+TEXT("\n")+gunyuchainui::Status(H.Orders[Index])+TEXT("\n")+H.Orders[Index].Detail+TEXT("\n戦法名を押すと担当武将を選択。↑↓で順序を変更。") : TEXT("武将と戦法を選んで予約。1手でも発動できます。成功を重ねると火力100・125・150％。")); })[Content]]];
    }
    return List;
}
FString ATacticalController::UnitChainNumber(int Index) const
{
    if(Field->IsPlaying()) return FString();
    const auto& Orders=Field->Presentation().Orders;
    for(int I=0;I<Orders.Num();++I) if(Orders[I].Actor==Index) return gunyuchainui::Number(I)+TEXT(" ");
    return FString();
}
