#pragma once
#include <string>
namespace frontline
{
    class StrategistJourney;
    enum class JourneyReliefOutcome { Ordered, Departed, Arrived, Held, Recovered, Lost, Unconfirmed };
    struct JourneyReliefRecord
    {
        int outgoing=-1,incoming=-1,source=-1,destination=-1;
        int issuedTurn=0,departedTurn=0,arrivedTurn=0,checkedTurn=0,gapCycles=0;
        bool realtime=false,allSecured=false;
        JourneyReliefOutcome outcome=JourneyReliefOutcome::Ordered;
    };
    bool ReliefPending(const JourneyReliefRecord&);
    std::u32string ReliefReviewText(const StrategistJourney&);
}
