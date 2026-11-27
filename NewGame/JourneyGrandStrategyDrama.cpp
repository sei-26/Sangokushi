#include "JourneyEpisode.hpp"
#include "StrategistJourney.hpp"

namespace frontline
{
    void BuildGrandStrategyDrama(const StrategistJourney& journey,JourneyEpisode& episode)
    {
        if(!journey.HasGrandStrategy()) return;
        if(journey.stage==JourneyStage::Preparation && episode.lines.size()>=6)
        {
            episode.setting=U"川を挟んだ三つの戦線。北の林で官軍左翼が押され、中央の橋には兵が詰まっている。";
            episode.lines[1]={HuangfuSong,U"北の仲間は、もう余力がない。私が北へ回れば支えられるが、中央の攻めは遅れる。橋を渡って先に本陣へ迫る手もある。軍師、どちらへ本隊を出す。",
                U"皇甫嵩が本隊の駒を、北の林と中央の橋の間へ置く。"};
            episode.lines[4]={1,U"私なら北へ向かえる。だが橋へ出す兵は減る。翼徳、先へ行く時は、後ろの旗を一度だけ見てくれ。",
                U"関羽は北の林道を、張飛は中央の橋を見つめる。"};
            episode.lines[5]={3,U"救いに行くなら、北陣を崩して帰ってくる。本隊を中央へ出すなら、その時間で前陣を取る。……出陣したら、俺が方針を伝えます。",
                U"主人公の手元に、二本の進路を記した旗が残る。遠くから、北の太鼓が届いた。"};
            return;
        }
        if(!journey.won || episode.lines.size()<3) return;
        const auto& grand=journey.history[YellowTurbanMission].review.grand;
        if(journey.stage!=JourneyStage::Complete && grand.approach==GrandApproach::CentralAssault && grand.forcedSteps>0)
            episode.lines[1]={HuangfuSong,U"軍師、君が橋の先を開けたから、本隊の旗がここまで届いた。前陣を取って終わらず、最後まで道を残した。あの采配を、兵たちも覚えている。",
                U"皇甫嵩が、中央を通った兵をねぎらう。戻った主力の兵と残る兵糧は、次の戦いへ引き継がれる。"};
        if(journey.stage==JourneyStage::Complete)
        {
            if(grand.relievedOn && grand.leftReturned>0)
                episode.lines.push_back({grand.reliefOfficer,grand.reliefOfficer==1 ? U"北でともに戦った兵が、この道を教えてくれた。軍師、私が先に立とう。今度は彼らの志に応える番だ。":grand.reliefOfficer==2 ?
                    U"おい軍師！ 広宗の連中が、今度は俺に道を教えるってよ。へへ、あの時駆けつけた甲斐があったな！":
                    U"北の兵が、また同じ旗で戦いたいと言っていた。次に出る時は、あの声も連れて行こう。",
                    U"数年後。反董卓の軍議にも、広宗で道をつないだ記憶が残る。"});
            return;
        }
        if(grand.relievedOn && grand.leftReturned>0)
            episode.lines[2]={grand.reliefOfficer,grand.reliefOfficer==1 ? U"北の旗も戻ったか。……軍師、よく決めた。あそこで待っていた兵がいたから、私も道をつなげた。":grand.reliefOfficer==2 ?
                U"おう、北の連中も戻ったぞ！ おい、あの旗を立てた奴を呼んでくれ。今日は俺の隣で飯だ！":
                U"北の旗も戻った。待っていてくれた兵に、今度は俺たちから声をかけよう。あの道は、片方だけではつながらなかった。",
                U"左翼を救援した隊と、生還した官軍の兵が同じ焚き火を囲む。"};
        else if(grand.leftReturned==0)
            episode.lines[2]={1,grand.relievedOn ? U"一度は北へ道をつないだ。だが、左翼は帰らなかった。軍師、そこから先に何があったか、兵の話を聞こう。":
                U"本陣は落ちた。……北の旗が、まだ戻らぬ。軍師、勝ったことも、あそこへ兵を戻せなかったことも、覚えておこう。",
                U"勝鬨が静まる。関羽のそばには、戻らなかった左翼の印が置かれている。"};
        else
            episode.lines[2]={1,U"北の仲間も持ちこたえた。中央の勝ちを、あの兵たちも待っていたのだな。軍師、労いを届けよう。",
                U"左翼の兵が傷を巻きながら、中央から戻る旗を迎える。"};
    }
}
