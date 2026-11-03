#include "JourneyAftermathRole.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    JourneyEpisodeLine RoleAftermathVoice(int tone,int actor,int other,bool incoming);
    JourneyRoleAftermath ReadRoleAftermath(const StrategistJourney& j)
    {
        JourneyRoleAftermath result;
        if(j.stage!=JourneyStage::Debrief || j.mission<0 || j.mission>=int(j.history.size())) return result;
        const auto& h=j.history[j.mission];const auto& review=h.review;
        if(!h.completed || !review.reliefTracked) return result;
        const JourneyReliefRecord* record=nullptr;
        for(auto it=review.relief.rbegin();it!=review.relief.rend();++it)
            if(it->outgoing>=0 && it->outgoing<3 && it->incoming>=0 && it->incoming<3 && it->outgoing!=it->incoming && it->departedTurn>0 && it->arrivedTurn>0)
            { record=&*it;break; }
        if(!record) return result;
        const auto& r=*record;using O=JourneyReliefOutcome;
        if(r.outcome!=O::Held && r.outcome!=O::Recovered && r.outcome!=O::Lost && r.outcome!=O::Unconfirmed) return result;
        if(r.outcome!=O::Unconfirmed && r.checkedTurn<=0) return result;
        const bool beacon=j.Definition().kind==JourneyObjective::HoldPositions && (r.source==j.goal || r.source==j.Definition().secondGoal);
        const bool partial=beacon && !r.allSecured && (r.outcome==O::Held || r.outcome==O::Recovered);
        const bool costly=h.story.recorded ? h.story.costly:(j.deployed+j.convoyEscort>0 && h.casualties*4>=j.deployed+j.convoyEscort);
        const bool broken=h.story.recorded && h.story.pledge!=JourneyPledge::None && !h.story.pledgeKept;
        const bool painful=!h.won || h.withdrawn || costly || broken;
        // Held, recovered, failed, unchecked, one wing only, or a local success with a painful result.
        const int tone=r.outcome==O::Lost ? 2:r.outcome==O::Unconfirmed ? 3:partial ? 4:r.outcome==O::Recovered ? 1:painful ? 5:0;
        const char32_t* states[]={U"交代直後の守備を確認",U"空白のあと守備を回復",U"到着後の守備は成立せず",U"移動は完了、守備は未確認",U"交代した場所は守備を確認、両翼の維持は未成立",U"交代直後は守備を確認。戦い全体の結果とは分けて振り返る"};
        result.summary=std::u32string(EpisodeSpeakerName(r.outgoing))+U" → "+EpisodeSpeakerName(r.incoming)+U" / "+states[tone];
        std::u32string facts=std::u32string(EpisodeSpeakerName(r.outgoing))+U"が持ち場を出て、"+EpisodeSpeakerName(r.incoming)+U"がそこへ着いた。"+states[tone]+U"。";
        if(partial && r.outcome==O::Recovered) facts+=U"交代の途中には、守りの空白もあった。";
        if(h.withdrawn || !h.won) facts+=U"その一手があっても、任務を果たしたことにはならなかった。";
        else if(costly) facts+=U"勝利の傍らには、大きな兵の損耗が残った。";
        if(broken) facts+=U"出陣前の約束も、果たせていない。";
        result.lines.push_back({-1,facts,U"主人公は、出た隊と残った隊の駒を並べる。"});
        auto living=[&](int id) { return id<int(j.world.armies.size()) && j.world.armies[id].troops>0; };
        if(living(r.incoming)) result.lines.push_back(RoleAftermathVoice(tone,r.incoming,r.outgoing,true));
        if(living(r.outgoing)) result.lines.push_back(RoleAftermathVoice(tone,r.outgoing,r.incoming,false));
        const char32_t* hero[]={U"前へ出る役も、残る役も、一緒に頼む。次も二人の名を呼ぶぞ。",U"戻せたから、よしでは済ませない。次は合図の前に、受け渡す時刻まで合わせよう。",U"埋まった場所だけ見てた。残る隊が守れるところまで、俺が策に入れるべきだった。",U"見届けたところまでを、俺の戦果にする。守ったことにはしない。",U"一つの火に安心して、もう一方を見落とした。次は両方を見て合図する。",U"つなげた一手で、今日の結果を隠さない。次の軍議で、俺の出した命令から見直す。"};
        result.lines.push_back({3,hero[tone],U"主人公は、二つの駒を結ぶ線を地図に残した。"});
        return result;
    }
}
