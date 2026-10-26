#pragma once
#include "TacticalPresentation.h"
namespace gunyucinema
{
    bool IsTactic(EBattleCue Kind);
    inline int ResultUnit(const FBattleBeat& B,const TArray<FVisualUnit>& U)
    {
        const int Preferred=B.Kind==EBattleCue::Rally || B.Kind==EBattleCue::Stop ? B.Actor:B.Target;
        if(U.IsValidIndex(Preferred) && U[Preferred].Id>=0) return Preferred;
        return U.IsValidIndex(B.Actor) && U[B.Actor].Id>=0 ? B.Actor:-1;
    }
    FString Name(EBattleCue Kind);
    FString Voice(EBattleCue Kind);
    FLinearColor Color(EBattleCue Kind);
    inline bool SupportChanged(const FVisualUnit& A,const FVisualUnit& Z)
    { return Z.Morale>A.Morale || (!A.bInspired && Z.bInspired) || (A.bDisordered && !Z.bDisordered) || (A.bPressured && !Z.bPressured); }
}
