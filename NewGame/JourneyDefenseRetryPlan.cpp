#include "JourneyDefenseAdvice.hpp"
#include "JourneyPlanExample.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    void AdaptDefenseRetryPlanExample(const StrategistJourney& j,JourneyPlanExample& p)
    {
        if(!j.campaignFlow || j.stage!=JourneyStage::Preparation || j.mission!=2 ||
           j.Definition().kind!=JourneyObjective::HoldPositions || !j.retryDefense || !p.warning.empty()) return;
        const auto& r=*j.retryDefense;
        if(r.mission!=j.mission || r.failedGuards<=0 || r.failedGuards>7 || r.endedAt<=0 || r.endedAt>16 ||
           (r.resolution!=JourneyResolution::Withdrawn && r.resolution!=JourneyResolution::ForceLost && r.resolution!=JourneyResolution::Deadline)) return;
        int free=0;while(!(r.failedGuards&(1<<free))) ++free;
        const std::u32string names[]={U"劉備",U"関羽",U"張飛"};int guards[2],count=0;
        for(int a=0;a<3;++a) if(a!=free) guards[count++]=a;
        p.adviser=free;p.remembersDefenseRetry=true;p.remembersCooperation=false;p.assignments={};
        p.assignments[guards[0]]={JourneyDuty::Hold,j.Definition().goal};
        p.assignments[guards[1]]={JourneyDuty::Hold,j.Definition().secondGoal};
        const char32_t* offers[]={
            U"私にも、手が足りず火の前で構えられない時があった。今度は私を動ける役にして、二人へ守りを預けないか。",
            U"前の試みでは、手を使い、火の前で構えられぬ時があった。今回は私を援護に残し、両翼の守りと役を分けよう。",
            U"前は手を使い切って、火の前で踏ん張れねえ時があった。今度は俺を動く役にしようぜ！ 守る奴とは役を分けるんだ。"};
        p.offer=offers[free];p.name=U"再挑戦 ／ "+names[free]+U"を援護へ、両翼の手を残す";
        p.reason=names[free]+U"「"+p.offer+U"」";
        p.purpose=names[guards[0]]+U"を北、"+names[guards[1]]+U"を南の烽火へ。"+names[free]+U"は固定せず、自分で動かす援護役として残す。";
        p.manualTask=names[free]+U"には自分で移動・援護・戦法を指示する。守備隊の到着と行動回復を待ち、敵が寄る側へ余力を回す。";
        p.manualTask+=j.RequiredHoldTurns()==2 ? U" 両烽火を同時に２指揮維持する。":U" 両烽火を同時に３指揮維持する。";
        p.caution=U"援護役に自動の攻撃指示は出さない。到着には距離と移動権が要る。守備隊まで戦法に参加させれば、今の固守を使えなくなる。役を変えても守備成功は保証されない。";
        if(r.failedGuards&(r.failedGuards-1)) p.caution+=U" 前の試みで余力不足があった他の隊も、守備では行動を残す必要がある。";
        p.caution+=U" 先遣伏兵を選んだ関羽も、移動すれば潜伏が解ける。";
    }
}
