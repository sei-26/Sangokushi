#include "JourneyCouncilStage.hpp"
#include "JourneyPlanExample.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    CouncilExchange DefenseRetryCouncil(const StrategistJourney&,const JourneyPlanExample& p)
    {
        CouncilExchange e;
        if(!p.remembersDefenseRetry || p.adviser<0 || p.adviser>2 || p.offer.empty() || p.assignments[p.adviser].duty!=JourneyDuty::None) return e;
        const int free=p.adviser;int guard=free==1 ? 2:1;
        if(p.assignments[guard].duty!=JourneyDuty::Hold) return e;
        const char32_t* address[]={U"兄者",U"雲長",U"翼徳"};
        e.subject=p.name;
        e.beats.push_back({{free,p.offer,U"二つの火を守る駒と、自分の駒を分けて置いた。"},{}});
        e.beats.back().reactions[guard]=U"こちらの役も、地図で確かめよう";
        std::u32string reply;
        if(guard==1)
            reply=free==0 ? U"兄者、この案なら、烽火は私たちが預かりましょう。軍師、走らせる役だけでなく、残す二人も見て策を決めよ。":
                U"翼徳、この案なら、火の前は私たちが支える。軍師、その一手をどこへ回すか、両方の火を見て決めよ。";
        else reply=std::u32string(address[free])+U"、この案なら火の前は俺たちだな！ 軍師、援護が要る時は呼ぶぞ。向こうの火も、忘れんなよ！";
        e.beats.push_back({{guard,reply,U"守る役の武将も、今の地図に自分の駒を添えた。"},{}});
        e.beats.push_back({{3,U"守る手まで使わせたのは俺だ。次は走る役と残る役を分けたい。まず、この案を地図で見せてくれ。",
            U"主人公は、まだ命令を出さず、今の担当と提案を見比べようとする。"},{}});
        const char32_t* closing[]={
            U"採るかは君が決めてくれ。私が動くなら、いつ誰を助けるかも一緒に考えよう。二人の守りへ、届く一手にしたい。",
            U"採るなら、援護を送る場所と時も君が指示せよ。守る二人の手を残せるか、そこまで見届けよう。",
            U"よし、今度は動ける役で待つ！ ……採るって決まってからだな。軍師、地図で役目を確かめようぜ！"};
        e.beats.push_back({{free,closing[free],U"案は、あなたが採用するまで命令にならない。",free==2 ? 4:0},{}});
        return e;
    }
}
