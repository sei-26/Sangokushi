#include "StrategistJourney.hpp"
#include "JourneyPlanExample.hpp"
#include "JourneyBattlefield.hpp"
#include <limits>
namespace frontline
{
    namespace
    {
        const JourneyReliefRecord* CheckedRelief(const JourneyBattleReview& review)
        {
            if(!review.reliefTracked) return nullptr;
            for(auto it=review.relief.rbegin();it!=review.relief.rend();++it)
                if(it->departedTurn && it->arrivedTurn && it->checkedTurn && !ReliefPending(*it) && it->outcome!=JourneyReliefOutcome::Unconfirmed) return &*it;
            return nullptr;
        }
        std::u32string Number(int v) { const auto s=std::to_string(v);return {s.begin(),s.end()}; }
        std::u32string Place(int tile) { return U"("+Number(tile%Width)+U","+Number(tile/Width)+U")"; }
        bool Ground(const StrategistJourney& j,int tile)
        {
            if(!StrategistJourney::OnMap(tile) || tile==Campaign::At(2,j.mission==2 ? 6:10) || tile==Campaign::At(14,3)) return false;
            const auto terrain=BattlefieldTerrain(j.mission,tile%Width,tile/Width,j.IsRescue());
            return terrain!=Terrain::River && terrain!=Terrain::Sea;
        }
        int WaitingPlace(const StrategistJourney& j,int incoming,int source,int other)
        {
            // Public terrain only. Never plan the successor's approach through the occupied guard post.
            std::array<int,TileCount> distance;distance.fill(-1);
            const int start=j.Definition().positions[incoming];if(!Ground(j,start)) return -1;
            std::vector<int> queue{start};distance[start]=0;
            for(std::size_t i=0;i<queue.size();++i)
                for(const auto& xy:hexgrid::Neighbors(queue[i]%Width,queue[i]/Width))
                {
                    const int tile=Campaign::At(xy.first,xy.second);
                    if(Ground(j,tile) && tile!=source && distance[tile]<0) { distance[tile]=distance[queue[i]]+1;queue.push_back(tile); }
                }
            int best=-1,steps=std::numeric_limits<int>::max();
            for(const auto& xy:hexgrid::Neighbors(source%Width,source/Width))
            {
                const int tile=Campaign::At(xy.first,xy.second);
                if(Ground(j,tile) && tile!=other && distance[tile]>=0 && distance[tile]<steps) { best=tile;steps=distance[tile]; }
            }
            return best;
        }
    }
    bool StrategistJourney::RetryAtCouncil(const StrategistJourney& beforeBattle)
    {
        if(stage!=JourneyStage::Debrief || won || beforeBattle.stage!=JourneyStage::Preparation || beforeBattle.mission!=mission) return false;
        std::optional<JourneyReliefRecord> lesson;
        if(campaignFlow && beforeBattle.campaignFlow) if(const auto* r=CheckedRelief(battleReview)) lesson=*r;
        *this=beforeBattle;retryRelief=lesson;
        return true;
    }
    void AdaptReliefPlanExample(const StrategistJourney& j,JourneyPlanExample& p)
    {
        if(!j.campaignFlow || j.stage!=JourneyStage::Preparation) return;
        const bool retry=j.retryRelief.has_value();const JourneyReliefRecord* record=retry ? &*j.retryRelief:nullptr;
        if(!record && j.mission>0)
        {
            const auto& previous=j.history[j.mission-1];
            if(previous.completed && previous.won) record=CheckedRelief(previous.review);
        }
        if(!record) return;
        const auto& r=*record;
        // A clean handover needs no corrective proposal; preserve the existing successful-preparation advice.
        if(r.outcome!=JourneyReliefOutcome::Recovered && r.outcome!=JourneyReliefOutcome::Lost && !(r.outcome==JourneyReliefOutcome::Held && !r.allSecured)) return;
        const int out=retry ? r.outgoing:j.mission==2 ? 2:0,in=retry ? r.incoming:j.mission==2 ? 0:1;
        const int source=retry ? r.source:j.mission==2 ? j.Definition().secondGoal:Campaign::At(10,j.CurrentPledge()==JourneyPledge::Preserve ? 8:10);
        const bool beacon=j.Definition().kind==JourneyObjective::HoldPositions && (source==j.Definition().goal || source==j.Definition().secondGoal);
        if(j.Definition().kind==JourneyObjective::HoldPositions && !beacon) return; // Three officers cannot also cover a fourth guard/waiting role.
        const int other=beacon ? (source==j.Definition().goal ? j.Definition().secondGoal:j.Definition().goal):-1;
        const int waiting=WaitingPlace(j,in,source,other);if(waiting<0 || !Ground(j,source)) return;
        const int third=3-out-in;const std::u32string names[]={U"劉備",U"関羽",U"張飛"};
        p.assignments={};p.assignments[out]={JourneyDuty::Hold,source};p.assignments[in]={JourneyDuty::Hold,waiting};
        if(other>=0) p.assignments[third]={JourneyDuty::Hold,other};
        p.adviser=r.incoming;
        p.name=retry ? U"再挑戦 ／ 後任を隣接待機":U"次の戦場 ／ 受け渡しの足場を作る";
        p.reason=(retry ? std::u32string(U"前回の試み："):std::u32string(U"前戦："))+names[r.outgoing]+U"→"+names[r.incoming]+U"。";
        if(r.outcome==JourneyReliefOutcome::Recovered) p.reason+=Number(r.gapCycles)+U"回の守備の空白後に立て直した。";
        else if(r.outcome==JourneyReliefOutcome::Lost) p.reason+=U"後任は到着したが、次の更新で守備条件を満たせなかった。";
        else p.reason+=U"受け渡した地点は守れたが、もう一方の烽火は条件を満たさなかった。";
        p.reason+=names[r.incoming]+(r.incoming==0 ? U"「次は、引き継ぐ前の居場所から決めよう。」":r.incoming==1 ? U"「次は、受け渡す前の足場を定めよ。」":U"「次は、先に隣で待つ役を決めようぜ。」");
        if(!retry) p.reason+=U" 戦場が変わるので、前の座標は流用しない。";
        p.purpose=names[out]+U"を守備地点"+Place(source)+U"へ、"+names[in]+U"を隣接する待機地点"+Place(waiting)+U"へ。";
        p.purpose+=other>=0 ? names[third]+U"はもう一方の烽火"+Place(other)+U"を担当。":names[third]+U"は自由指揮で、攻める場所と時機を決める。";
        p.manualTask=U"出陣後、後任の到着と移動権の回復を待ち、交代を検討する。継続保持が必要なら出る隊を残す。交代は自動実行せず、配置を比較して自分で合図する。";
        p.warning=U"交代は最短2周期・空白1周期（同時進行）。\n継続保持を優先するなら、守備隊を残す。";
        p.caution=U"待機地点にも固守を指示するため行動権を使い、攻撃へ回す兵力が減る。同時進行の交代は隣接待機でも最短2周期・空白1周期。到着周期は固守できない。兵150・士気25と敵の接近を確かめる。";
        if(other>=0) p.caution+=U" 両烽火を保つ条件を優先し、待機隊だけ先走らせない。";
    }
}
