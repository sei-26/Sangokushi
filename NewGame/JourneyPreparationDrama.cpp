#include "StrategistJourney.hpp"
namespace frontline
{
    std::vector<JourneyDialogueLine> BaseJourneyConversation(const StrategistJourney&,int);
    std::vector<JourneyDialogueLine> PreparedJourneyConversation(const StrategistJourney& j,int officer)
    {
        auto lines=BaseJourneyConversation(j,officer);
        if(j.stage!=JourneyStage::Debrief && j.stage!=JourneyStage::Complete) return lines;
        const auto& h=j.history[j.mission];const auto& r=h.review;
        if(!h.completed || !r.preparationTracked) return lines;
        const int who=std::clamp(officer,0,2);
        using Lines=std::vector<JourneyDialogueLine>;Lines response;
        const bool painful=!h.won || h.withdrawn || h.story.costly ||
            (h.story.pledge!=JourneyPledge::None && !h.story.pledgeKept);
        if(who==1 && j.preparation==0)
        {
            if(r.openingAmbushes>0)
                response={{1,U"初手の伏兵は届いた。森へ入れる、その一手前を考えたのは君だ。"},
                    {3,painful ? U"でも、そこから先まで守り切れなかった。成功した一手だけ眺めて終わりにはしない。":U"森に置けば勝てると思ってた。お前が待てる間に、次の隊を間に合わせるところまでが俺の仕事なんだな。"},
                    {1,U"そうだ。私を隠すだけでは策にならぬ。何のために待たせるか、次も聞かせろ。"}};
            else response={{1,U"初手の伏兵にはつながらなかったな。地図の印は、敵を呼んではくれぬ。"},
                {3,U"使わずに済んだのか、機会を逃したのか。そこは、配置を見直して確かめたい。"},
                {1,U"伏兵を使わず済む戦もある。だが、使うつもりなら、私の待つ時間まで策に入れよ。"}};
        }
        else if(j.preparation==1 && who==0)
        {
            if(r.resupplyTurn==0) response={{0,U"予備の糧は、封を切らず戻ったね。"},
                {3,painful ? U"残せた、とはまだ言えない。使うべき時を逃したのか、地図を見直したい。":U"使うために持ったけど、使い切ることが役目じゃない。次に必要な時のために残す。"},
                {0,U"うん。袋の数と、帰ってきた人の顔を、両方見て決めよう。"}};
            else if(r.resupplyMorale==0) response={{0,U"補給の時、士気はすでに保てていたね。兵たちには糧も渡った。"},
                {3,U"気力だけじゃ、道は歩けないからな。配った糧を、その先の行軍へつなぐところまで考える。"},
                {0,U"腹を満たすことも、兵を支える采配だ。次に進む距離も、一緒に見ておこう。"}};
            else response={{0,U"補給の合図は、君が出したね。前へ出ろ、だけが軍師の言葉ではないんだ。"},
                {3,painful ? U"立て直したところはあった。でも、それで今日の結果を帳消しにはできない。":U"もう一度戦えと言うなら、その力を渡すところまで引き受けたい。"},
                {0,U"なら、次の軍議でも兵の話から始めよう。私もそこにいる。"}};
        }
        else if(j.preparation==1 && who==2 && r.resupplyTurn>0)
        {
            if(r.postSupplySteps>0) response={{2,U"糧を回したあとも、戦法を使えたな。軍師、お前の仕事は最初の号令だけじゃねえんだな。"},
                {3,painful ? U"でも、もう一手打てたことと、皆を帰せたことは別だ。そこを忘れたくない。":U"戦い始めたら、後はお前たち任せ。それじゃ、一緒にいる意味がないだろ。"},
                {2,U"おう。だったら最後まで声を出せよ。俺も聞いてるからな。"}};
            else response={{2,U"補給は出したけど、そのあとの戦法にはつながらなかったな。"},
                {3,U"立て直しただけで、次の一手まで決まるわけじゃなかった。動く場所と相手も見ないとな。"},
                {2,U"無理に撃てって話じゃねえぞ。行けでも戻れでも、次の声を聞かせろよ。"}};
        }
        lines.insert(lines.end(),response.begin(),response.end());return lines;
    }
}
