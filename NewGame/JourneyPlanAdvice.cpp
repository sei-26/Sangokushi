#include "StrategistJourney.hpp"
#include "JourneyPlanExample.hpp"
namespace frontline
{
    // Suggestions are derived from settled player history and authored public terrain only.
    void AdaptPlanExample(const StrategistJourney& j,JourneyPlanExample& p)
    {
        if(!j.campaignFlow || j.stage!=JourneyStage::Preparation || j.mission<1 || j.mission>2) return;
        const auto& previous=j.history[j.mission-1];const auto& r=previous.review;
        if(!previous.completed || !previous.won || !r.preparationTracked) return;
        const int strength=previous.survivors+previous.casualties;
        const bool costly=previous.story.costly || (strength>0 && previous.casualties*4>=strength);
        if(costly)
        { p.adviser=0;p.reason=U"劉備「前の戦いは、戻らなかった兵も多かった。今度は支える場所を先に決めないか。」"; }
        else if(r.openingAmbushes>0)
        { p.adviser=1;p.reason=U"関羽「初手の伏兵は届いた。だが同じ地図ではない。次は、この森と渡り口を足場にするのはどうだ。」"; }
        else if(r.resupplyTurn>0 && r.postSupplySteps>0)
        { p.adviser=2;p.reason=U"張飛「糧を回したあとも、戦法を使えたな。次も誰を残し、誰を動かすか、俺たちに合図をくれよ。」"; }
        else if(r.resupplyTurn>0)
        { p.adviser=0;p.reason=U"劉備「前は補給した後、戦法は使わなかったね。今回はまず要所を支え、必要な時に次の一手を考えよう。」"; }
        else return; // The saved history does not distinguish an unused reserve from an unused opening setup.
        if(j.mission==1)
        {
            const bool north=j.CurrentPledge()==JourneyPledge::Preserve;
            const int main=Campaign::At(10,north ? 8:10),other=Campaign::At(10,north ? 10:8);
            if(p.adviser==1)
            {
                p.name=U"関羽の案 ／ 森から北橋へ";
                p.assignments={JourneyAssignment{JourneyDuty::Hold,Campaign::At(10,8)},JourneyAssignment{JourneyDuty::Ambush,Campaign::At(9,7)},JourneyAssignment{}};
                p.purpose=U"劉備が北橋の手前を支え、関羽が北の森林で伏兵を準備。張飛の進路と攻撃は自分で指揮する。";
                p.manualTask=U"南側から出る隊は北へ寄せる時間が要る。張飛は北橋(11,8)を経由してから港へ指示。敵を見て戦法を使い、最後は指揮所へ一隊を入れる。";
                p.caution=U"前戦で伏兵が成功しても、今回の敵が寄るとは限らない。斥候・孤立に注意し、待ち続けず別の戦法や橋へ切り替える。";
            }
            else
            {
                p.name=p.adviser==2 ? U"張飛の案 ／ 橋を支え、突破へ":U"劉備の案 ／ 両橋を支える";
                p.assignments={JourneyAssignment{JourneyDuty::Hold,main},JourneyAssignment{JourneyDuty::Hold,other},JourneyAssignment{}};
                p.purpose=U"劉備と関羽を二つの橋の手前へ。張飛は配置を固定せず、渡る橋と仕掛ける時を自分で決める。";
                p.manualTask=U"二人が到着する間も、張飛には自分で指示する。敵を見て渡る橋を選び、固守する隊を攻めに回す時は継続指示を変更する。";
                p.caution=U"守りを分けると主攻の兵力は減る。固守は行動権を使い、戦法を自動発動しない。先遣伏兵を選んでも関羽の行軍で隠蔽は解除される。";
            }
        }
        else
        {
            if(p.adviser==2)
            {
                p.reason=U"張飛「糧を回したあとも、戦法を使えたな。今度は俺が南の烽火を支える。関羽を動ける役に残して、途中の合図をくれよ。」";
                p.name=U"張飛の案 ／ 両翼と遊撃";
                p.assignments={JourneyAssignment{JourneyDuty::Hold,j.Definition().goal},JourneyAssignment{},JourneyAssignment{JourneyDuty::Hold,j.Definition().secondGoal}};
                p.purpose=U"劉備を北、張飛を南の烽火へ。関羽の配置は固定せず、自分で両翼の援護へ動かす。";
                p.manualTask=U"関羽は未指定なので、自分で伏兵・移動・戦法を指示する。両烽火の守備を確かめ、敵が寄った側へ援護を送る。";
                p.caution=U"張飛を攻めに出せば南の守りが空く。関羽の援護が届かない側もあるので、敵の接近と両翼の維持を先に確かめる。";
            }
            else
            {
                p.name=p.adviser==1 ? U"関羽の案 ／ 渡り口を支える":U"劉備の案 ／ 両翼を固める";
                p.assignments={JourneyAssignment{JourneyDuty::Hold,j.Definition().goal},JourneyAssignment{JourneyDuty::Hold,Campaign::At(7,7)},JourneyAssignment{JourneyDuty::Hold,j.Definition().secondGoal}};
                p.purpose=U"劉備と張飛を二つの烽火へ。関羽は西の橋で固守し、両岸をつなぐ渡り口を支える。";
                p.manualTask=U"両翼の配置を先に確かめる。敵の接近を見て、関羽を橋に残すか攻めへ切り替えるか決める。固守のまま戦法は自動で撃たない。";
                p.caution=U"橋に味方が残ると、救援の通り道も塞ぐ。関羽が届かない時や味方を渡す時は、橋から退かす順番を自分で決める。";
            }
            p.manualTask+=j.RequiredHoldTurns()==2 ? U" 両烽火を同時に２指揮維持すれば任務達成。":U" 両烽火を同時に３指揮維持すれば任務達成。";
        }
    }
}
