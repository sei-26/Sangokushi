#include "JourneyPreludeOpening.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    JourneyEpisode PreludeOpeningAftermath(const StrategistJourney& j)
    {
        JourneyEpisode e;
        if(!j.campaignFlow || j.mission!=0 || j.stage!=JourneyStage::Debrief || !j.history[0].completed) return e;
        const auto& h=j.history[0];
        e.title=U"序章 ／ 初めて、軍師と呼ばれた日";e.place=U"義勇軍の帰陣 ／ 一八四年";
        e.setting=U"勝ち方と、連れ帰った兵。その両方を、三人は覚えている。";
        e.question=U"この一戦を携えて、黄巾の本戦へ。";
        if(!h.won)
        {
            e.lines={{3,U"道を開けなかった。俺の合図を待っていた人に、何て言えばいい。",U"主人公は、泥のついた旗を握りしめた。"},
                {0,U"帰った兵と一緒に、君もここにいる。言い訳より先に、皆の顔を見よう。",U"劉備は、主人公の横に腰を下ろした。"},
                {2,U"次はどこで俺を呼ぶ？ 同じ所で止まるなら、俺が声をかけてやる。",U"張飛が、畳みかけずに返事を待つ。"},
                {3,U"今日の結果は残す。やり直すなら、最初の役目から変える。先へ行くなら、その重さも持っていく。",U"再挑戦も、結果を受け入れて黄巾の本戦へ進むことも選べる。"}};
            return e;
        }
        if(h.review.resolution==JourneyResolution::Delivered)
            e.lines.push_back({4,U"車輪が止まっても、皆さんの旗が見えました。……子どもが、あの旗を描きたいと言っています。",U"季安は荷台を振り返り、今度は笑った。"});
        else if(h.review.resolution==JourneyResolution::ExitSecured)
            e.lines.push_back({0,U"敵の旗は残っている。でも、道は私たちのものだ。季安、今なら通れる。",U"主人公の確保した出口から、民の列が動き始める。"});
        else e.lines.push_back({2,U"先鋒が退いたぞ！ 軍師、見ろよ。あいつら、道を通っていく！",U"剣先の向こうで、季安が民の列へ手を振っていた。",4});
        if(h.story.costly)
            e.lines.push_back({1,U"道は開いた。だが、戻らぬ兵もいる。今日の勝ちを、その名で飾るな。次に誰を残すか、共に考えよう。",U"関羽は帰還兵の列に目を向けた。"});
        else if(h.review.longestChain>=2)
            e.lines.push_back({1,U"一人の一撃で終わらせず、次へつないだ。敵ではなく、味方の動きを見ていたな。",U"関羽は、主人公が合図を出した場所を振り返った。"});
        else e.lines.push_back({1,U"力を合わせる時も、一人で行く時もある。今日は、道が開いたところで戦いを終えた。それを覚えておけ。",U"関羽は追撃をやめた兵たちを、帰陣の列へ戻した。"});
        e.lines.push_back({3,h.story.costly ? U"勝っただけで、いい軍師にはなれない。帰れなかった兵の分まで、次の策を考える。":
            U"戦場で呼ばれた時、怖かった。でも、声を返してくれた。……次も、皆に聞こえる合図を出すよ。",U"主人公が旗を巻く。三人は、もうその横にいる。"});
        e.lines.push_back({0,U"ならば、本隊へ行こう。黄巾の大軍が待っている。君の策を、もっと大きな戦場へ届けよう。",U"初陣の戦果を携え、皇甫嵩の軍へ向かう。別働任務へ残ることも選べる。",1});
        return e;
    }
}
