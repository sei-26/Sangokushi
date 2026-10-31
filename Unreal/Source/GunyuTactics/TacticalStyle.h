#pragma once
#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "Fonts/SlateFontInfo.h"
#include "Fonts/CompositeFont.h"
#include "Styling/CoreStyle.h"
namespace gunyustyle
{
    inline const FLinearColor Paper(.98f,.94f,.83f),Brass(.96f,.73f,.35f),Vermilion(.72f,.15f,.12f);
    inline FSlateFontInfo Font(int Size,bool Literary=false)
    {
        if(!Literary) return FCoreStyle::GetDefaultFontStyle("Regular",Size);
        static const TSharedPtr<const FCompositeFont> Typeface=MakeShared<FCompositeFont>(FName(TEXT("Regular")),
            FPaths::ProjectContentDir()/TEXT("UI/Fonts/BIZUDMincho-Regular.ttf"),EFontHinting::Default,EFontLoadingPolicy::LazyLoad);
        return FSlateFontInfo(Typeface,float(Size));
    }
}
