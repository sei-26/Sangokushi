#pragma once
#include "CoreMinimal.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "TacticalChrome.h"
namespace gunyuui
{
    inline const FLinearColor Ink(.86f, .90f, .91f), Gold(.92f, .73f, .39f);
    inline TSharedRef<STextBlock> Label(TFunction<FString()> Value, int Size = 15, FLinearColor Color = Ink)
    {
        return SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular", Size))
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
            .SetNormal(FSlateRoundedBoxBrush(FLinearColor(.012f,.025f,.030f),2.f,FLinearColor(.12f,.18f,.18f),1.f))
            .SetHovered(FSlateRoundedBoxBrush(FLinearColor(.085f,.13f,.14f),2.f,FLinearColor(.66f,.52f,.27f),1.f))
            .SetPressed(FSlateRoundedBoxBrush(FLinearColor(.015f,.035f,.04f),2.f,FLinearColor(.9f,.7f,.33f),1.f))
            .SetDisabled(FSlateRoundedBoxBrush(FLinearColor(.018f,.024f,.029f),2.f,FLinearColor(.10f,.13f,.14f),1.f));
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
