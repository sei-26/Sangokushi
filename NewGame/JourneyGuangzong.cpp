#include "StrategistJourney.hpp"

namespace frontline
{
    int StrategistJourney::GuangzongCommandsLeft() const
    {
        const auto& review=battleReview.guangzong;
        return review.alarmTurn ? std::max(0,review.alarmTurn+GuangzongWindow-turn):0;
    }

    const JourneyBattleReview::Guangzong& StrategistJourney::GuangzongSupport() const
    {
        return history[YellowTurbanMission].review.guangzong;
    }

    void StrategistJourney::CheckGuangzong()
    {
        auto& review=battleReview.guangzong;
        if(!review.alarmTurn)
        {
            bool underPressure=!TacticChain::Living(world,3) || chain.disordered[3] || world.armies[3].morale<=70;
            for(int officer=0;officer<3 && !underPressure;++officer)
                underPressure=TacticChain::Living(world,officer) && !chain.concealed[officer] &&
                    world.MapDistance(world.armies[officer].tile,goal)<=2;
            if(!underPressure) return;
            review.alarmTurn=turn;
            Report(U"前陣に迫る義勇軍を見て、黄巾の南翼が街道への転進を始めた。今なら前陣を攻めるか、街道へ一隊を送れる。");
        }
        if(!GuangzongCommandsLeft() || review.roadOfficer>=0 || !PositionSecured(GuangzongRoad)) return;
        for(int officer=0;officer<3;++officer)
            if(world.armies[officer].tile==GuangzongRoad && CanOccupyObjective(world.armies[officer]) && !chain.disordered[officer])
            {
                review.roadOfficer=officer;
                ++world.revision;
                Report(world.generals[officer].name+U"隊が街道を開いた。民は脇道へ退いた。前陣へ向かおう。");
                return;
            }
    }
}
