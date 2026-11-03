#include "JourneyPlanExample.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    void AdaptContributionPlanExample(const StrategistJourney& j,JourneyPlanExample& p)
    {
        if(!j.campaignFlow || j.stage!=JourneyStage::Preparation || j.mission<1 || j.mission>2 || !p.warning.empty()) return;
        const auto& h=j.history[j.mission-1];const auto& r=h.review;const int mask=r.coordinatedOfficers;
        const int strength=h.survivors+h.casualties;
        if(!h.completed || !h.won || h.withdrawn || r.endedAt<=0 || mask<0 || mask>7 || !(mask&(mask-1)) ||
           r.tacticAttempts<=0 || r.tacticSteps<2 || r.longestChain<2 || h.story.costly ||
           (strength>0 && h.casualties*4>=strength) || (h.story.recorded && h.story.pledge!=JourneyPledge::None && !h.story.pledgeKept)) return;
        p.adviser=mask&4 ? 2:1;p.remembersCooperation=true;p.assignments={};
        const bool zhang=p.adviser==2;
        if(j.mission==1)
        {
            const bool north=j.CurrentPledge()==JourneyPledge::Preserve;
            const int bridge=Campaign::At(10,north ? 8:10);
            p.assignments[p.adviser]={JourneyDuty::Hold,bridge};
            if(zhang) p.assignments[1]={JourneyDuty::Ambush,Campaign::At(9,north ? 8:9)};
            p.name=zhang ? U"張飛の志願 ／ 橋の支えは俺がやる":U"関羽の志願 ／ 橋を預かり、進む隊を残す";
            p.offer=zhang ? U"前は俺も一手つないだぜ。今度は橋の手前を俺が支える。兄者を動ける役にして、最後の旗までつなごうぜ！":
                U"前戦では私も連鎖を担った。今度は橋の手前を預かろう。兄者と翼徳を、先へ出せる役に残すのだ。";
            p.purpose=zhang ? U"張飛が渡る橋の手前を守備し、関羽は近くの森林で伏兵を準備。劉備は固定せず、自分で渡河・援護・占領を指揮する。":
                U"関羽が渡る橋の手前を守備。劉備と張飛は固定せず、渡河と指揮所へ入る役を自分で指揮する。";
            p.manualTask=U"守備隊の到着を待ちながら、自由な隊へ渡る橋と占領の指示を出す。敵を退けるだけでは終わらず、指揮所へ一隊を入れる。";
            p.caution=zhang ? U"張飛の守備は行動権を使い、突破を自動発動しない。攻めに戻す時は守備の継続指示を変える。劉備にも自分で指示が必要。":
                U"関羽の継続担当は守備。先遣伏兵を選んでも、守備へ進むと潜伏は解ける。固守は行動権を使い、二隊の渡河・攻撃・占領には自分で指示が必要。";
        }
        else
        {
            p.assignments[zhang ? 0:1]={JourneyDuty::Hold,j.Definition().goal};
            p.assignments[2]={JourneyDuty::Hold,j.Definition().secondGoal};
            p.name=zhang ? U"張飛の志願 ／ 南の火を預けろ":U"関羽の志願 ／ 北の火を預かる";
            p.offer=zhang ? U"前の戦いじゃ俺も一手つないだ。今度は南の火を預けろ！ 関羽を動ける役にして、俺たちの間をつないでくれ、軍師！":
                U"前戦では私も一手を担った。今度は北の火を預かろう。兄者を動ける役に残せば、両翼へ声を届けられる。";
            p.purpose=zhang ? U"劉備を北、張飛を南の烽火へ。関羽は固定せず、自分で両翼の援護と交代の時機を指揮する。":
                U"関羽を北、張飛を南の烽火へ。劉備は固定せず、自分で両翼の援護と交代の時機を指揮する。";
            p.manualTask=zhang ? U"関羽は未指定なので、自分で移動・伏兵準備・戦法を指示する。両翼が整うまでは守備隊を残す。":
                U"劉備は未指定なので、自分で移動・援護・交代を指示する。両翼が整うまでは関羽と張飛を残す。";
            p.manualTask+=j.RequiredHoldTurns()==2 ? U" 両烽火を同時に２指揮維持する。":U" 両烽火を同時に３指揮維持する。";
            p.caution=zhang ? U"関羽に継続指示は出さない。先遣伏兵の仕込みも、関羽を動かすと解ける。自由な隊の援護には距離と行動権が要り、張飛を出せば南の守りが空く。":
                U"関羽の継続担当は北の守備。先遣伏兵を選んでも、行軍で潜伏は解ける。固守と劉備の援護には行動権が要り、到着だけで守備成功にはならない。";
        }
        p.reason=std::u32string(zhang ? U"張飛「":U"関羽「")+p.offer+U"」";
    }
}
