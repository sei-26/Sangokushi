#pragma once
#include "StrategistJourney.hpp"

namespace frontline
{
    enum class JourneyDirective { None, March, Attack, Hold, Ambush };
    enum class JourneyArrival { Wait, Hold, Ambush };
    struct JourneyStandingOrder
    {
        JourneyDirective kind = JourneyDirective::None;
        int destination = -1, target = -1;
        JourneyArrival arrival=JourneyArrival::Wait;
        std::vector<int> waypoints;
    };
    inline bool SameJourneyOrder(const JourneyStandingOrder& a,const JourneyStandingOrder& b)
    { return a.kind==b.kind && a.destination==b.destination && a.target==b.target && a.arrival==b.arrival && a.waypoints==b.waypoints; }
    struct JourneyMarchPlan
    {
        std::vector<int> route,points;
        bool complete=true;
    };
    // Engine-independent command clock and persistent orders. A pulse resolves
    // both sides from the same starting observation, preserving story outcomes.
    class JourneyRealtime
    {
    public:
        static constexpr double Period = 12.;
        static constexpr int MaxMarchPoints=4;
        bool paused = true;
        double speed = 1., elapsed = 0., activeSeconds = 0.;
        int pulses = 0;
        std::array<JourneyStandingOrder,3> orders{};
        void Reset();
        void ApplyAssignments(const StrategistJourney&);
        void Advance(double seconds);
        bool Due() const { return elapsed + 1e-7 >= Period; }
        void Consume(bool singleStep = false);
        bool Queue(const StrategistJourney& journey, int army, JourneyDirective kind, int value = -1);
        void Cancel(int army);
        bool AppendWaypoint(const StrategistJourney&, int army, int tile);
        bool UndoWaypoint(int army);
        bool SetArrival(const StrategistJourney&, int army, JourneyArrival);
        JourneyMarchPlan MarchPlan(const StrategistJourney&, int army) const;
        std::vector<int> Route(const StrategistJourney& journey, int army) const;
        bool Resolve(StrategistJourney& journey, const std::array<bool,3>& reservedAttacks = {});
    private:
        static bool Ground(const Campaign&,int tile);
        void AdvanceMarch(const Campaign&,int army);
    };
}
