#include "JourneySupport.hpp"
namespace frontline
{
    std::vector<int> JourneySupportContext(const StrategistJourney& j,const JourneyRealtime& clock,int army)
    {
        std::vector<int> key{int(j.stage),j.mission,j.turn,army,j.goal,j.Definition().secondGoal,int(j.Definition().kind)};
        for(int id=0;id<3;++id)
        {
            const auto& unit=j.world.armies[id];const auto& o=clock.orders[id];const auto& a=j.assignments[id];
            key.insert(key.end(),{unit.tile,unit.troops,unit.morale,unit.faction,int(j.chain.moved[id]),int(j.chain.spent[id]),int(j.chain.concealed[id]),int(j.guarding[id]),
                int(a.duty),a.tile,int(o.kind),o.destination,o.target,int(o.arrival),int(o.waypoints.size())});
            key.insert(key.end(),o.waypoints.begin(),o.waypoints.end());
        }
        for(int tile=0;tile<TileCount;++tile) key.insert(key.end(),{int(j.world.tiles[tile].terrain),int(j.intelligence[0].visible[tile])});
        for(const auto& city:j.world.cities) key.push_back(city.tile);
        for(const auto& seen:j.intelligence[0].contacts) if(seen.current && seen.observed.troops>0 && j.world.Hostile(j.world.player,seen.observed.faction))
            key.insert(key.end(),{seen.id,seen.observed.tile,seen.observed.troops,seen.observed.faction});
        return key;
    }
}
