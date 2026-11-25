#pragma once
#include "StrategistJourney.hpp"
#include "CoreMinimal.h"

namespace gunyucommand
{
    // Only settled mission facts and a current public contact. This limits
    // automatic suggestions, never the player's manual tactics or selection.
    inline FString SettledTargetReason(const frontline::StrategistJourney& J,int Target)
    {
        const auto* Seen=J.intelligence[0].Find(Target);
        if(!Seen || !Seen->current || Seen->observed.troops<=0 || Seen->observed.morale>25) return {};
        if(J.HasGrandStrategy() && Target==8 && J.battleReview.grand.relievedOn)
            return TEXT("北陣を退ける役目は達成済み。追撃は戦法から自由に指示できる。");
        if(J.HasYangrenStrategy() && Target==8 && J.battleReview.grand.yangren.repelledOn)
            return TEXT("騎兵反撃は撃退済み。追撃は戦法から自由に指示できる。");
        return {};
    }
}
