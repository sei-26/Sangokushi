#include "JourneyEpisode.hpp"
#include "StrategistJourney.hpp"

namespace frontline
{
    void BuildGrandStrategyDrama(const StrategistJourney&,JourneyEpisode&);
    JourneyEpisode BuildGrandBattleEpisode(const StrategistJourney& journey)
    {
        JourneyEpisode episode;
        const bool yangren=journey.HasSunJian();
        const int commander=yangren ? SunJian:HuangfuSong;
        episode.title=yangren ? U"第二章　陽人大決戦":U"第一章　広宗大決戦";
        episode.place=yangren ? U"一九一年・陽人の戦場":U"一八四年・広宗の戦場";
        episode.setting=yangren ? U"孫堅軍が山道を出る。正面には呂布の騎兵、遠くには董卓軍の本陣がある。":
            U"官軍と黄巾軍が三つの戦線で向かい合う。中央の旗を合図に、全軍の総攻撃が始まる。";
        episode.question=journey.ObjectiveBrief();
        if(journey.stage==JourneyStage::Preparation)
        {
            if(!yangren)
                episode.lines.push_back({ZhangLiang,U"兄者の旗を見ろ！ 俺たちは、あの旗の下でやっと一つになれた。今日ここで、ばらばらにされてたまるか！",
                    U"黄いろい旗が、北陣から南陣まで次々と上がる。"});
            else
                episode.lines.push_back({LuBu,U"正面へ来るなら受けてやる。だが旗が揺れたくらいで逃げるなよ。俺はまだ、馬を走らせてもいない。",
                    U"董卓軍の前面。騎兵が一斉に槍を立てる。"});
            episode.lines.push_back({commander,yangren ?
                U"俺の本隊は南道を抜ける。左と右にも兵を出す。軍師、辻を開け。合流したら、そのまま敵の本陣まで押し込むぞ！":
                U"左翼は北陣を、右翼は南陣を押さえる。私の本隊は中央で待つ。前陣にそなたらの旗が立ったら、全軍を動かす。",
                U"将たちは地図を囲む。主人公の受け持つ道が、本隊と両翼をつないでいる。"});
            episode.lines.push_back({0,U"旗を立てた先で、俺たちを待つ兵がいる。そこまで道をつなごう。軍師、お前の合図は俺たちが守る。",
                U"劉備は主人公の隣へ立ち、全軍の配置を見渡す。"});
            episode.lines.push_back({2,U"北も南も、どえらい数だな！ ……で、俺はどこをぶち抜きゃ、みんなが前へ出られる？",
                U"張飛が身を乗り出す。関羽は敵本陣の手前へ鞘の先を置く。"});
            episode.lines.push_back({1,U"急ぐな、翼徳。前陣を取る者と、取った道を支える者が要る。軍師、私は後に続く兵まで見て動く。",
                U"陣の外から、左右の部隊が出発する太鼓が響く。"});
            episode.lines.push_back({3,yangren ?
                U"辻を開いたら、孫堅殿へ合図を送る。呂布を追って皆をばらばらにするな。本隊と両翼を生かして、あの本陣を落とす。":
                U"前陣を取ったら、俺が合図を出す。そこからが総攻撃だ。関羽、張飛、目の前だけでなく左右の味方も見てくれ。",
                U"主人公が軍旗を受け取る。戦場のあちこちで、同じ時を待つ旗が揺れている。"});
        }
        else if(journey.stage==JourneyStage::Complete && !yangren && journey.won)
            episode.lines={
                {0,U"広宗で見た兵の顔を、忘れずにいたい。旗が替わっても、明日から食べていかなければならない人たちだ。",U"黄巾の乱の後。劉備の前に、安喜へ向かう支度が並ぶ。"},
                {2,U"あの戦場を越えたと思ったら、今度は役所か。兄者、俺に判子は持たせるなよ。",U"張飛は笑い、槍を肩へ戻す。"},
                {3,U"次に旗を上げる時も、お前たちと軍議がしたい。誰を前へ出し、誰を帰すか。その続きを考えたい。",U"やがて都から董卓の名が届き、諸侯が兵を集め始める。"}};
        else if(journey.won)
        {
            const auto& result=journey.history[journey.mission].review.grand;
            const auto leader=std::u32string(EpisodeSpeakerName(result.openedBy));
            episode.lines={
                {-1,yangren ? U"董卓軍の本陣が崩れた。孫堅軍の旗が山道から戦場へ広がり、左右の部隊が呼び合う。":
                    U"黄巾軍の本陣が崩れた。前陣に立った義勇軍の旗の下を、官軍本隊が通り抜けた。",U"三つの戦線を、勝鬨が渡っていく。"},
                {commander,leader+U"の旗を見た時、兵が一斉に前を向いた。軍師、そなたの一手は、ここにいる全軍を動かした。",
                    U"本隊の指揮官が主人公へ向き直る。"},
                {1,result.alliedLosses>result.alliedStarted/3 ? U"勝った。だが、左右の陣にも傷ついた者が多い。軍師、帰りの順まで決めよう。":
                    U"左右の兵も戻ってきた。旗を取るだけでなく、続く者を通せた。それがお前の采配だった。",
                    U"関羽が、実際に帰ってきた兵の報せを受け取る。"},
                {3,U"戻った隊から負傷者を運べ。最後の隊が帰るまで、ここに旗を置く。……皆、俺の合図に応えてくれてありがとう。",
                    U"主人公は兵たちへ頭を下げ、帰陣の合図を出す。"}};
        }
        else
            episode.lines={
                {commander,U"軍師、一隊の前進が全軍の前進になるとは限らぬ。どこで兵が止まり、どこから崩れたか。次はともに見よう。",U"戦場から、本隊と両翼の報告が集まる。"},
                {3,U"俺たちの道だけを見ていた。次は、旗を立てた後に誰が続けるかまで決める。戻った兵から、話を聞かせてくれ。",U"主人公は北陣、中央、南陣の印を地図へ置き直す。"}};
        BuildGrandStrategyDrama(journey,episode);
        return episode;
    }
}
