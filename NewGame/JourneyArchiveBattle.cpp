#include "JourneyArchive.hpp"

namespace frontline
{
    void JourneyArchive::IO(ArmyContact& v)
    { (*this)(v.id,v.seenDay,v.current,v.observed); }
    void JourneyArchive::IO(BattleIntelligence& v)
    { (*this)(v.visible,v.owners,v.contacts); }
    void JourneyArchive::IO(TacticChain& v)
    { (*this)(v.concealed,v.moved,v.spent,v.disordered,v.pressured,v.inspired,v.signatureUsed); }
    void JourneyArchive::IO(ChainOrder& v)
    { (*this)(v.army,v.enemy,v.tactic,v.destination); }
    void JourneyArchive::IO(ChainUnitChange& v)
    { (*this)(v.army,v.tile,v.troops,v.morale,v.concealed,v.disordered,v.pressured,v.inspired); }
    void JourneyArchive::IO(ChainEvent& v)
    {
        (*this)(v.army,v.enemy,v.from,v.to,v.damage,v.tactic,v.success,v.reason,v.changes);
        if(version>=2) (*this)(v.combo,v.comboPercent,v.setupPercent);
        else if(reading) { v.combo=0;v.comboPercent=v.setupPercent=100; }
    }
    void JourneyArchive::IO(ChainResult& v)
    { (*this)(v.events,v.completed,v.prediction); }
    void JourneyArchive::IO(JourneyBattleReview& v)
    { (*this)(v.resolution,v.endedAt,v.enemyStops,v.trafficStops,v.tacticAttempts,v.tacticSteps,v.longestChain); }
    void JourneyArchive::IO(JourneyResult& v)
    { (*this)(v.completed,v.won,v.withdrawn,v.rescued,v.survivors,v.casualties,v.review,v.route); }
    void JourneyArchive::IO(StrategistJourney& v)
    { (*this)(v.world,v.chain,v.intelligence,v.stage,v.mission,v.turn,v.enemyPolicy,v.trust,v.bestChain,v.preparation,v.deployed,v.casualties,v.survivors,v.goal,v.heldTurns,v.convoyStep,v.convoyEscort,v.recoveredCargo,v.guarding,v.won,v.reinforced,v.promise,v.convoyRoute,v.battleReview,v.withdrawn,v.history,v.lastChain,v.reports,v.message); }
}
