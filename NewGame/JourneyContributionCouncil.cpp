#include "JourneyCouncilStage.hpp"
#include "JourneyPlanExample.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    CouncilExchange ContributionCouncil(const StrategistJourney&,const JourneyPlanExample& p)
    {
        CouncilExchange e;if(!p.remembersCooperation) return e;
        e.subject=p.name;
        const bool zhang=p.adviser==2;
        e.beats.push_back({{p.adviser,p.offer,U"前戦で動いた駒を、今度は支える場所へ置いた。",1<<p.adviser},{}});
        e.beats.back().reactions[0]=U"その役を引き受けてくれるか";
        e.beats.push_back({{zhang ? 1:0,zhang ? U"守りを預かるなら、出る時も軍師の合図を待て。私も、今の地図に合わせて動こう。":
            U"雲長がそこを引き受けるなら、私は動けるね。軍師、私をどこへ走らせる？",U"仲間が、空いた駒の行き先を主人公へ尋ねた。",zhang ? 2:1},{}});
        e.beats.push_back({{3,U"その案、地図で見せてくれ。誰が残り、誰が動けるのか。確かめてから、俺が役目を頼む。",U"主人公は、まだ命令を出さずに二つの配置を見比べる。",0},{}});
        e.beats.push_back({{p.adviser,zhang ? U"おう！ 残れって言うなら、残る理由も任せろ。攻める時は、でかい声で呼べよ！":
            U"承知した。留まるのも進むのも、君の策だ。採るかどうか、今の戦場を見て決めよ。",U"提案は、あなたが採用するまで今の担当と分けて置かれる。",1<<p.adviser},{}});
        return e;
    }
}
