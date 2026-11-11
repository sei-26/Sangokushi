#include "JourneyAftermathRole.hpp"
#include "StrategistJourney.hpp"

namespace frontline
{
    JourneyRoleAftermath ReadDutyAftermath(const StrategistJourney& j)
    {
        JourneyRoleAftermath result;
        if(j.stage!=JourneyStage::Debrief || j.mission<0 || j.mission>=int(j.history.size()) ||
            (!j.HasConvoy() && j.Definition().kind!=JourneyObjective::CaptureCamp)) return result;
        const auto& h=j.history[j.mission];
        if(!h.completed || h.review.resolution==JourneyResolution::None || h.review.endedAt!=j.turn) return result;

        int done=-1, missed=-1;
        for(int id=0;id<3;++id)
        {
            const auto& duty=h.assignments[id];
            if(duty.duty==JourneyDuty::None) continue;
            const bool ambush=duty.duty==JourneyDuty::Ambush;
            result.summary+=result.summary.empty() ? U"軍議の担当　":U" ／ ";
            result.summary+=std::u32string(EpisodeSpeakerName(id))+U"："+(ambush ? U"伏兵":U"固守")+
                (duty.completed ? (ambush ? U"準備":U"実行"):U"未成立");
            if(id>=int(j.world.armies.size()) || j.world.armies[id].troops<=0) continue;
            int& speaker=duty.completed ? done:missed;
            if(speaker<0) speaker=id;
        }

        // Give an unfulfilled role room to speak even if the main objective succeeded.
        const int speaker=missed>=0 ? missed:done;
        if(speaker<0) return result;
        const auto& duty=h.assignments[speaker];
        const char32_t* text;
        if(duty.duty==JourneyDuty::Ambush)
            text=duty.completed ? U"森に伏したとき、兵まで息を合わせてくれた。軍師、次も合図は一度でいい。":
                U"あの森では伏兵を仕込めなかった。軍師、次は待たせる時と、出す時を一緒に決めよう。";
        else
        {
            const char32_t* held[]={
                U"あそこで兵を止めて、盾を並べたよ。軍師、駆け出す時より、待つ時の方が君の声をよく覚えている。",
                U"任された場所で守りを固めた。前へ出る者がいるなら、踏みとどまる者も要る。次も私の名を呼べ。",
                U"待てって言うから、あそこで踏ん張ったぞ！ ……次は、いつまで待つかも先に言っとけよ、軍師。"};
            const char32_t* unheld[]={
                U"あそこでは、守りを固めるところまでいかなかったね。次は、私が踏みとどまれる場所を一緒に探そう。",
                U"任された場所で、守りを固められなかった。駒を置く時、次はそこで構える時間も残しておけ。",
                U"任された場所で踏ん張るはずが、守りを固められなかった。次は俺にできる仕事を、地図の前で言わせてくれ。"};
            text=duty.completed ? held[speaker]:unheld[speaker];
        }
        result.lines.push_back({speaker,text,U"武将は、軍議で自分の名を呼ばれた場所に駒を戻した。"});
        return result;
    }
}
