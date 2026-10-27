#pragma once
#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "Fonts/SlateFontInfo.h"
#include "Fonts/CompositeFont.h"
#include "Styling/CoreStyle.h"
namespace gunyustyle
{
    inline const FLinearColor Paper(.89f,.86f,.78f),Brass(.70f,.55f,.33f),Vermilion(.46f,.105f,.065f);
    inline FSlateFontInfo Font(int Size,bool Literary=false)
    {
        if(!Literary) return FCoreStyle::GetDefaultFontStyle("Regular",Size);
        static const TSharedPtr<const FCompositeFont> Typeface=MakeShared<FCompositeFont>(FName(TEXT("Regular")),
            FPaths::ProjectContentDir()/TEXT("UI/Fonts/BIZUDMincho-Regular.ttf"),EFontHinting::Default,EFontLoadingPolicy::LazyLoad);
        return FSlateFontInfo(Typeface,float(Size));
    }
}
