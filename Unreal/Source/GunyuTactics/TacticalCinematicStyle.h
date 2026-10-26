#pragma once
#include "TacticalPresentation.h"
namespace gunyucinema
{
    bool IsTactic(EBattleCue Kind);
    FString Name(EBattleCue Kind);
    FString Voice(EBattleCue Kind);
    FLinearColor Color(EBattleCue Kind);
    inline bool SupportChanged(const FVisualUnit& A,const FVisualUnit& Z)
    { return Z.Morale>A.Morale || (!A.bInspired && Z.bInspired) || (A.bDisordered && !Z.bDisordered) || (A.bPressured && !Z.bPressured); }
}
