#pragma once
namespace frontline
{
    enum class YangrenApproach { None, Forest, Advance };
    struct JourneyYangrenBattle
    {
        bool active=false;
        YangrenApproach approach=YangrenApproach::None;
        int orderedOn=0,counterOn=0,repelledOn=0,mainReturned=0;
    };
}
