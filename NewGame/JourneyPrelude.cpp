#include "StrategistJourney.hpp"

namespace frontline
{
    bool StrategistJourney::CanJoinMainForce() const
    {
        if(!campaignFlow || !grandCampaign || mission>=YellowTurbanMission || !history[0].completed) return false;
        return (stage==JourneyStage::Debrief && history[mission].completed) ||
            (stage==JourneyStage::Complete && history[mission].completed) || (stage==JourneyStage::Preparation && mission>0);
    }
    bool StrategistJourney::JoinMainForce()
    {
        if(!CanJoinMainForce()) return false;
        for(int m=1;m<YellowTurbanMission;++m)
            if(!history[m].completed)
            {
                auto& h=history[m];h={};h.completed=h.campaignFlow=true;h.difficulty=difficulty;
                h.review.resolution=JourneyResolution::JoinedMainForce;
            }
        shortPrelude=true;mission=YellowTurbanMission;stage=JourneyStage::Preparation;turn=1;enemyPolicy=1;
        battleReview={};assignments={};retryRelief.reset();retryDefense.reset();reinforced=won=withdrawn=false;
        reserveFood=0;convoyRoute=JourneyConvoyRoute::Road;legacyBattlefield=false;goal=Definition().goal;
        world.day+=28;
        const bool support=MusterCampaignSupport();
        Report(std::u32string(U"義勇軍は皇甫嵩の本隊へ合流した。道中の任務は戦果に数えず、前の戦いの兵・糧・信頼を持って広宗へ向かう。")+
            (support ? U" 官軍から出陣に必要な兵と糧が届いた。":U""));
        return true;
    }
    bool StrategistJourney::ContinuePrelude()
    {
        if(!CanJoinMainForce() || stage==JourneyStage::Preparation) return false;
        shortPrelude=false;
        return Continue();
    }
    std::u32string StrategistJourney::CampaignRoad() const
    {
        if(!campaignFlow) return {};
        const int current=mission<YellowTurbanMission ? 0:mission-YellowTurbanMission+1;
        const char32_t* chapters[]={U"序章",U"黄巾の乱",U"反董卓連合",U"陽人",U"洛陽"};
        std::u32string line;
        for(int i=0;i<5;++i)
        {
            if(i) line+=U" → ";
            line+=i==current ? U"【":U"";line+=chapters[i];line+=i==current ? U"】":U"";
        }
        return line;
    }
}
