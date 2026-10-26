#include "JourneyArchive.hpp"

namespace frontline
{
    void JourneyArchive::IO(JourneyStandingOrder& v)
    { (*this)(v.kind,v.destination,v.target,v.arrival,v.waypoints); }
    void JourneyArchive::IO(JourneyRealtime& v)
    { (*this)(v.paused,v.speed,v.elapsed,v.activeSeconds,v.pulses,v.orders); }
    void JourneyArchive::IO(JourneyChainSignal& v)
    { (*this)(v.sequence); }
    void JourneyArchive::IO(JourneyBattleNotice& v)
    { (*this)(v.kind,v.army,v.tile,v.turn,v.name,v.text); }
    void JourneyArchive::IO(JourneyBattleWatch& v)
    { (*this)(v.stalled,v.tracking,v.lastOrders,v.history); }
    void JourneyArchive::IO(JourneySaveData& v)
    { (*this)(v.journey,v.beforeBattle,v.realtime,v.signal,v.watch,v.orders,v.tactics,v.selected,v.target,v.dialogue,v.preparation,v.interlocutor,v.realtimeMode,v.pauseOnAlert,v.threatOverlay); }
}
