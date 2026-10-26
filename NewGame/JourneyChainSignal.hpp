#pragma once
#include "StrategistJourney.hpp"

namespace frontline
{
    enum class ChainSignalState { Off, Waiting, Ready, Invalid };
    struct ChainSignalRead
    {
        ChainSignalState state = ChainSignalState::Off;
        int completed = 0, total = 0, blockingArmy = -1;
        std::u32string reason;
    };
    // An explicit, single-use authorization for this exact sequence. Inspection
    // uses the same sanitized prediction as the player's reservation cards.
    class JourneyChainSignal
    {
    public:
        bool Arm(const StrategistJourney&, const std::vector<ChainOrder>&);
        void Cancel() { sequence.clear(); }
        bool Armed() const { return !sequence.empty(); }
        ChainSignalRead Inspect(const StrategistJourney&, const std::vector<ChainOrder>&) const;
        bool Release(const StrategistJourney&, const std::vector<ChainOrder>&, bool running);
        std::array<bool,3> ReservedAttacks() const;
    private:
        friend struct JourneyArchive;
        std::vector<ChainOrder> sequence;
        static ChainSignalRead Assess(const StrategistJourney&, const std::vector<ChainOrder>&);
    };
}
