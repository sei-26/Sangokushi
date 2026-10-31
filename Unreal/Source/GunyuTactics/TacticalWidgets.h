#pragma once
#include "CoreMinimal.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "TacticalChrome.h"
#include "TacticalStyle.h"
namespace gunyuui
{
    inline const FLinearColor Ink=gunyustyle::Paper,Gold=gunyustyle::Brass;
    inline TSharedRef<STextBlock> Label(TFunction<FString()> Value, int Size = 15, FLinearColor Color = Ink)
    {
        return SNew(STextBlock).Font(gunyustyle::Font(Size,Size>=21))
            .ColorAndOpacity(Color).AutoWrapText(true)
            .Text_Lambda([Value] { return FText::FromString(Value()); });
    }
    inline const FProgressBarStyle& ProgressStyle()
    {
        static const FProgressBarStyle Style=FProgressBarStyle()
            .SetBackgroundImage(FSlateColorBrush(FLinearColor(.006f,.012f,.016f)))
            .SetFillImage(FSlateColorBrush(FLinearColor::White));
        return Style;
    }
    inline const FButtonStyle& CommandStyle()
    {
        static const FButtonStyle Style=FButtonStyle()
            .SetNormal(FSlateRoundedBoxBrush(FLinearColor(.105f,.037f,.044f),7.f,FLinearColor(.52f,.32f,.13f),1.f))
            .SetHovered(FSlateRoundedBoxBrush(FLinearColor(.27f,.075f,.065f),7.f,FLinearColor(.98f,.73f,.34f),1.5f))
            .SetPressed(FSlateRoundedBoxBrush(FLinearColor(.40f,.10f,.07f),7.f,FLinearColor(1.f,.84f,.49f),1.5f))
            .SetDisabled(FSlateRoundedBoxBrush(FLinearColor(.034f,.026f,.032f),7.f,FLinearColor(.14f,.10f,.08f),1.f));
        return Style;
    }
    inline TSharedRef<SButton> Button(TFunction<FString()> Value, TFunction<void()> Click,
        TFunction<bool()> Enabled = [] { return true; }, bool bCompact = false)
    {
        return SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(bCompact ? FMargin(6, 2) : FMargin(8, 5))
            .ButtonColorAndOpacity(FLinearColor::White)
            .IsEnabled_Lambda([Enabled] { return Enabled(); })
            .OnClicked_Lambda([Click] { Click(); return FReply::Handled(); })
            [ Label(Value, bCompact ? 13 : 15) ];
    }
}
