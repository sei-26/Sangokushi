#pragma once
#include "CoreMinimal.h"

namespace TacticalMap
{
    struct FBattlefieldStyle
    {
        FLinearColor Ground, Forest, Rock, Road;
        float RidgeHeight, RidgeFalloff, TreeScale;
    };
    inline FBattlefieldStyle BattlefieldStyle(int Mission)
    {
        if (Mission == 1)
            return {FLinearColor(.21f,.205f,.115f),FLinearColor(.13f,.18f,.065f),
                FLinearColor(.30f,.27f,.19f),FLinearColor(.33f,.255f,.14f),145.f,24000.f,.90f};
        if (Mission == 2)
            return {FLinearColor(.145f,.17f,.18f),FLinearColor(.075f,.13f,.12f),
                FLinearColor(.30f,.32f,.34f),FLinearColor(.245f,.23f,.19f),260.f,48000.f,.78f};
        return {FLinearColor(.14f,.175f,.105f),FLinearColor(.09f,.145f,.042f),
            FLinearColor(.24f,.235f,.18f),FLinearColor(.27f,.205f,.115f),175.f,24000.f,1.f};
    }
}
