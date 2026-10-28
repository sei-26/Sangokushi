#pragma once
#include "TacticalPreparationMap.h"
namespace planningguide
{
    FString Place(const FPreparationMap&,int Tile);
    FString Order(const FPreparationMap&,int Officer);
    // Public terrain only; this is not a promise of a safe or unobstructed march.
    TArray<int> Route(const FPreparationMap&,int Officer,int Tile);
}
