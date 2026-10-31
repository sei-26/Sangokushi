#include "StrategistJourney.hpp"
namespace frontline
{
    std::vector<JourneyDialogueLine> PreparedJourneyConversation(const StrategistJourney&,int);
    std::vector<JourneyDialogueLine> JourneyConversation(const StrategistJourney& j,int officer)
    {
        auto lines=PreparedJourneyConversation(j,officer);
        if(j.stage!=JourneyStage::Debrief && j.stage!=JourneyStage::Complete) return lines;
        const auto& h=j.history[j.mission];const auto& review=h.review;const int who=std::clamp(officer,0,2);
        if(!h.completed || !review.reliefTracked) return lines;
        const JourneyReliefRecord* record=nullptr;
        for(auto it=review.relief.rbegin();it!=review.relief.rend();++it)
            if(it->departedTurn && it->arrivedTurn && (who==it->outgoing || who==it->incoming)) { record=&*it;break; }
        if(!record) return lines;
        const auto& r=*record;using O=JourneyReliefOutcome;using Lines=std::vector<JourneyDialogueLine>;Lines response;
        const bool painful=!h.won || h.withdrawn || h.story.costly || (h.story.pledge!=JourneyPledge::None && !h.story.pledgeKept);
        if(r.outcome==O::Recovered)
        {
            if(who==0) response={{0,U"火は繋ぎ直せた。でも、空いた時間は確かにあったよ。"},{3,U"同じ合図で動かせば間に合うと思った。受け渡す場所と時間を、別々に考えないとな。"},{0,U"なら次は、出る前にもう一度、私たちの足元を見よう。"}};
            else if(who==1) response={{1,U"火は戻った。だが、一度途切れた事実は消えぬ。"},{3,U"戻せたから良かった、で済ませない。空く時間を、策の代償として引き受ける。"},{1,U"それでよい。次は待つ命令にも、理由を持たせよ。"}};
            else response={{2,U"火、繋ぎ直したな。けど、待ってる間は長かったぜ。"},{3,U"動き出す速さより、引き継ぐ時刻を合わせるべきだった。"},{2,U"おう。次は一歩ずつ声を掛けろ。俺もちゃんと待つ。"}};
        }
        else if(r.outcome==O::Lost)
        {
            if(who==0) response={{0,U"人を入れれば守れる。そう見えたのかい。"},{3,U"交代の形は作れた。でも、守りが続くとは限らなかった。そこまで見切れていなかった。"},{0,U"誰を置くかだけじゃない。そこで何ができるかも、一緒に見よう。"}};
            else if(who==1) response={{1,U"二隊は動いた。だが、守る条件は揃わなかった。"},{3,U"空いた場所を埋めるだけで、引き継いだ気になっていた。"},{1,U"その場所に残る者の戦いまで、君の策に入れよ。"}};
            else response={{2,U"俺たちは動いたぜ。なのに火が続かなかった。何を見落とした？"},{3,U"隊の位置ばかり見ていた。引き継いだ先で守れるかまで、俺が考えるべきだった。"},{2,U"なら次は、そこまで言えよ。動く前なら、俺たちも考えられる。"}};
        }
        else if(r.outcome==O::Unconfirmed)
            response={{who,who==0 ? U"二人とも場所は替わった。でも、その後まで守れたとは、まだ言えないね。":who==1 ? U"交代の動きは終えた。守りが続いたかは、確かめておらぬ。":U"場所は替わったけどよ。そのあとまで持ったかは、見届けてねえな。"},{3,U"合図を出したことと、守り切ったことを一緒にはしない。今日、確かめたところまでを覚えておく。"},{who,who==1 ? U"うむ。次は、残る者のその先まで見届けよ。":U"次は、残った隊のその先まで、一緒に見よう。"}};
        else if(r.outcome==O::Held && !r.allSecured)
            response={{who,who==1 ? U"この場所は繋いだ。だが、もう一方の火は続いていなかった。":U"交代した場所は守れた。でも、もう一方の烽火まで保てたわけじゃない。"},{3,U"一か所を守った手応えで、戦場全部を見た気になりたくない。"},{who,who==2 ? U"そうだな。次は、両方見える場所から声を掛けてくれ。":U"次は、両方の守りを見てから決めよう。"}};
        else if(r.outcome==O::Held && painful)
            response={{who,who==0 ? U"交代した直後の火は繋げた。けれど、今日の結果は別に考えなければならないね。":who==1 ? U"受け渡した守りは、一度は繋いだ。今日の結果を、それで覆い隠すな。":U"交代は噛み合った。けどよ、今日の結果まで笑って済ましちゃ駄目だろ。"},{3,U"繋げた一手で、今日の結果を隠したくない。誰を出して誰を残したか、全部見直す。"},{who,who==2 ? U"次は一人で背負い込むな。俺たちにも考えさせろ。":U"次の軍議で、私の置き場所から話そう。"}};
        else if(r.outcome==O::Held)
        {
            if(who==0) response={{0,U"交代の後の火は、繋がったね。君は、出る者と残る者の両方に役目を渡した。"},{3,U"誰かを前へ出すなら、その背中を誰に任せるかまで決めたい。"},{0,U"次も、帰ってくる場所を残そう。私もそこを引き受けるから。"}};
            else if(who==1) response={{1,U"出す者と残る者。君は、その双方の責を負った。"},{3,U"前へ出た隊の手柄だけを見ない。火を預かった隊も、策の真ん中にいた。"},{1,U"心得たなら、次も私に役目を告げよ。進む時も、留まる時もだ。"}};
            else if(who==r.outgoing) response={{2,U"前へ行けって声が来た時、後ろには仲間の旗があった。あれなら、俺も踏み出せる。"},{3,U"お前に任せる前に、お前の後ろを任せる相手を決める。それも俺の采配だ。"},{2,U"おう！ 次も声を切らすなよ。俺も勝手に飛び出さねえからな。"}};
            else response={{2,U"俺が残って、仲間が出る。留まるのも策ってわけか。"},{3,U"そこを預けられたから、前へ一隊回せた。お前を残したのには理由がある。"},{2,U"なら堂々と言えよ。『ここを頼む』ってな。任されたら、俺だって踏ん張るぜ。"}};
        }
        if(j.Definition().kind!=JourneyObjective::HoldPositions || (r.source!=j.goal && r.source!=j.Definition().secondGoal))
            for(auto& line:response)
                for(std::size_t p=0;(p=line.text.find(U"火",p))!=std::u32string::npos;p+=2) line.text.replace(p,1,U"守り");
        lines.insert(lines.end(),response.begin(),response.end());return lines;
    }
}
