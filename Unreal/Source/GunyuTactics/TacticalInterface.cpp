#include "TacticalTrial.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Engine/Texture2D.h"
#include "PortraitCatalog.hpp"

#include "TacticalWidgets.h"
using namespace gunyuui;
bool ATacticalController::IsPointerOverInterface() const
{
    if (!bInterfaceVisible) return false;
    for (const auto& Panel : InputPanels) if (Panel.IsValid() && Panel->IsHovered()) return true;
    return false;
}

FVector2D ATacticalController::LabelPosition(int Index) const
{
    if(LabelLayoutFrame!=GFrameCounter) const_cast<ATacticalController*>(this)->LayoutMapLabels();
    return UnitLabelPositions.IsValidIndex(Index) ? UnitLabelPositions[Index] : FVector2D(-2000,-2000);
}
FVector2D ATacticalController::MapLabelPosition(FVector World, FVector2D Size) const
{
    FVector2D Point;
    if (!ProjectWorldLocationToScreen(World, Point)) return FVector2D(-2000, -2000);
    int W, H; GetViewportSize(W, H);
    const float Scale = FMath::Min(W / 1600.f, H / 900.f);
    if (Scale <= 0) return FVector2D(-2000, -2000);
    return (Point - FVector2D((W - 1600 * Scale) * .5, (H - 900 * Scale) * .5)) / Scale - Size * .5;
}

TSharedRef<SWidget> ATacticalController::PortraitWidget(TFunction<int()> Id, float Width, float Height)
{
    return SNew(SBox).WidthOverride(Width).HeightOverride(Height)
        [SNew(SScaleBox).Stretch(EStretch::ScaleToFit).HAlign(HAlign_Center).VAlign(VAlign_Center).Clipping(EWidgetClipping::ClipToBounds)
         [SNew(SImage).Image_Lambda([this, Id, Width]() -> const FSlateBrush* { const auto& Brushes=Width<100 ? FaceBrushes : PortraitBrushes;return Brushes.IsValidIndex(Id()) ? Brushes[Id()].Get() : FCoreStyle::Get().GetBrush("NoBrush"); })]];
}
void ATacticalController::AddBattlePanel(TSharedRef<SCanvas> Canvas,FVector2D Position,FVector2D Size,TSharedRef<SWidget> Content,float Inset)
{
    auto Panel=SNew(STacticalChrome).Padding(Inset)
        .Visibility_Lambda([this] { return Field->Presentation().Journey.bVisible ? EVisibility::Collapsed : EVisibility::Visible; })[Content];
    InputPanels.Add(Panel);Canvas->AddSlot().Position(Position).Size(Size)[Panel];
}
void ATacticalController::BuildInterface()
{
    InputPanels.Reset();
    PortraitSheet = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Art/Officers/T_ShuPortraitSheet.T_ShuPortraitSheet"));
    PortraitBrushes.Reset();FaceBrushes.Reset();
    if(PortraitSheet) PortraitSheet->SetForceMipLevelsToBeResident(3600.f);
    if (PortraitSheet)
        for (int CatalogId : {0, 4, 2})
        {
            auto Brush = MakeShared<FSlateBrush>();
            Brush->SetResourceObject(PortraitSheet); Brush->DrawAs = ESlateBrushDrawType::Image;
            // Source atlas dimensions, not the temporary 32x32 streaming resource.
            constexpr int W=1024,H=1536;
            const auto Crop=campaignvisual::PortraitCrop(CatalogId,W,H);
            Brush->ImageSize=FVector2D(Crop[2],Crop[3]);
            Brush->SetUVRegion(FBox2d(FVector2D(double(Crop[0])/W,double(Crop[1])/H),FVector2D(double(Crop[0]+Crop[2])/W,double(Crop[1]+Crop[3])/H)));
            PortraitBrushes.Add(Brush);
            auto Face=MakeShared<FSlateBrush>(*Brush);
            const double Left=Crop[0]+Crop[2]*.18,Right=Crop[0]+Crop[2]*.82;
            const double Top=Crop[1],Bottom=Crop[1]+Crop[3]*.84;
            Face->ImageSize=FVector2D(Right-Left,Bottom-Top);
            Face->SetUVRegion(FBox2d(FVector2D(Left/W,Top/H),FVector2D(Right/W,Bottom/H)));
            FaceBrushes.Add(Face);
        }
    LoadCutinPortraits();
    LoadNovelArt();
    auto Canvas=SNew(SCanvas).Visibility(EVisibility::SelfHitTestInvisible);
    Canvas->AddSlot().Position(gunyulayout::MapOrigin).Size(gunyulayout::MapSize)[MarchRouteOverlay(true)];
    Canvas->AddSlot().Position(gunyulayout::MapOrigin).Size(gunyulayout::MapSize)[MarchRouteOverlay()];
    Canvas->AddSlot().Position(gunyulayout::MapOrigin).Size(gunyulayout::MapSize)[MapLabelLeaderOverlay()];
    Canvas->AddSlot().Position(gunyulayout::MapOrigin).Size(gunyulayout::MapSize)[ChainPlanOverlay()];
    Canvas->AddSlot().Position(gunyulayout::MapOrigin).Size(gunyulayout::MapSize)[BattleActivityOverlay()];
    BuildMapLabels(Canvas);
    BuildBattleHeader(Canvas);
    BuildCameraInterface(Canvas);
    BuildTacticCinema(Canvas);
    BuildRosterInterface(Canvas);
    BuildCommandInterface(Canvas);
    BuildBattleFeed(Canvas);
    BuildRealtimeInterface(Canvas);
    BuildTacticInterface(Canvas);
    BuildEnemyInterface(Canvas);
    BuildBattleWatchInterface(Canvas);
    BuildMarchInterface(Canvas);
    BuildJourneyInterface(Canvas);
    BuildEncounterInterface(Canvas);
    BuildChainReviewInterface(Canvas);
    BuildEpisodeInterface(Canvas);
    BuildSaveInterface(Canvas);
    Overlay = SNew(SOverlay)
        + SOverlay::Slot()
        [SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
         .Visibility_Lambda([this] { return bInterfaceVisible ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed; })
         [SNew(SBox).WidthOverride(1600).HeightOverride(900)[Canvas]]]
        + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(18)
        [SNew(SBorder).Padding(FMargin(12,6)).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
         .BorderBackgroundColor(FLinearColor(.012f,.022f,.024f,.85f))
         .Visibility_Lambda([this] { return bInterfaceVisible ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
         [Label([] { return TEXT("Tab：指揮画面に戻る"); },13,Gold)]];
    if (GEngine && GEngine->GameViewport) GEngine->GameViewport->AddViewportWidgetContent(Overlay.ToSharedRef());
}
