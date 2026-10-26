#pragma once
#include "TacticalFormation.h"

namespace gunyumotion
{
    struct FCompany
    {
        FVector Facing=FVector::ForwardVector;
        gunyuformation::EShape Before=gunyuformation::EShape::Line,Rest=Before,Action=Before;
        float Progress=0,StateBlend=1,FormationBlend=0,March=0,Recoil=0,Brace=0,Salute=0,Stature=1;
        bool bAttack=false;
    };
    struct FPose
    {
        FVector Root=FVector::ZeroVector;
        FVector Hip[2],Knee[2],Foot[2],Shoulder[2],Elbow[2],Hand[2];
        FQuat Torso=FQuat::Identity,Spear=FQuat::Identity,Shield=FQuat::Identity;
        FVector SpearCenter,ShieldCenter;
    };
    // Displayed observations and the current beat only. Never reads the simulation.
    FCompany Read(const FVisualUnit&,const TArray<FVisualUnit>&,const FBattleBeat*,float Progress);
    FVector FormationOffset(const FCompany&,int Soldier,int Count);
    FPose Pose(const FCompany&,int Soldier,int UnitId,float VisualTime);
}
