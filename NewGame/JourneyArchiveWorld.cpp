#include "JourneyArchive.hpp"

namespace frontline
{
    void JourneyArchive::IO(Tile& v)
    { (*this)(v.terrain,v.owner); }
    void JourneyArchive::IO(City& v)
    { (*this)(v.name,v.tile,v.owner,v.troops,v.food,v.gold,v.farming,v.commerce,v.order,v.logistics,v.worker,v.work,v.workLeft,v.helper,v.governor,v.level); }
    void JourneyArchive::IO(Region& v)
    { (*this)(v.name,v.tile,v.city,v.owner); }
    void JourneyArchive::IO(General& v)
    { (*this)(v.name,v.faction,v.home,v.leadership,v.readyDay,v.politics,v.specialty,v.intelligence,v.charm,v.trait,v.tactic,v.policy); }
    void JourneyArchive::IO(Army& v)
    { (*this)(v.faction,v.general,v.tile,v.target,v.troops,v.cargoFood,v.food,v.morale,v.movement,v.aiAssemblyDays,v.arm,v.stance,v.supplied,v.retreat,v.path,v.tacticLeft,v.tacticReadyDay,v.tacticQueued,v.scouting); }
    void JourneyArchive::IO(TacticEvent& v)
    { (*this)(v.general,v.faction,v.tile,v.target,v.tactic); }
    void JourneyArchive::IO(ReturnEvent& v)
    { (*this)(v.general,v.city,v.troops,v.cargoFood); }
    void JourneyArchive::IO(Bond& v)
    { (*this)(v.a,v.b,v.value); }
    void JourneyArchive::IO(Mission& v)
    { (*this)(v.general,v.helper,v.city,v.target,v.faction,v.targetFaction,v.left,v.kind); }
    void JourneyArchive::IO(Assignment& v)
    { (*this)(v.general,v.from,v.to,v.faction,v.left); }
    void JourneyArchive::IO(Training& v)
    { (*this)(v.city,v.first,v.second,v.faction,v.left); }
    void JourneyArchive::IO(Chronicle& v)
    { (*this)(v.day,v.text); }
    void JourneyArchive::IO(Campaign& v)
    { (*this)(v.tiles,v.cities,v.regions,v.tileRegion,v.generals,v.armies,v.tacticEvents,v.returnEvents,v.log,v.bonds,v.missions,v.assignments,v.training,v.chronicle,v.policyCouncil,v.truceUntil,v.regard,v.randomState,v.player,v.day,v.commands,v.result,v.commandBudget,v.revision,v.legacyLayout,v.hexMap,v.aiPlannedDay,v.aiCommands); }
}
