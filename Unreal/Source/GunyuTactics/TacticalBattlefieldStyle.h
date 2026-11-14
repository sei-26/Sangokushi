#pragma once
#include "CoreMinimal.h"
#include "JourneyBattlefield.hpp"

namespace TacticalMap
{
    struct FBattlefieldStyle
    {
        FLinearColor Ground, Forest, Rock, Road;
        float RidgeHeight, RidgeFalloff, TreeScale;
        float RoadHalfWidth=58.f;
    };
    inline FBattlefieldStyle BattlefieldStyle(frontline::JourneyMap Map)
    {
        using frontline::JourneyMap;
        if(Map==JourneyMap::LuoyangGate)
            return {FLinearColor(.24f,.205f,.16f),FLinearColor(.115f,.14f,.077f),
                FLinearColor(.34f,.28f,.25f),FLinearColor(.46f,.36f,.26f),95.f,29000.f,.74f,96.f};
        if(Map==JourneyMap::LuoyangDepot)
            return {FLinearColor(.185f,.205f,.135f),FLinearColor(.09f,.15f,.065f),
                FLinearColor(.30f,.28f,.21f),FLinearColor(.54f,.34f,.17f),195.f,31000.f,1.06f,66.f};
        if(Map==JourneyMap::Yangren)
            return {FLinearColor(.25f,.22f,.13f),FLinearColor(.11f,.16f,.075f),
                FLinearColor(.34f,.27f,.21f),FLinearColor(.55f,.32f,.13f),265.f,38000.f,1.05f,75.f};
        if (Map == JourneyMap::Port)
            return {FLinearColor(.21f,.205f,.115f),FLinearColor(.13f,.18f,.065f),
                FLinearColor(.30f,.27f,.19f),FLinearColor(.33f,.255f,.14f),145.f,24000.f,.90f};
        if (Map == JourneyMap::Valley)
            return {FLinearColor(.145f,.17f,.18f),FLinearColor(.075f,.13f,.12f),
                FLinearColor(.30f,.32f,.34f),FLinearColor(.245f,.23f,.19f),260.f,48000.f,.78f};
        if (Map == JourneyMap::PortNorth)
            return {FLinearColor(.16f,.205f,.16f),FLinearColor(.10f,.18f,.115f),
                FLinearColor(.26f,.31f,.32f),FLinearColor(.49f,.31f,.17f),205.f,31000.f,.98f,74.f};
        if (Map == JourneyMap::RefugeeRoad || Map == JourneyMap::CoalitionRoad)
            return {FLinearColor(.245f,.224f,.128f),FLinearColor(.14f,.19f,.071f),
                FLinearColor(.37f,.31f,.22f),FLinearColor(.53f,.32f,.15f),130.f,27000.f,.83f,87.f};
        if (Map == JourneyMap::RefugeeWood)
            return {FLinearColor(.145f,.185f,.12f),FLinearColor(.075f,.15f,.065f),
                FLinearColor(.24f,.28f,.25f),FLinearColor(.47f,.30f,.14f),230.f,34000.f,1.15f,71.f};
        if (Map == JourneyMap::Guangzong)
            return {FLinearColor(.263f,.222f,.134f),FLinearColor(.13f,.175f,.07f),
                FLinearColor(.37f,.29f,.20f),FLinearColor(.51f,.31f,.15f),115.f,29000.f,.82f,76.f};
        if (Map == JourneyMap::CoalitionFord)
            return {FLinearColor(.164f,.19f,.174f),FLinearColor(.089f,.155f,.10f),
                FLinearColor(.30f,.33f,.32f),FLinearColor(.45f,.30f,.17f),200.f,29000.f,.87f,75.f};
        return {FLinearColor(.14f,.175f,.105f),FLinearColor(.09f,.145f,.042f),
            FLinearColor(.24f,.235f,.18f),FLinearColor(.27f,.205f,.115f),175.f,24000.f,1.f};
    }
}
