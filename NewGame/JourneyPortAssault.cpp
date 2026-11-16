#include "StrategistJourney.hpp"
#include "JourneyBattlefield.hpp"

namespace frontline
{
    void StrategistJourney::RecordBattleMove(int army,int from,int to)
    {
        RecordReliefMove(army,from,to);
        if(!IsPortAssault() || army<0 || army>=3 || !OnMap(from) || !OnMap(to) ||
            world.tiles[from].terrain!=Terrain::Bridge || to%Width<=from%Width) return;
        const int bridge=from/Width<goal/Width ? 0:1;
        auto& first=battleReview.port.crossedBy[bridge];
        if(first<0) first=army;
    }

    int StrategistJourney::PortBlockers() const
    {
        if(!IsPortAssault()) return 0;
        int mask=0;
        for(int id=3;id<5;++id)
            if(TacticChain::Living(world,id) && world.Hostile(world.player,world.armies[id].faction) &&
                world.MapDistance(world.armies[id].tile,goal)<=1) mask|=1<<(id-3);
        return mask;
    }

    void StrategistJourney::ExpirePortOpening()
    {
        auto& port=battleReview.port;
        if(port.clearedBy>=0 && (turn-port.clearedOn>PortOpeningCycles || PortBlockers()!=0))
        {
            port.clearedBy=-1;
            port.clearedOn=0;
        }
    }

    void StrategistJourney::RecordPortClearance(int officer,int enemy,int before,int after)
    {
        auto& port=battleReview.port;
        if(after)
        {
            port.clearedBy=-1;
            port.clearedOn=0;
        }
        else if(officer>=0 && officer<3 && enemy>=3 && enemy<5 && before==(1<<(enemy-3)))
        {
            port.clearedBy=officer;
            port.clearedOn=turn;
        }
    }

    void StrategistJourney::RecordPortChain(int blockers,const ChainResult& result)
    {
        for(const auto& step:result.events)
        {
            if(!step.success) continue;
            int remaining=blockers;
            for(const auto& unit:step.changes)
                if(unit.army>=3 && unit.army<5)
                {
                    const int bit=1<<(unit.army-3);
                    if(unit.troops>0 && world.MapDistance(unit.tile,goal)<=1) remaining|=bit;
                    else remaining&=~bit;
                }
            RecordPortClearance(step.army,step.enemy,blockers,remaining);
            blockers=remaining;
        }
    }

    void StrategistJourney::RecordPortCapture()
    {
        for(int id=0;id<3;++id)
            if(world.armies[id].tile==goal && CanOccupyObjective(world.armies[id]))
            {
                battleReview.port.capturedBy=id;
                return;
            }
    }
}
