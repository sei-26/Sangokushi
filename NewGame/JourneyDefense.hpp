#pragma once
namespace frontline
{
    class StrategistJourney;
    enum class JourneyDefenseReason { Unchecked, Secured, Vacant, Depleted, NeedsGuard, GuardSpent, GuardWeak, Overwhelmed };
    struct JourneyDefenseRead
    {
        int goal=-1,guard=-1,turn=0,threats=0;
        bool secured=false,guarded=false,attackSpent=false,moved=false;
        bool canGuard=false; // Native/manual Hold eligibility; realtime automatic Hold also waits for movement to recover.
        JourneyDefenseReason reason=JourneyDefenseReason::Unchecked;
    };
    // Current friendly facts and current player contacts only; immutable estimate, never a promise of the next update.
    JourneyDefenseRead ReadJourneyDefense(const StrategistJourney&,int goal);
    // Latest completed local check, before action recovery. Display-only, never saved or treated as battle history.
    JourneyDefenseRead ReadJourneyDefenseCheck(const StrategistJourney&,int goal);
}
