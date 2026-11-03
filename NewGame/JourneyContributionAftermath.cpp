#include "JourneyAftermathRole.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    JourneyEpisodeLine ContributionAftermathVoice(int actor,int other,int kind,bool painful);
    JourneyRoleAftermath ReadContributionAftermath(const StrategistJourney& j)
    {
        JourneyRoleAftermath result;
        if(j.stage!=JourneyStage::Debrief || j.mission<0 || j.mission>=int(j.history.size())) return result;
        const auto& h=j.history[j.mission];const auto& r=h.review;
        if(!h.completed || r.endedAt<=0 || r.endedAt!=j.turn) return result;
        auto living=[&](int id) { return id>=0 && id<3 && id<int(j.world.armies.size()) && j.world.armies[id].faction==j.world.player && j.world.armies[id].troops>0; };
        int captor=-1;
        if(j.Definition().kind==JourneyObjective::CaptureCamp && r.resolution==JourneyResolution::Captured && h.won && j.won && !h.withdrawn && !j.withdrawn &&
           StrategistJourney::OnMap(j.goal) && j.world.tiles[j.goal].owner==j.world.player)
            for(int id=0;id<3;++id) if(living(id) && j.world.armies[id].tile==j.goal && j.world.armies[id].troops>=150 && j.world.armies[id].morale>=25)
            { if(captor>=0) { captor=-1;break; }captor=id; }
        const int mask=r.coordinatedOfficers>=0 && r.coordinatedOfficers<=7 && (r.coordinatedOfficers&(r.coordinatedOfficers-1)) &&
            r.tacticAttempts>0 && r.tacticSteps>=2 && r.longestChain>=2 ? r.coordinatedOfficers:0;
        if(captor<0 && !mask) return result;
        std::u32string names;
        for(int id=0;id<3;++id) if(mask&(1<<id)) { if(!names.empty()) names+=U"・";names+=EpisodeSpeakerName(id); }
        const bool costly=h.story.recorded ? h.story.costly:(j.deployed+j.convoyEscort>0 && h.casualties*4>=j.deployed+j.convoyEscort);
        const bool broken=h.story.recorded && h.story.pledge!=JourneyPledge::None && !h.story.pledgeKept;
        const bool painful=!h.won || h.withdrawn || costly || broken;
        std::u32string facts;
        if(captor>=0)
        {
            result.summary=std::u32string(U"河港に旗を立てた：")+EpisodeSpeakerName(captor);
            facts=std::u32string(EpisodeSpeakerName(captor))+U"が指揮所に入り、河港の旗が替わった。";
        }
        if(mask)
        {
            if(!result.summary.empty()) result.summary+=U" ／ ";result.summary+=U"連鎖に参加："+names;
            facts+=U"この戦いで連鎖をつないだのは、"+names+U"。";
        }
        if(h.withdrawn || !h.won) facts+=U"だが、任務は果たせなかった。つながった一手と、戦い全体の結果は別に残った。";
        else if(costly) facts+=U"任務は果たした。けれど、大きな兵の損耗も残った。";
        else if(captor<0) facts+=U"その一手を含む采配で、任務を果たした。";
        if(broken) facts+=U"出陣前の約束は、果たせなかった。";
        result.lines.push_back({-1,facts,U"主人公は、旗の印と武将たちの駒を地図に残す。"});
        int lead=captor;
        if(lead<0) for(int id=0;id<3;++id) if((mask&(1<<id)) && living(id)) { lead=id;break; }
        int partner=-1;
        for(int id=0;id<3;++id) if(id!=lead && (mask&(1<<id)) && living(id)) { partner=id;break; }
        if(captor>=0 && partner<0) for(int id=0;id<3;++id) if(id!=lead && living(id)) { partner=id;break; }
        if(lead>=0) result.lines.push_back(ContributionAftermathVoice(lead,partner,captor>=0 ? 0:1,painful));
        if(partner>=0) result.lines.push_back(ContributionAftermathVoice(partner,lead,mask&(1<<partner) ? 2:3,painful));
        const auto hero=painful ? U"働いてくれた名前も、残した代償も忘れない。次の軍議では、俺の命令から見直す。":
            captor>=0 ? U"最後に旗を立てるところまでが、俺たちの策だ。皆、次の地図も一緒に開いてくれ！":
            U"その一手は、お前たちがつないだ。次も俺が合図を出す。誰か一人に、全部背負わせない。";
        result.lines.push_back({3,hero,U"主人公は、手柄を一つの駒に集めず、地図を仲間へ向けた。"});
        return result;
    }
}
