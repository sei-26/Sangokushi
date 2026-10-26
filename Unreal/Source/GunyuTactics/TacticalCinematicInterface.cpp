#include "TacticalTrial.h"
#include "TacticalInkStroke.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
using namespace gunyuui;
void ATacticalController::BuildTacticCinema(TSharedRef<SCanvas> Canvas)
{
    auto Read=[this] { return Field->ReadTacticCinema(); };
    auto Banner=SNew(SOverlay)
        + SOverlay::Slot()[SNew(STacticalInkStroke).Read(Read)]
        + SOverlay::Slot().Padding(24,20)[SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Chain; },13,Gold)]
            + SVerticalBox::Slot().AutoHeight().Padding(0,5)[Label([Read] { return Read().Officer; },20)]
            + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Bold",34))
                .ColorAndOpacity_Lambda([Read] { return FSlateColor(Read().Accent); }).ShadowOffset(FVector2D(2,3)).ShadowColorAndOpacity(FLinearColor::Black)
                .Text_Lambda([Read] { return FText::FromString(Read().Tactic); })]
            + SVerticalBox::Slot().AutoHeight().Padding(0,7)[Label([Read] { return Read().Voice; },13)]];
    auto Intro=SNew(SOverlay).Visibility_Lambda([this,Read] { return bTacticCinema && Read().bVisible && Read().Intro>.001f ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
        + SOverlay::Slot().HAlign(HAlign_Left)[CutinPortraitWidget([Read] { return Read().Actor; },320,480)]
        + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0,40,0,0)[SNew(SBox).WidthOverride(455).HeightOverride(180)[Banner]];
    Intro->ForceVolatile(true);Intro->SetRenderOpacity(0);CinematicIntro=Intro;
    Canvas->AddSlot().Position_Lambda([Read] { return FVector2D(20-(1-Read().Intro)*55,180); }).Size(FVector2D(740,480))[Intro];
    auto Result=SNew(SOverlay).Visibility_Lambda([this,Read] { return bTacticCinema && Read().bVisible && Read().Impact>.001f && !Read().Primary.IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
        + SOverlay::Slot()[SNew(STacticalInkStroke).Read(Read)]
        + SOverlay::Slot().Padding(18,10)[SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Chain+TEXT("  /  ")+Read().Tactic; },13,Gold)]
            + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Bold",36))
                .ColorAndOpacity_Lambda([Read] { return FSlateColor(Read().Accent); }).ShadowOffset(FVector2D(2,3)).ShadowColorAndOpacity(FLinearColor::Black)
                .Text_Lambda([Read] { return FText::FromString(Read().Primary); })]
            + SVerticalBox::Slot().AutoHeight().Padding(0,3)[Label([Read] { return Read().Secondary; },12)]];
    Result->ForceVolatile(true);Result->SetRenderOpacity(0);CinematicResult=Result;
    Canvas->AddSlot().Position_Lambda([this] { return TacticResultPosition(); }).Size(FVector2D(380,142))[Result];
}
FVector2D ATacticalController::TacticResultPosition() const
{
    const auto R=Field->ReadTacticCinema();const auto& U=Field->VisualUnits();const int Id=U.IsValidIndex(R.Target) && U[R.Target].Id>=0 ? R.Target:R.Actor;
    if(!U.IsValidIndex(Id) || U[Id].Id<0) return FVector2D(-2000,-2000);
    auto P=MapLabelPosition(U[Id].Position+FVector(0,0,280),FVector2D(380,142));
    if(P.X<-1000) return FVector2D(-2000,-2000);
    P.X=FMath::Clamp(P.X,270.,850.);P.Y=FMath::Clamp(P.Y,224.,570.);return P;
}
