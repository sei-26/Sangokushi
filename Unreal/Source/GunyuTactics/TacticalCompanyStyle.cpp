#include "TacticalCompanyStyle.h"
namespace gunyucompany
{
    FEquipment Equipment(const FVisualUnit& U,int Soldier,int Count)
    {
        FEquipment E;
        E.bCommander=!U.bEnemy && U.Id>=0 && U.Id<3 && Soldier==3;
        E.bShield=Soldier<FMath::DivideAndRoundUp(Count,2);
        if(E.bCommander)
        {
            E.VisualScale=.94f;
            E.Weapon=U.Id==0 ? EWeapon::Sword:U.Id==1 ? EWeapon::Glaive:EWeapon::Serpent;
            E.bShield=U.Id==0;
        }
        E.ShaftLength=E.bShield ? 92:116;
        return E;
    }
    int RepresentativeCount(const FVisualUnit& U) { return FMath::Clamp(FMath::CeilToInt(U.Troops/35.f),6,48); }
    FLinearColor Livery(const FVisualUnit& U)
    {
        if(U.bEnemy) return FLinearColor(.46f,.23f,.035f);
        if(U.Id==1) return FLinearColor(.025f,.25f,.105f);
        if(U.Id==2) return FLinearColor(.34f,.055f,.035f);
        return FLinearColor(.035f,.16f,.36f);
    }
    gunyumotion::FPose EquippedPose(gunyumotion::FPose P,const FEquipment& E)
    {
        if(!E.bShield)
        {
            P.Hand[0]=P.SpearCenter+P.Spear.RotateVector(FVector(0,0,-24));
            P.Elbow[0]=(P.Shoulder[0]+P.Hand[0])*.5f+P.Torso.RotateVector(FVector(-6,-5,-6));
        }
        return P;
    }
}
