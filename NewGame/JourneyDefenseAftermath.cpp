#include "JourneyAftermathRole.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    JourneyEpisodeLine DefenseAftermathVoice(int actor,int other,int kind,bool painful);
    JourneyRoleAftermath ReadDefenseAftermath(const StrategistJourney& j)
    {
        JourneyRoleAftermath result;
        if(j.stage!=JourneyStage::Debrief || j.mission<0 || j.mission>=int(j.history.size()) || j.Definition().kind!=JourneyObjective::HoldPositions) return result;
        const auto& h=j.history[j.mission];const auto& r=h.review;
        if(!h.completed || r.endedAt<=0 || r.endedAt!=j.turn || r.confirmedDefenders<0 || r.confirmedDefenders>7 || r.spentGuardFailures<0 || r.spentGuardFailures>7) return result;
        if(h.won!=j.won || h.withdrawn!=j.withdrawn || r.resolution==JourneyResolution::None || (h.won && r.resolution!=JourneyResolution::Held)) return result;
        const int defenders=r.confirmedDefenders,failures=r.spentGuardFailures;
        if(!(defenders|failures)) return result;
        const int chain=r.coordinatedOfficers>0 && r.coordinatedOfficers<=7 && (r.coordinatedOfficers&(r.coordinatedOfficers-1)) &&
            r.tacticAttempts>0 && r.tacticSteps>=2 && r.longestChain>=2 ? r.coordinatedOfficers:0;
        auto names=[](int mask)
        {
            std::u32string text;for(int id=0;id<3;++id) if(mask&(1<<id)) { if(!text.empty()) text+=U"・";text+=EpisodeSpeakerName(id); }return text;
        };
        auto add=[&](int mask,const char32_t* label)
        { if(mask) { if(!result.summary.empty()) result.summary+=U" ／ ";result.summary+=std::u32string(label)+names(mask); } };
        add(chain,U"連鎖：");add(defenders,U"烽火の守備：");add(failures,U"固守できず：");
        const bool costly=h.story.recorded ? h.story.costly:(j.deployed+j.convoyEscort>0 && h.casualties*4>=j.deployed+j.convoyEscort);
        const bool broken=h.story.recorded && h.story.pledge!=JourneyPledge::None && !h.story.pledgeKept;
        const bool painful=!h.won || h.withdrawn || costly || broken;
        std::u32string facts;
        if(chain) facts+=U"この戦いの連鎖には、"+names(chain)+U"が手を貸した。";
        if(defenders) facts+=U"火を支えた隊として、"+names(defenders)+U"の名も残る。";
        if(failures) { if(!facts.empty()) facts+=U"ただ、";facts+=names(failures)+U"には、手を使い果たし、火の前で構えられない時もあった。"; }
        if(!h.won || h.withdrawn) facts+=U"任務は果たせなかった。局所の働きだけで、その結果を塗り替えない。";
        else if(costly) facts+=U"任務は果たした。だが、大きな兵の損耗も残った。";
        else facts+=U"その働きと代償を含め、任務は果たされた。";
        if(broken) facts+=U"出陣前の約束は、果たせなかった。";
        result.lines.push_back({-1,facts,U"火の印と、武将の名が残る地図を広げる。"});
        auto living=[&](int id) { return id>=0 && id<3 && id<int(j.world.armies.size()) && j.world.armies[id].faction==j.world.player && j.world.armies[id].troops>0; };
        auto first=[&](int mask,int except)
        { for(int id=0;id<3;++id) if(id!=except && (mask&(1<<id)) && living(id)) return id;return -1; };
        int lead=first(failures,-1);if(lead<0) lead=first(defenders,-1);if(lead<0) lead=first(chain,-1);
        int partner=first(lead>=0 && (failures&(1<<lead)) ? defenders:chain,lead);
        if(partner<0) partner=first(defenders|chain|failures,lead);
        auto kind=[&](int id) { return (failures&(1<<id)) ? 1:(defenders&(1<<id)) ? 0:2; };
        if(lead>=0) result.lines.push_back(DefenseAftermathVoice(lead,partner,kind(lead),painful));
        if(partner>=0) result.lines.push_back(DefenseAftermathVoice(partner,lead,kind(partner),painful));
        const char32_t* hero=!h.won || h.withdrawn ? U"働いた名があっても、俺の命令で任務は果たせなかった。次は、残す手まで一緒に考えてくれ。":
            costly || broken ? U"誰が動き、誰が火の前にいたかも忘れない。残した代償を、次の命令で引き受ける。":
            failures ? U"使う手は俺が選んだ。その先で守れるかまで、俺の采配だ。次は残す手も、皆と決める。":
            U"前へ出す名と、火の前に残す名。次も俺が呼ぶ。一人に全部を背負わせず、皆で策をつくろう。";
        result.lines.push_back({3,hero,U"主人公は、攻めと守りの駒を自分の手で並べ直す。"});return result;
    }
}
