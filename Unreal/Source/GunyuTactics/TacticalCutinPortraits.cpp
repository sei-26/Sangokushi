#include "TacticalTrial.h"
#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
void ATacticalController::LoadCutinPortraits()
{
    CutinTextures.Reset();CutinBrushes.Reset();
    for(const TCHAR* Name:{TEXT("T_LiuCutinV1"),TEXT("T_GuanCutinV1"),TEXT("T_ZhangCutinV1")})
    {
        const FString Path=FString(TEXT("/Game/Art/Officers/"))+Name+TEXT(".")+Name;
        auto* Texture=LoadObject<UTexture2D>(nullptr,*Path);CutinTextures.Add(Texture);
        auto Brush=MakeShared<FSlateBrush>();Brush->DrawAs=ESlateBrushDrawType::Image;
        Brush->SetResourceObject(Texture);Brush->ImageSize=FVector2D(1024,1536);CutinBrushes.Add(Brush);
        if(Texture) Texture->SetForceMipLevelsToBeResident(3600.f);
    }
}
TSharedRef<SWidget> ATacticalController::CutinPortraitWidget(TFunction<int()> Id,float Width,float Height)
{
    return SNew(SBox).WidthOverride(Width).HeightOverride(Height)
        [SNew(SScaleBox).Stretch(EStretch::ScaleToFit).Clipping(EWidgetClipping::ClipToBounds)
         [SNew(SImage).Image_Lambda([this,Id]() -> const FSlateBrush*
         {
             const int I=Id();
             if(CutinTextures.IsValidIndex(I) && CutinTextures[I] && CutinBrushes.IsValidIndex(I)) return CutinBrushes[I].Get();
             return PortraitBrushes.IsValidIndex(I) ? PortraitBrushes[I].Get() : FCoreStyle::Get().GetBrush("NoBrush");
         })]];
}
