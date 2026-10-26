#pragma once
#include "TacticalPresentation.h"
namespace gunyuchainui
{
    inline FString Status(const FOrderPresentation& Order)
    { return !Order.bEvaluated ? TEXT("先行待ち") : Order.bSuccess ? TEXT("成立見込") : TEXT("ここで停止"); }
    inline FLinearColor Color(const FOrderPresentation& Order)
    { return !Order.bEvaluated ? FLinearColor(.50f,.57f,.61f) : Order.bSuccess ? FLinearColor(.38f,.83f,.66f) : FLinearColor(1.f,.48f,.25f); }
    inline FString Number(int Index)
    { static const TCHAR* Symbols[]={TEXT("①"),TEXT("②"),TEXT("③")};return Index>=0 && Index<3 ? Symbols[Index] : TEXT(""); }
}
