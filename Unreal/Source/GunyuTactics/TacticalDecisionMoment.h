#pragma once

#include "CoreMinimal.h"

namespace frontline { class StrategistJourney; }

namespace gunyudecision
{
    enum class EKind { None, EscortBlocked, PortOpening, BeaconLost, GuangzongSplit, SunJianBlocked, DepotIntercept,
        GrandOpening,GrandRelief,GrandRally,GrandCounter,CoalitionReturned };

    struct FMoment
    {
        EKind Kind=EKind::None;
        int Tile=-1;
        const TCHAR* Message=TEXT("");
    };

    struct FSnapshot
    {
        FMoment Moment;
        int HoldCount=0, LostBeacon=-1;
        bool bBattle=false;
    };

    bool CanSeePortOpening(const frontline::StrategistJourney&);
    FSnapshot Capture(const frontline::StrategistJourney&);
    FMoment Compare(const FSnapshot& Before,const FSnapshot& After);
}
