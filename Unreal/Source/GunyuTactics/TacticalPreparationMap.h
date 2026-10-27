#pragma once
#include "CoreMinimal.h"

struct FPlanningCell { int Tile=-1;uint8 Terrain=0; };
struct FPlanningStart { int Tile=-1,Officer=-1; };
struct FPlanningSite
{
    int Tile=-1;
    FString Name,Description;
    TArray<FString> Hints;
};
// Upcoming public geography. There is deliberately no enemy/army-state field.
struct FPreparationMap
{
    FString Name,Objective;
    TArray<FPlanningCell> Cells;
    TArray<FPlanningStart> Starts;
    TArray<FPlanningSite> Sites;
    TArray<int> ConvoyRoute;
    bool bChooseRoute=false,bSouthern=false;
};
