#pragma once
#include "TacticalPresentation.h"
namespace gunyufocus
{
    struct FClear { FVector Center;float Radius; };
    struct FRead
    {
        int Actor=-1,Target=-1;
        FVector Hero=FVector::ZeroVector,Blade=Hero,Victim=Hero,Center=Hero;
        float Weight=0;
        bool bActive=false,bHasTarget=false;
        TArray<FClear> Clearings;
    };
    FRead Read(const TArray<FVisualUnit>&,const FBattleBeat*,float Progress,float Time,TFunctionRef<float(FVector)> Height);
    float ClothOpacity(FVector Point,FVector Camera,const FRead&);
    bool SmokeTest();
}
