#pragma once
#include "CampaignTypes.hpp"
#include "JourneyRelief.hpp"
#include <vector>

namespace frontline
{
    enum class JourneyConvoyRoute { Road, SouthernWood };
    struct JourneyConvoyPlan
    {
        const char32_t* name;
        const char32_t* description;
        int guideGold;
        std::vector<int> tiles; // Includes the starting cell.
    };
    const JourneyConvoyPlan& ConvoyPlan(JourneyConvoyRoute);
    enum class JourneyResolution { None, EnemyBroken, ExitSecured, Delivered, Held, Withdrawn, ForceLost, ConvoyLost, Deadline, Captured };
    enum class YangrenCounter { None, ForestLure, Disrupted, Defeated };
    struct JourneyYangrenReview
    {
        bool tracked=false, wasBlocked=false;
        int blockers=0; // The two enemy slots observed at the last halt.
        int pendingOfficer=-1, roadOfficer=-1, cavalryOfficer=-1;
        YangrenCounter counter=YangrenCounter::None;

        int TrustGain(bool won) const
        {
            return tracked && won ? 2 + (roadOfficer>=0 ? 3:0) + (counter!=YangrenCounter::None ? 2:0) : 0;
        }
        bool operator==(const JourneyYangrenReview& other) const
        {
            return tracked==other.tracked && wasBlocked==other.wasBlocked && blockers==other.blockers &&
                pendingOfficer==other.pendingOfficer && roadOfficer==other.roadOfficer &&
                cavalryOfficer==other.cavalryOfficer && counter==other.counter;
        }
    };
    struct JourneyBattleReview
    {
        JourneyResolution resolution=JourneyResolution::None;
        int endedAt=0,enemyStops=0,trafficStops=0,tacticAttempts=0,tacticSteps=0,longestChain=0;
        int coordinatedOfficers=0;
        int confirmedDefenders=0,spentGuardFailures=0; // Positive per-officer local check facts, never timing or continuous defense.
        bool reliefTracked=false;
        std::vector<JourneyReliefRecord> relief;
        bool preparationTracked=false;
        int openingAmbushes=0,resupplyTurn=0,resupplyMorale=0,postSupplySteps=0;
        JourneyYangrenReview yangren;
    };
}
