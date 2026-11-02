#pragma once
#include "TacticalPresentation.h"
namespace gunyuatmosphere
{
    struct FCloud { FVector Position,Scale;float Lift=0; };
    // Displayed motion/current published casualties only; no world or orders.
    TArray<FCloud> GroundDust(const TArray<FVisualUnit>&,const FBattleBeat*,float Progress);
}
