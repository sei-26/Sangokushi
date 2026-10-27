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
            .SetNormal(FSlateRoundedBoxBrush(FLinearColor(.020f,.025f,.027f),3.f,FLinearColor(.16f,.15f,.12f),1.f))
            .SetHovered(FSlateRoundedBoxBrush(FLinearColor(.090f,.055f,.040f),3.f,FLinearColor(.55f,.36f,.20f),1.f))
            .SetPressed(FSlateRoundedBoxBrush(FLinearColor(.13f,.042f,.028f),3.f,FLinearColor(.70f,.49f,.25f),1.f))
            .SetDisabled(FSlateRoundedBoxBrush(FLinearColor(.017f,.020f,.022f),3.f,FLinearColor(.085f,.09f,.085f),1.f));
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
