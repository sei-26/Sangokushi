#include "JourneyCouncilStage.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    std::u32string CouncilBattleVoice(const StrategistJourney& j,const ChainEvent& e,int step)
    {
        if(!j.campaignFlow || !j.battleReview.preparationTracked || !e.success || e.army<0 || e.army>2) return {};
        if(j.preparation==0 && j.turn==1 && e.army==1 && e.tactic==ChainTactic::Ambush && e.setupPercent==125)
            return U"軍師の仕込みどおりだ。皆、この一撃につなげ！";
        if(j.preparation==1 && j.battleReview.resupplyTurn>0 && step==0 &&
           j.lastChain.completed>0 && j.battleReview.postSupplySteps==j.lastChain.completed)
        {
            const char32_t* voices[]={U"君が残した糧を、ここからの一手につなぐ！",U"糧は届いた。今度は、私の一手で応えよう！",U"軍師、あの糧でまだ行ける！ もう一発だ！"};
            return voices[e.army];
        }
        return {};
    }
    CouncilExchange BuildCouncilExchange(const StrategistJourney& j,int voice,int preparation,CouncilDecision accepted)
    {
        CouncilExchange e;
        if(j.stage!=JourneyStage::Preparation || voice < -1 || voice>2 || preparation<0 || preparation>1) return e;
        auto Say=[&](int speaker,const char32_t* text,const char32_t* direction,
                     const char32_t* liu=U"",const char32_t* guan=U"",const char32_t* zhang=U"")
        { e.beats.push_back({{speaker,text,direction},{liu,guan,zhang}}); };
        const auto& previous=j.history[std::max(0,j.mission-1)];
        const bool costly=j.mission>0 && previous.completed && (previous.story.recorded ? previous.story.costly:
            previous.casualties>0 && previous.casualties*4>=previous.survivors+previous.casualties);
        if(accepted!=CouncilDecision::None)
        {
            if(accepted==CouncilDecision::OpenRoad || accepted==CouncilDecision::Escort)
            {
                if(j.mission!=0 || (accepted==CouncilDecision::Escort)!=j.IsRescue()) return e;
                e.subject=j.IsRescue() ? U"決めた。車が着くまで、俺たちの戦いだ。":U"決めた。まず、この道を開こう。";
                if(j.IsRescue())
                {
                    Say(3,U"車が出口に着くまで引き受ける。敵を倒しても、勝手に追うなよ！",U"主人公が、地図の出口まで一本の線を引いた。",U"ああ、一緒に行こう",U"出口まで、だな",U"任せとけ！");
                    Say(2,U"へっ、車輪が遅けりゃ俺が押してやる！ 追うなって方は……覚えとく！",U"張飛が笑う。関羽は、その肩を一度だけ叩いた。",U"頼もしいね",U"後半を忘れるな");
                }
                else
                {
                    Say(3,U"先鋒を崩して、道を開く。まず俺たちの旗を、あそこまで進めよう！",U"主人公の駒の先へ、三人の視線が集まった。",U"あの先へ進もう",U"道を開くぞ",U"よし、初陣だ！");
                    Say(0,U"その道を通る者が、我らの次をつくる。さあ、二人とも。軍師の策を聞こう。",U"劉備が自分の駒を、主人公へ差し出した。");
                }
                Say(1,U"守るものが決まった。次は、誰がどこで力を出すかだ。",U"軍議の視線が、地図へ戻る。");
                e.beats[0].dialogue.warmMask=5;e.beats[1].dialogue.warmMask=j.IsRescue() ? 7:1;
                return e;
            }
            const int chosen=accepted==CouncilDecision::Ambush ? 0:accepted==CouncilDecision::Reserves ? 1:-1;
            if(chosen<0 || preparation!=chosen) return e;
            if(chosen==0)
            {
                e.subject=U"関羽の先手を、皆の一撃につなぐ。";
                Say(3,U"関羽、先に林へ入ってくれ。張飛は合図で続け。最初の一撃を、二人でつくる！",U"主人公が二つの駒の間へ、指を走らせた。",U"君の一策だ",U"承知した",U"待ってたぜ！");
                Say(2,U"俺の出番もあるんだな！ だったら待つ。……おい関羽、合図より先に片づけんなよ？",U"張飛が関羽の方へ身を乗り出す。",U"二人とも頼むよ",U"誰に言っている");
                Say(1,U"ならば、遅れるなよ。軍師、先遣の場所を決めてくれ。動けば、伏兵は解ける。それだけは覚えておけ。",U"関羽は笑みを収め、森林の印へ駒を置く。",U"地図で決めよう",U"",U"言ったな、兄者！");
            }
            else
            {
                e.subject=U"一撃で終わらなくても、もう一度立つ。";
                Say(3,U"予備の糧を積もう。一度打って、俺の合図で立て直す。張飛、お前の二度目の出番もつくるぞ！",U"主人公が、前線の後ろへ予備の駒を置いた。",U"皆の力をつなごう",U"糧を回す時が肝心だ",U"二度でも三度でも！");
                Say(1,U"糧を積んだだけで、策が成るわけではない。配る時は君が決めろ。私も、その合図を待とう。",U"関羽が張飛の前へ、もう一つの駒を寄せる。",U"ああ、任せよう",U"",U"分かってるって！");
                Say(2,U"よっしゃ！ 最初っから全部ぶん投げねえ。帰ってくる場所があるなら、思いっきり行けるぜ！",U"張飛が拳を開き、主人公へ向けた。",U"頼むよ、翼徳",U"言葉どおりにな");
            }
            Say(0,U"決まったね。これは君の策であり、我ら皆の戦いだ。地図を整えたら、出よう！",U"幕の向こうで、軍旗が一斉に風を受けた。",U"共に行こう",U"支度を整える",U"さあ、行こうぜ！");
            for(auto& beat:e.beats) beat.dialogue.warmMask=5;
            e.beats.back().dialogue.warmMask=7;
            if(chosen==0) e.beats[1].dialogue.warmMask=7;
            else e.beats[1].dialogue.warmMask=1;
            return e;
        }
        if(voice==-1)
        {
            e.subject=j.mission==0 ? U"名もなき軍師、その最初の軍議":j.mission==1 ? U"大河の向こうへ、我らの旗を":U"この一戦で、戦役を動かす";
            if(costly)
            {
                Say(1,U"前の勝利で、戻らぬ者もいた。今日は、その名も一緒に連れて行く。軍師、次の策を聞こう。",U"関羽が地図の端を押さえる。張飛も、今度は遮らない。",U"皆で引き受けよう",U"",U"今度こそ、だ");
                Say(0,U"重い役目だ。だが、一人で背負わなくていい。この場の誰にでも、力を求めてくれ。",U"劉備が三人の駒を、主人公の手元へ寄せた。");
            }
            else if(j.mission==0)
            {
                Say(2,U"軍師！ 先頭は俺で決まりだろ？ でかい旗の奴から、まとめて吹っ飛ばしてやる！",U"張飛が駒を前へ滑らせる。関羽の指が、その行く手を止めた。",U"まずは聞こうか",U"まだ軍議の途中だ");
                Say(1,U"その後に続く兵は、飛んでは来られぬ。私が先に林へ入る。敵を引きつけ、一撃を合わせるのだ。",U"二人の駒が、地図の上で向き合った。",U"君はどう見る？",U"",U"俺にも一撃、残せよ？");
                Say(3,U"二人とも、力を貸してくれ。誰を前へ出し、どうつなぐかは俺が決める。",U"主人公が二つの駒の間へ、自分の駒を置いた。",U"そう、それが君の役だ",U"聞こう",U"おう！ 言ってみろ！");
                Say(1,U"先鋒を崩しても、後詰が控えていよう。最初の一撃に全員を使うか、その次へ一隊残すか。軍師、そこまで読もう。",U"関羽が、敵の旗の後ろにも駒を置く。",U"開いた道を使う隊も必要だ",U"",U"次の奴なら俺が止める！");
            }
            else if(j.mission==1)
            {
                Say(2,U"港を取れば、この川を俺たちが渡るんだな！ 今度は、地図の外まで行こうぜ！",U"張飛の指が、川の向こうの白い余白へ伸びた。",U"旗の届く先へ",U"まずは橋を渡れ");
                Say(1,U"敵を追う隊と、指揮所へ入る隊を分ける。軍師、その最後の一手まで聞かせてくれ。",U"関羽が指揮所へ駒を置いた。",U"その先で本隊と合流だ",U"",U"突破役なら、ここにいるぜ");
                Say(3,U"橋を渡った先で反撃されるかもしれない。張飛、一人で駆け抜けるな。渡る隊を支え、最後に旗を立てる隊も残す。",U"主人公が、橋の出口と指揮所へ別々の駒を置く。",U"帰る道もつなごう",U"それなら渡れる",U"分かった！ 合図してくれよ！");
            }
            else
            {
                Say(0,U"本隊の旗が、山の向こうまで続いている。あの軍勢の勝敗を、我らの二つの火が支える。",U"幕が開く。幾重もの旗が、尾根の先へ消えていった。",U"君に両翼を預ける",U"二つの火を絶やすな",U"でっけえ役目だな！");
                Say(2,U"片方は俺が守る！ でもよ、向こうが危なくなったら、呼べ。声が届かなきゃ火で知らせろ！",U"張飛が、離れた二つの印を見比べる。",U"一人にはしないよ",U"互いを見て戦え");
                Say(1,U"一隊に全員を引きつけられるな。もう一隊が手薄な翼を突けば、二つの火は続かぬ。追う者より、残す者を先に決めよう。",U"関羽が、二つの烽火の間へ予備の駒を置いた。",U"皆が両翼を見て戦おう",U"",U"俺は残るぜ。呼ぶまでな！");
            }
            Say(0,U"一人の豪勇も、皆で合わせる策も、我らにはある。軍師、まず誰の考えを聞きたい？",U"三人が主人公へ向き直る。ここから、あなたの軍議が始まる。");
        }
        else if(voice==0)
        {
            e.subject=U"劉備 ― この旗で、何を成すか";
            if(j.mission==0)
            {
                Say(0,U"道を開く。その先へ、車も送り届ける。似ているが、引き受ける戦いは変わるね。",U"劉備は敵の旗ではなく、季安の車の駒を見た。",U"君が決めてくれ",U"出口の守りも必要だ",U"車だって守ってやるさ");
                Say(2,U"両方やりゃいい！ ……と言いてえけど、追っかけてばかりじゃ駄目なんだろ？",U"張飛が頭をかく。関羽が、珍しくすぐに頷いた。",U"そのとおりだよ",U"分かってきたな");
                Say(0,U"車まで引き受けるなら、護衛と糧を用意しよう。君の言葉を、兵の役目にするんだ。",U"劉備が主人公の返事を待っている。");
            }
            else
            {
                Say(0,j.mission==1 ? U"この港を取れば、集まった者たちを本隊へつなげられる。私たちだけの武名に終わらせたくない。":U"両翼を守れば、本隊が決戦を仕掛けられる。遠くの兵も、君の合図を待っている。",U"劉備は、地図の外へ続く道をなぞった。");
                Say(3,U"なら、この場の三人だけで終わらない策にする。俺たちの勝ち方を、向こうまで届かせよう。",U"主人公が頷く。張飛も、今度は地図の外を見ている。",U"ああ、共に進もう",U"配置へ落とし込もう",U"聞こえるくらい派手にな！");
            }
        }
        else if(voice==1)
        {
            e.subject=U"関羽 ― 最初の一手を、こちらがつくる";
            Say(1,U"私を先に森林へ入れよ。敵が来てから探すのでは遅い。仕込んだ一手を、皆の一撃へつなぐ。",U"関羽が地図の林へ駒を寄せる。張飛も、それを覗き込んだ。",U"先手を取る策だね",U"",U"なら俺は、その次だ！");
            Say(2,U"先に隠れてんのか。ずるいな！ ……いや、敵から見たら、そりゃ怖えな。",U"張飛が楽しそうに笑う。関羽は、わざと答えない。",U"はは、いい組み合わせだ",U"見つからなければ、な");
            Say(1,U"先遣は近くの林までだ。見つかれば、そこから戦う。予備の糧は持てぬ。軍師、どちらに賭ける。",U"地図で場所を確かめてから出られる。採用だけで伏兵の成功は決まらない。");
        }
        else
        {
            e.subject=U"張飛 ― 一度きりで、終わらせるな";
            Say(2,U"俺の一撃で決めてやる！ でもよ、敵が踏ん張ったら、もう一発ぶちかましてえ。後ろに糧、残しとこうぜ。",U"張飛が握った拳を、今度は地図の後ろへ置いた。",U"皆がもう一度動けるように",U"ほう。先まで考えたか");
            Say(1,U"二度目を狙うなら、一度目で孤立するな。糧を回す時まで、私が横を支えよう。",U"関羽が張飛の駒の横へ、自分の駒を添えた。",U"二人をつなぐのは君だ",U"",U"それでこそ、兄者だ！");
            Say(0,U"予備の糧は、一度だけ全軍へ回せる。最初から配りはしない。君がここだと思った時に、合図を出してくれ。",U"劉備が主人公を見る。積む費用も、配る時も、あなたの判断になる。");
        }
        // Authored reactions, never keyword guesses. Losses retain a sober performance.
        if(!costly)
        {
            if(voice==-1)
            {
                for(auto& beat:e.beats) if(beat.dialogue.speaker==0) beat.dialogue.warmMask=1;
                if(j.mission==0) { e.beats[0].dialogue.warmMask=4;e.beats[1].dialogue.warmMask=3;e.beats[2].dialogue.warmMask=5; }
                else if(j.mission==1) e.beats[0].dialogue.warmMask=5;
                else e.beats[1].dialogue.warmMask=4;
            }
            else if(voice==0) { e.beats[0].dialogue.warmMask=1;e.beats[1].dialogue.warmMask=5; }
            else if(voice==1) { e.beats[0].dialogue.warmMask=5;e.beats[1].dialogue.warmMask=7; }
            else { e.beats[0].dialogue.warmMask=7;e.beats[1].dialogue.warmMask=6;e.beats[2].dialogue.warmMask=5; }
        }
        return e;
    }
}
