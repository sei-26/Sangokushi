#pragma once
#include <array>
#include <string>
namespace frontline
{
    enum class JourneyDuty { None, Hold, Ambush };
    struct JourneyAssignment
    {
        JourneyDuty duty=JourneyDuty::None;
        int tile=-1;
        bool completed=false;
    };
    const char32_t* DutyName(JourneyDuty);
}
