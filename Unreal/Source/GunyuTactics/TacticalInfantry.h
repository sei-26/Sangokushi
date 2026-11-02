#pragma once
#include "TacticalSoldierMotion.h"
#include "TacticalCompanyStyle.h"
namespace gunyuinfantry
{
    struct FBatches
    {
        TArray<FTransform> Cloth,Iron,Skin,Poles,Armour,Helmets,Shields,Limbs,Spears,Swords,Glaives,Serpents;
        TArray<FTransform> Heads,Beards,Capes;
    };
    void Soldier(FBatches&,const gunyumotion::FPose&,FVector Base,FQuat Heading,float Scale,float Height,const gunyucompany::FEquipment& = {});
}
