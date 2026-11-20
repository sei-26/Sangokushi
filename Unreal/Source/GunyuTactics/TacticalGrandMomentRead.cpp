#include "TacticalGrandMoment.h"
#include "JourneyEpisode.hpp"
#include "JourneyMission.hpp"
namespace gunyugrand
{
    FGrandMoment Compare(const FGrandProgress& A,const FGrandProgress& B)
    {
        FGrandMoment R;
        if(!A.bActive || !A.bBattle || !B.bActive || A.Mission!=B.Mission || (!B.bBattle && !B.bWon)) return R;
        R.Mission=B.Mission;R.Tile=B.Goal;
        const bool Coalition=B.Mission==frontline::CoalitionMission,Yangren=B.Mission==frontline::YangrenMission;
        R.Speaker=Coalition ? frontline::CaoCao:Yangren ? frontline::SunJian:frontline::HuangfuSong;
        if(!A.bWon && B.bWon)
        {
            R.Kind=EGrandMoment::Victory;R.bVictory=true;R.Title=TEXT("軍師の采配が、会戦を決めた");
            R.Line=Coalition && B.Returned ? TEXT("曹操「援軍も、俺の隊も渡った。軍師……お前が残した橋だな。」"):
                Yangren ? TEXT("孫堅「騎兵が退く！ 軍師、隣の旗まで守り抜いたぞ！」"):
                Coalition ? TEXT("曹操「見たか、徐栄！ この先陣は、もう孤軍ではない！」"):
                TEXT("皇甫嵩「敵本陣が崩れた！ 全軍、軍師の旗へ勝鬨を上げよ！」");
            R.Reply=TEXT("主人公「帰る兵を迎えろ。今日、並んだ旗を覚えておこう。」");
        }
        else if(!A.Repelled && B.Repelled) { R.Kind=EGrandMoment::Repelled;R.Title=TEXT("騎兵反撃、撃退！");R.Line=TEXT("孫堅「よく踏ん張った！ お前の旗が、最後まで見えていたぞ！」"); }
        else if(!A.Counter && B.Counter) { R.Kind=EGrandMoment::Counter;R.Speaker=frontline::LuBu;R.Title=TEXT("孫堅と合流 ―― 呂布、反撃！");R.Line=TEXT("呂布「旗が二本になっただけだ！ 騎兵、まとめて踏み砕け！」");R.Reply=TEXT("主人公「散るな！ 孫堅殿の隣へ、旗を寄せろ！」"); }
        else if(!A.Returned && B.Returned) { R.Kind=EGrandMoment::Return;R.Title=TEXT("連合の旗、西岸へ帰還");R.Line=TEXT("曹操「援軍が渡った。最後は俺だ。軍師、橋の向こうで会おう！」"); }
        else if((!A.Rescued && B.Rescued) || (!A.Relieved && B.Relieved))
        {
            R.Kind=EGrandMoment::Relief;R.FocusArmy=6;R.Title=Coalition ? TEXT("南の包囲を突破 ―― 援軍へ届いた！"):TEXT("北の救援、成功！");
            R.Line=Coalition ? TEXT("曹操「その旗は義勇軍か！ 張邈の兵を頼む。俺たちも、後に続く！」"):
                TEXT("皇甫嵩「北の旗は残った！ 道がつながったぞ、全軍、前へ！」");
            R.Reply=Coalition ? (B.bRetreat ? TEXT("主人公「援軍を先に通す！ 橋の出口を空けて迎えろ！」"):
                TEXT("主人公「南の旗はつながった！ 曹操殿と、対岸を押せ！」")):TEXT("主人公「北の仲間を一人にするな。ここから、全軍で押す！」");
            if(Coalition && !B.bRetreat && !A.Rallied && B.Rallied)
            {
                R.Kind=EGrandMoment::Rally;R.FocusArmy=5;R.Title=TEXT("援軍救援 ―― 曹操と共同攻勢！");
                R.Line=TEXT("曹操「南の包囲も開いたか！ 軍師、俺が北を回る。正面はお前たちに任せた！」");
            }
        }
        else if(!A.Rallied && B.Rallied)
        {
            R.Kind=EGrandMoment::Rally;R.Title=TEXT("英傑の旗と、並び立つ");
            R.Line=Coalition ? TEXT("曹操「待っていたぞ、軍師。北は俺が回る。お前の旗は、正面から見せろ！」"):
                Yangren ? TEXT("孫堅「合流したぞ！ 敵が戻ってきても、互いの旗を離すな！」"):
                TEXT("皇甫嵩「本隊、前陣を通過！ 両翼へ総攻撃の太鼓を渡せ！」");
        }
        else if(!A.Opened && B.Opened)
        {
            R.Kind=EGrandMoment::Breach;R.Title=TEXT("前陣突破 ―― 全軍へ合図！");
            R.Line=Coalition ? TEXT("曹操「対岸に味方の旗！ 軍師、ここからは一緒だ！」"):
                Yangren ? TEXT("孫堅「辻の旗が見えた！ 南の道を開け、合流するぞ！」"):
                TEXT("皇甫嵩「義勇軍が前陣を開いた！ 本隊と両翼、あの旗へ続け！」");
        }
        if(R.Kind!=EGrandMoment::None && R.Reply.IsEmpty()) R.Reply=TEXT("主人公「この一手を、全軍の動きに変える。合図を送れ！」");
        return R;
    }
}
