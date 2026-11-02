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
        float VisualScale=.72f;
    };
    // Costume and representative ranks, never additional troop types or bonuses.
    FEquipment Equipment(const FVisualUnit&,int Soldier,int Count);
    int RepresentativeCount(const FVisualUnit&);
    FLinearColor Livery(const FVisualUnit&);
    gunyumotion::FPose EquippedPose(gunyumotion::FPose,const FEquipment&);
    inline FVector StandardPoint(FVector2D UV) { return FVector(-72+UV.X*54,-12,142-UV.Y*64); }
}
