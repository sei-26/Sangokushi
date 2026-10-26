#pragma once
#include "JourneyRealtime.hpp"

namespace frontline
{
    enum class JourneyRouteStatus { Idle, Moving, Engaging, NoRoute, LostTarget, Guarding, Ambushing };
    struct JourneyOrderRead
    {
        JourneyRouteStatus status=JourneyRouteStatus::Idle;
        std::vector<int> route;
        bool complete=true;
    };
    JourneyOrderRead ReadJourneyOrder(const StrategistJourney&, const JourneyRealtime&, int army);

    struct JourneyWatchUnit
    {
        int id=-1,tile=-1,troops=0,morale=0,cargo=0;
        std::u32string name;
    };
    struct JourneyWatchSnapshot
    {
        int turn=0;
        bool battle=false;
        std::array<JourneyWatchUnit,6> units{};
        std::array<JourneyStandingOrder,3> orders{};
        std::array<JourneyOrderRead,3> paths{};
        std::array<bool,3> moved{};
    };
    enum class JourneyNoticeKind { Contact, Critical, Defeated, ConvoyHit, Stalled, TargetLost };
    struct JourneyBattleNotice
    {
        JourneyNoticeKind kind=JourneyNoticeKind::Contact;
        int army=-1,tile=-1,turn=0;
        std::u32string name,text;
    };
    // Observed snapshots are the only input to reporting. The monitor never
    // queries hidden enemies, changes combat, or decides whether time is paused.
    class JourneyBattleWatch
    {
    public:
        static constexpr int HistoryLimit=8;
        static JourneyWatchSnapshot Capture(const StrategistJourney&, const JourneyRealtime&);
        void Reset() { history.clear();stalled={};tracking={};lastOrders={}; }
        std::vector<JourneyBattleNotice> Record(const JourneyWatchSnapshot& before, const JourneyWatchSnapshot& after);
        const std::vector<JourneyBattleNotice>& History() const { return history; }
    private:
        friend struct JourneyArchive;
        std::array<int,3> stalled{};
        std::array<bool,3> tracking{};
        std::array<JourneyStandingOrder,3> lastOrders{};
        std::vector<JourneyBattleNotice> history; // Newest first; bounded per battle.
    };
}
