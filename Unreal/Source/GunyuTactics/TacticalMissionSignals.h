#pragma once
#include "CoreMinimal.h"
#include <array>

struct FBattleBeat;
struct FVisualUnit;

namespace gunyumission
{
    enum class ESignal { None, CartAdvance, RoadOpened, WingHeld, WingLost };

    struct FSignal
    {
        ESignal Kind=ESignal::None;
        int Tile=-1,Actor=-1,Count=0,Total=0;
        FVector Origin=FVector::ZeroVector,Position=FVector::ZeroVector;
        float Weight=0,Phase=0;
    };

    // The cart and each wing have their own result and lifetime.
    struct FSignals
    {
        FSignal Cart;
        std::array<FSignal,2> Wings{};
    };

    inline constexpr float SignalLifetime=3.4f;
    FSignals ReadSignals(const FBattleBeat*,const TArray<FVisualUnit>& Displayed,float Progress);
    FLinearColor SignalColor(ESignal);
}
