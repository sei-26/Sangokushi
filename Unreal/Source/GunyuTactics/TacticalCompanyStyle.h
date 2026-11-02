#pragma once
#include "TacticalSoldierMotion.h"
namespace gunyucompany
{
    enum class EWeapon { Spear,Sword,Glaive,Serpent };
    struct FEquipment
    {
        EWeapon Weapon=EWeapon::Spear;
        bool bShield=true,bCommander=false;
        float ShaftLength=92;
    };
    // Costume and representative ranks, never additional troop types or bonuses.
    FEquipment Equipment(const FVisualUnit&,int Soldier,int Count);
    FLinearColor Livery(const FVisualUnit&);
    gunyumotion::FPose EquippedPose(gunyumotion::FPose,const FEquipment&);
}
