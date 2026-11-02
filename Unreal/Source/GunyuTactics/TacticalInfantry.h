#pragma once
#include "TacticalSoldierMotion.h"
#include "TacticalCompanyStyle.h"
namespace gunyuinfantry
{
    struct FBatches
    {
        TArray<FTransform> Cloth,Iron,Skin,Poles,Armour,Helmets,Shields,Limbs,Spears,Swords,Glaives,Serpents;
    };
    void Soldier(FBatches&,const gunyumotion::FPose&,FVector Base,FQuat Heading,float Scale,float Height,const gunyucompany::FEquipment& = {});
}
