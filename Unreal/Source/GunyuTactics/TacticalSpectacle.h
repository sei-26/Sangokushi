#pragma once
#include "TacticalPresentation.h"
namespace gunyuspectacle
{
    struct FShot
    {
        FVector Center=FVector::ZeroVector;
        float Weight=0,CloseDistance=1750,Tilt=42,Sweep=0,Pulse=0,Shake=0,Bloom=0;
        bool bValid=false;
    };
    FShot Read(const FBattleBeat*,const TArray<FVisualUnit>&,float Progress);
}
