#include "TacticalTrial.h"
#include "HexGrid.hpp"
#include "WorldLayout.hpp"
#include "TacticalWidgets.h"
#include "TacticalTargetVisual.h"
#include "TacticalActionFocus.h"
#include "TacticalMapLabelLayout.h"
#include "TacticalBattleResponse.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
using namespace gunyuui;
FString ATacticalController::GoalDutyStatus(int Index) const
{
    if(Index>=3) return {};
    const auto& H=Field->Presentation();
    const int Tile=Index==0 ? H.GoalTile:Index==1 ? H.SecondGoalTile:SelectedDutyTile();
    return Tile>=0 && Tile==SelectedDutyTile() && Tile==H.DutyProgress.Tile ? H.DutyProgress.Caption:FString();
}

FVector2D ATacticalController::GoalLabelSize(int Index) const
{
    auto Size=gunyumaplabel::GoalSize(Index,!Field->Presentation().Defenses.IsEmpty());
    if(!GoalDutyStatus(Index).IsEmpty()) { Size.X=FMath::Max(Size.X,192.);Size.Y+=18; }
    return Size;
}

void ATacticalController::BuildMapLabels(TSharedRef<SCanvas> Canvas)
{
    for (int Index = 0; Index < 4; ++Index)
        Canvas->AddSlot().Position_Lambda([this,Index] { return GoalLabelPosition(Index); })
        .Size_Lambda([this,Index] { return GoalLabelSize(Index); })
        [SNew(SBorder).Visibility(EVisibility::HitTestInvisible).Clipping(EWidgetClipping::ClipToBounds).Padding(FMargin(8, 4))
         .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.07f, .055f, .02f, .94f))
         [SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[Label([this,Index]
          {
              const auto& H=Field->Presentation();
              if(Index==3) return FString(TEXT("◆ 行軍が重なる地点"));
              return Index==2 ? FString(TEXT("◆ 軍議の担当地点")):TEXT("◆ ")+(Index==0 ? H.GoalLabel:H.SecondGoalLabel);
          },13,Gold)]
          + SVerticalBox::Slot().AutoHeight()[SNew(SVerticalBox)
             .Visibility_Lambda([this,Index] { return Field->Presentation().Defenses.IsValidIndex(Index) ? EVisibility::HitTestInvisible:EVisibility::Collapsed; })
             + SVerticalBox::Slot().AutoHeight()[Label([this,Index] { const auto& D=Field->Presentation().Defenses;return D.IsValidIndex(Index) ? D[Index].Current:FString(); },11)]
             + SVerticalBox::Slot().AutoHeight()[Label([this,Index] { const auto& D=Field->Presentation().Defenses;return D.IsValidIndex(Index) ? D[Index].Last:FString(); },11,Gold)]]
          + SVerticalBox::Slot().AutoHeight()[SNew(SBox)
             .Visibility_Lambda([this,Index] { return GoalDutyStatus(Index).IsEmpty() ? EVisibility::Collapsed:EVisibility::HitTestInvisible; })
             [Label([this,Index] { return GoalDutyStatus(Index); },11,FLinearColor(.68f,.86f,.79f))]]]];
    for (int Index = 0; Index < 6; ++Index)
    {
        auto Contents=SNew(SVerticalBox).Visibility(EVisibility::HitTestInvisible)
          + SVerticalBox::Slot().AutoHeight()
          [ SNew(SBorder).Padding(FMargin(4, 2)).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor_Lambda([this, Index] { return Field->VisualUnits().IsValidIndex(Index) && Field->VisualUnits()[Index].bEnemy ? FLinearColor(.14f, .04f, .025f, .96f) : FLinearColor(.025f, .065f, .065f, .96f); })
            [SNew(SHorizontalBox)
              + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).Visibility(Index < 3 ? EVisibility::HitTestInvisible : EVisibility::Collapsed)[PortraitWidget([Index] { return Index; }, 28, 32)]]
              + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(28).HeightOverride(28)
                .Visibility_Lambda([this,Index] { const auto& U=Field->VisualUnits();return U.IsValidIndex(Index) &&
                    HistoricalPortraitBrush(U[Index].HistoricalSpeaker) ? EVisibility::HitTestInvisible:EVisibility::Collapsed; })
                [SNew(SImage).Image_Lambda([this,Index] { const auto& U=Field->VisualUnits();return U.IsValidIndex(Index) ?
                    HistoricalPortraitBrush(U[Index].HistoricalSpeaker):nullptr; })]]
              + SHorizontalBox::Slot().FillWidth(1).Padding(3, 0)
                [SNew(SVerticalBox)
                 + SVerticalBox::Slot().AutoHeight()[Label([this, Index] { return Field->VisualUnits().IsValidIndex(Index) ? Field->VisualUnits()[Index].Name.Replace(TEXT("の"), TEXT("")) : FString(); }, 13, Gold)]
                 + SVerticalBox::Slot().AutoHeight()[Label([this, Index] { return Field->VisualUnits().IsValidIndex(Index) ? FString::Printf(TEXT("%s %d"), Index == 5 ? TEXT("護衛") : TEXT("兵"), Field->VisualUnits()[Index].Troops) : FString(); }, 11)] ] ] ]
          + SVerticalBox::Slot().AutoHeight()[SNew(SBox).HeightOverride(3)
             [SNew(SProgressBar).Style(&ProgressStyle()).Percent_Lambda([this,Index] { const auto& U=Field->VisualUnits();return U.IsValidIndex(Index) ? FMath::Clamp(U[Index].Morale/100.f,0.f,1.f):0.f; }).FillColorAndOpacity_Lambda([this,Index] { const auto& U=Field->VisualUnits();return FSlateColor(U.IsValidIndex(Index) && U[Index].bEnemy ? FLinearColor(.9f,.35f,.18f) : FLinearColor(.31f,.71f,.62f)); })]]
          + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular",11))
             .Text_Lambda([this,Index] { return FText::FromString(UnitChainNumber(Index)+UnitMapStatus(Index)); })
             .ColorAndOpacity_Lambda([this,Index] { if(const auto* Read=gunyutargetui::Find(Field->Presentation(),Index)) return FSlateColor(gunyutargetui::Color(*Read));return FSlateColor(Field->IsPlaying() ? Field->ReadUnitActivity(Index).Color:FLinearColor(.75f,.91f,.75f)); })]
          + SVerticalBox::Slot().AutoHeight()[SNew(SBox).HeightOverride(2)
             .Visibility_Lambda([this,Index] { return Field->IsPlaying() && Field->ReadUnitActivity(Index).bActive ? EVisibility::HitTestInvisible:EVisibility::Collapsed; })
             [SNew(SProgressBar).Style(&ProgressStyle()).Percent_Lambda([this,Index] { return Field->ReadUnitActivity(Index).Progress; })
              .FillColorAndOpacity_Lambda([this,Index] { return FSlateColor(Field->ReadUnitActivity(Index).Color); })]]
          + SVerticalBox::Slot().AutoHeight()[Label([this, Index] { return Field->VisualUnits().IsValidIndex(Index) && Field->VisualUnits()[Index].Loss > 0 ? FString::Printf(TEXT("−%d"), Field->VisualUnits()[Index].Loss) : FString(); }, 21, FLinearColor(1.f, .35f, .15f))];
        auto Plate=SNew(SBorder).Padding(1).BorderImage_Lambda([this,Index]
            { const auto* Read=gunyutargetui::Find(Field->Presentation(),Index);return FCoreStyle::Get().GetBrush(Read && Read->bSelected ? "WhiteBrush":"NoBrush"); })
            .BorderBackgroundColor_Lambda([this,Index] { const auto* Read=gunyutargetui::Find(Field->Presentation(),Index);return Read ? gunyutargetui::Color(*Read):FLinearColor::Transparent; })
            .ColorAndOpacity_Lambda([this,Index]
            {
                const auto A=Field->ReadActionFocus();
                const bool Passive=bTacticCinema && Field->ReadTacticCinema().bVisible && A.bActive && A.bHasTarget && Index!=A.Actor && Index!=A.Target;
                return FLinearColor(1,1,1,Passive ? 1-FMath::SmoothStep(0.f,.20f,A.Weight):1.f);
            })
            .ToolTipText_Lambda([this,Index] { const auto* Read=gunyutargetui::Find(Field->Presentation(),Index);return FText::FromString(Read ? Field->Presentation().SelectedTacticName+TEXT("\n")+Read->Detail:FString()); })
            .Cursor(EMouseCursor::Hand)
            .Visibility_Lambda([this,Index]
            { const auto& U=Field->VisualUnits();return U.IsValidIndex(Index) && U[Index].Id>=0 && (U[Index].Troops>0 || Field->IsPlaying()) && !Field->Presentation().Journey.bVisible ? EVisibility::Visible : EVisibility::Collapsed; })
            .OnMouseButtonDown_Lambda([this,Index](const FGeometry&,const FPointerEvent& Event)
            {
                if(Event.IsMouseButtonDown(EKeys::MiddleMouseButton)) return FReply::Handled();
                if(Event.GetEffectingButton()==EKeys::LeftMouseButton) { SelectMapUnit(Index);return FReply::Handled(); }
                const auto& U=Field->VisualUnits();
                if(Event.GetEffectingButton()==EKeys::RightMouseButton && Field->bRealtimeCommand && U.IsValidIndex(Index) && U[Index].Id>=0 && U[Index].bEnemy && U[Index].Troops>0)
                {
                    const auto Cell=hexgrid::Pick(U[Index].Position.X/200.,U[Index].Position.Y/200.);
                    Field->CommandAt(Cell.first+Cell.second*world::Width);IgnoreMapInputThroughFrame=Frames+1;return FReply::Handled();
                }
                return FReply::Unhandled();
            })[SNew(SBorder).Padding(0).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(FLinearColor(.012f,.024f,.026f,.96f))[Contents]];
        InputPanels.Add(Plate);Canvas->AddSlot().Position_Lambda([this,Index] { return LabelPosition(Index); }).Size(FVector2D(132,86))[Plate];
    }
}
