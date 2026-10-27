#include "TacticalTrial.h"
#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SBox.h"
void ATacticalController::LoadNovelArt()
{
    NovelBackgrounds.Reset();NovelBackgroundBrushes.Reset();
    for(const TCHAR* Name:{TEXT("T_CommandTentV1"),TEXT("T_NightCampV1"),TEXT("T_JiAnStandingV1"),TEXT("T_StrategistStandingV1")})
    {
        auto* Texture=LoadObject<UTexture2D>(nullptr,*(FString(TEXT("/Game/Art/Novel/"))+Name+TEXT(".")+Name));
        if(Texture) Texture->SetForceMipLevelsToBeResident(3600.f);
        auto Brush=MakeShared<FSlateBrush>();Brush->SetResourceObject(Texture);Brush->DrawAs=ESlateBrushDrawType::Image;
        Brush->ImageSize=Texture ? FVector2D(Texture->GetSizeX(),Texture->GetSizeY()):FVector2D(1600,900);
        if(NovelBackgrounds.Num()<2) { NovelBackgrounds.Add(Texture);NovelBackgroundBrushes.Add(Brush); }
        else { CutinTextures.Add(Texture);CutinBrushes.Add(Brush); }
    }
}
TSharedRef<SWidget> ATacticalController::NovelBackdropWidget(TFunction<bool()> Night)
{
    return SNew(SScaleBox).Stretch(EStretch::ScaleToFill).Clipping(EWidgetClipping::ClipToBounds)
        [SNew(SImage).Image_Lambda([this,Night]() -> const FSlateBrush*
        { const int I=Night() ? 1:0;return NovelBackgroundBrushes.IsValidIndex(I) ? NovelBackgroundBrushes[I].Get():FCoreStyle::Get().GetBrush("NoBrush"); })];
}
TSharedRef<SWidget> ATacticalController::NovelPortraitWidget(TFunction<int()> Speaker,float Width,float Height)
{
    return CutinPortraitWidget([Speaker] { const int S=Speaker();return S==3 ? 4:S==4 ? 3:S; },Width,Height);
}
