#pragma once
#include "TacticalSoldierMotion.h"
namespace gunyuinfantry
{
    struct FBatches
    {
        TArray<FTransform> Cloth,Iron,Skin,Poles,Armour,Helmets,Shields,Limbs,Spears;
    };
    void Soldier(FBatches&,const gunyumotion::FPose&,FVector Base,FQuat Heading,float Scale,float Height);
}
