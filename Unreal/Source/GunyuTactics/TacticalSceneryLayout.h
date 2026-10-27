#pragma once
#include "CoreMinimal.h"
#include <array>

namespace TacticalMap
{
    enum class EBackdropKind { Village, RiverPort, MountainPass };
    struct FSceneryLayout
    {
        EBackdropKind Kind;
        const TCHAR* ViewName;
        double West,East,Front,Back,Gate;
        float WallHeight;
        FVector VistaFocus;
        float VistaDistance;
        double ValleyCenter,ValleyHalfWidth,ValleyBlendWidth;
        std::array<FVector,5> Peaks;
    };
    inline FSceneryLayout SceneryLayout(int Mission)
    {
        if(Mission==1)
            return {EBackdropKind::RiverPort,TEXT("河港"),2650,3850,-230,-1100,3250,180,
                FVector(2550,900,0),5500,2900,1100,700,
                {{FVector(350,-1400,740),FVector(950,-2400,1050),FVector(4350,-1600,870),FVector(4800,-2800,1100),FVector(2400,-3500,1000)}}};
        if(Mission==2)
            return {EBackdropKind::MountainPass,TEXT("関門"),1100,2500,-220,-820,1800,230,
                FVector(1800,850,0),5300,1800,640,440,
                {{FVector(500,-950,1400),FVector(750,-2000,1850),FVector(3100,-850,1500),FVector(3700,-2000,1900),FVector(1800,-3400,1500)}}};
        return {EBackdropKind::Village,TEXT("集落"),720,2110,-180,-900,1500,60,
            FVector(1400,900,0),5300,1400,1400,850,
            {{FVector(300,-1500,620),FVector(1050,-2600,970),FVector(3100,-1700,720),FVector(4200,-2900,1060),FVector(2200,-3600,1050)}}};
    }
    // Public geography only; the continuation is scenery, outside the order grid.
    inline double PortRiverX(double Y)
    { return 2400+65*(1-FMath::Cos((289.-Y)*.0022)); }
}
