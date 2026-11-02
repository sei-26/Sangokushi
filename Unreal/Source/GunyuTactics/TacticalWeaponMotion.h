#pragma once
#include "TacticalCompanyStyle.h"
namespace gunyuweapon
{
    struct FRibbon { FVector A,B,C,D;FLinearColor Color; };
    // Displayed observations determine whether to attack, salute or remain idle.
    gunyumotion::FPose Pose(const gunyumotion::FCompany&,const gunyucompany::FEquipment&,int Soldier,int UnitId,float Time);
    FVector BladeTip(const gunyumotion::FPose&,const gunyucompany::FEquipment&);
    bool IsAttack(const TArray<FVisualUnit>&,const FBattleBeat*,float Progress);
    TArray<FRibbon> Ribbons(const TArray<FVisualUnit>&,const FBattleBeat*,float Progress,float Time,TFunctionRef<float(FVector)> Height);
    bool SmokeTest();
}
