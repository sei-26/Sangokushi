#include "TacticalTrial.h"
#include "HexGrid.hpp"
#include "WorldLayout.hpp"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
using namespace gunyuui;
void ATacticalController::BuildMapLabels(TSharedRef<SCanvas> Canvas)
{
    for (int Index = 0; Index < 2; ++Index)
        Canvas->AddSlot().Position_Lambda([this, Index]
        {
            const auto& H = Field->Presentation();
            const int Tile = Index == 0 ? H.GoalTile : H.SecondGoalTile;
            return !H.bBattle || H.Journey.bVisible || Tile < 0 ? FVector2D(-2000, -2000) : MapLabelPosition(Field->SurfaceLocation(Tile) + FVector(0, 0, 240), FVector2D(140, 34));
        }).Size(FVector2D(140, 34))
        [SNew(SBorder).Visibility(EVisibility::HitTestInvisible).Padding(FMargin(8, 4))
         .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.07f, .055f, .02f, .94f))
         [Label([this] { return TEXT("◆ ") + Field->Presentation().GoalLabel; }, 13, Gold)]];
    for (int Index = 0; Index < 6; ++Index)
    {
        auto Contents=SNew(SVerticalBox).Visibility(EVisibility::HitTestInvisible)
          + SVerticalBox::Slot().AutoHeight()
          [ SNew(SBorder).Padding(FMargin(4, 2)).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor_Lambda([this, Index] { return Field->VisualUnits().IsValidIndex(Index) && Field->VisualUnits()[Index].bEnemy ? FLinearColor(.14f, .04f, .025f, .96f) : FLinearColor(.025f, .065f, .065f, .96f); })
            [SNew(SHorizontalBox)
              + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).Visibility(Index < 3 ? EVisibility::HitTestInvisible : EVisibility::Collapsed)[PortraitWidget([Index] { return Index; }, 28, 32)]]
              + SHorizontalBox::Slot().FillWidth(1).Padding(3, 0)
                [SNew(SVerticalBox)
                 + SVerticalBox::Slot().AutoHeight()[Label([this, Index] { return Field->VisualUnits().IsValidIndex(Index) ? Field->VisualUnits()[Index].Name.Replace(TEXT("の"), TEXT("")) : FString(); }, 13, Gold)]
                 + SVerticalBox::Slot().AutoHeight()[Label([this, Index] { return Field->VisualUnits().IsValidIndex(Index) ? FString::Printf(TEXT("%s %d"), Index == 5 ? TEXT("護衛") : TEXT("兵"), Field->VisualUnits()[Index].Troops) : FString(); }, 11)] ] ] ]
          + SVerticalBox::Slot().AutoHeight()[SNew(SBox).HeightOverride(3)
             [SNew(SProgressBar).Style(&ProgressStyle()).Percent_Lambda([this,Index] { const auto& U=Field->VisualUnits();return U.IsValidIndex(Index) ? FMath::Clamp(U[Index].Morale/100.f,0.f,1.f):0.f; }).FillColorAndOpacity_Lambda([this,Index] { const auto& U=Field->VisualUnits();return FSlateColor(U.IsValidIndex(Index) && U[Index].bEnemy ? FLinearColor(.9f,.35f,.18f) : FLinearColor(.31f,.71f,.62f)); })]]
          + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular",11))
             .Text_Lambda([this,Index] { return FText::FromString(UnitChainNumber(Index)+UnitMapStatus(Index)); })
             .ColorAndOpacity_Lambda([this,Index] { return FSlateColor(Field->IsPlaying() ? Field->ReadUnitActivity(Index).Color:FLinearColor(.75f,.91f,.75f)); })]
          + SVerticalBox::Slot().AutoHeight()[SNew(SBox).HeightOverride(2)
             .Visibility_Lambda([this,Index] { return Field->IsPlaying() && Field->ReadUnitActivity(Index).bActive ? EVisibility::HitTestInvisible:EVisibility::Collapsed; })
             [SNew(SProgressBar).Style(&ProgressStyle()).Percent_Lambda([this,Index] { return Field->ReadUnitActivity(Index).Progress; })
              .FillColorAndOpacity_Lambda([this,Index] { return FSlateColor(Field->ReadUnitActivity(Index).Color); })]]
          + SVerticalBox::Slot().AutoHeight()[Label([this, Index] { return Field->VisualUnits().IsValidIndex(Index) && Field->VisualUnits()[Index].Loss > 0 ? FString::Printf(TEXT("−%d"), Field->VisualUnits()[Index].Loss) : FString(); }, 21, FLinearColor(1.f, .35f, .15f))];
        auto Plate=SNew(SBorder).Padding(0).BorderImage(FCoreStyle::Get().GetBrush("NoBrush")).Cursor(EMouseCursor::Hand)
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
            })[Contents];
        InputPanels.Add(Plate);Canvas->AddSlot().Position_Lambda([this,Index] { return LabelPosition(Index); }).Size(FVector2D(132,86))[Plate];
    }
}
