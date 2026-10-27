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
    ConversationTextures.Reset();ConversationBrushes.Reset();
    for(const TCHAR* Name:{TEXT("T_LiuConversationV1"),TEXT("T_GuanConversationV1"),TEXT("T_ZhangConversationV1")})
    {
        auto* Texture=LoadObject<UTexture2D>(nullptr,*(FString(TEXT("/Game/Art/Novel/"))+Name+TEXT(".")+Name));
        if(Texture) Texture->SetForceMipLevelsToBeResident(3600.f);
        auto Brush=MakeShared<FSlateBrush>();Brush->SetResourceObject(Texture);Brush->DrawAs=ESlateBrushDrawType::Image;
        Brush->ImageSize=FVector2D(1024,1536);ConversationTextures.Add(Texture);ConversationBrushes.Add(Brush);
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
    return SNew(SBox).WidthOverride(Width).HeightOverride(Height)
        [SNew(SScaleBox).Stretch(EStretch::ScaleToFit).Clipping(EWidgetClipping::ClipToBounds)
         [SNew(SImage).Image_Lambda([this,Speaker] { return NovelPortraitBrush(Speaker()); })
          .ColorAndOpacity_Lambda([this,Speaker] { return NovelPortraitTone(Speaker()); })]];
}

const FSlateBrush* ATacticalController::NovelPortraitBrush(int Speaker) const
{
    if(Speaker>=0 && Speaker<3 && ConversationTextures.IsValidIndex(Speaker) && ConversationTextures[Speaker]) return ConversationBrushes[Speaker].Get();
    const int Index=Speaker==3 ? 4:Speaker==4 ? 3:Speaker;
    return CutinBrushes.IsValidIndex(Index) ? CutinBrushes[Index].Get():FCoreStyle::Get().GetBrush("NoBrush");
}
FLinearColor ATacticalController::NovelPortraitTone(int Speaker) const
{
    if(!bEpisodeOpen) return FLinearColor::White;
    const auto Level=[](int Who,int S) { return Who==S ? 1.f:.68f; };
    const float T=FMath::Clamp(NovelLineTime/.22f,0.f,1.f);
    const float Light=FMath::Lerp(Level(NovelPreviousSpeaker,Speaker),Level(CurrentEpisodeLine().Speaker,Speaker),T*T*(3-2*T));
    return FLinearColor(Light,Light,Light,FMath::Clamp(NovelStageTime/.3f,0.f,1.f));
}
