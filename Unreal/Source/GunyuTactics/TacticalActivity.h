#pragma once
#include "TacticalPresentation.h"
struct FUnitActivity
{
    FString Caption;
    FLinearColor Color=FLinearColor(.75f,.91f,.75f);
    FVector Direction=FVector::ForwardVector;
    float Progress=0;
    int Partner=-1;
    bool bActive=false,bActor=false,bImpact=false,bStopped=false;
};
namespace gunyuactivity
{
    struct FMark { TArray<FVector> Points;FLinearColor Color; };
    FUnitActivity Read(int Id,const TArray<FVisualUnit>&,const FBattleBeat*,float Progress);
    TArray<FMark> Markers(const TArray<FVisualUnit>&,const FBattleBeat*,float Progress);
}
