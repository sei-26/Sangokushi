#include "JourneyAutoBattle.hpp"
namespace frontline
{
    void CoordinateJourneyAutoMovement(const StrategistJourney&,JourneyAutoPlan&);
    void CoordinateJourneyAutoEngagement(const StrategistJourney&,JourneyAutoPlan&,const std::array<int,3>&);
    std::vector<ChainOrder> PlanJourneyAutoTactics(const StrategistJourney&,const std::array<bool,3>&);
    JourneyAutoPlan PlanJourneyAutoBattle(const StrategistJourney& j)
    {
        JourneyAutoPlan plan;
        if(j.stage!=JourneyStage::Battle) return plan;
        const auto& w=j.world;
        const bool capture=j.Definition().kind==JourneyObjective::CaptureCamp && !j.depotInterception;
        int intercept=-1;
        if(j.depotInterception)
        {
            const auto* seen=j.intelligence[0].Find(4);
            intercept=seen && seen->current ? seen->observed.tile+1:DepotColumnStart-1;
        }
        std::array<int,3> duties{-1,-1,-1};
        auto Assign=[&](int tile)
        {
            int best=-1,score=100000;
            for(int a=0;a<3;++a) if(TacticChain::Living(w,a) && duties[a]<0)
            {
                if(!CanOccupyObjective(w.armies[a])) continue;
                JourneyRealtime approach;
                approach.Queue(j,a,JourneyDirective::March,tile);
                auto route=approach.Route(j,a);
                if(route.empty()) route=approach.ReliefApproach(j,a,tile).route;
                const int distance=route.empty() ? 200+w.MapDistance(w.armies[a].tile,tile):int(route.size())-1;
                const int value=distance*10+(w.armies[a].troops<500 || w.armies[a].morale<40 ? 30:0);
                if(value<score) { score=value;best=a; }
            }
            if(best>=0) duties[best]=tile;
        };
        if(j.Definition().kind==JourneyObjective::HoldPositions)
        { Assign(j.goal);Assign(j.Definition().secondGoal); }
        else if(capture)
        {
            if(j.HasGrandStrategy() && j.battleReview.grand.approach==GrandApproach::RelieveLeft &&
                !j.battleReview.grand.relievedOn && TacticChain::Living(w,6)) Assign(GrandNorthPass);
            Assign(j.goal);
            const auto& guangzong=j.battleReview.guangzong;
            if(j.mission==YellowTurbanMission && guangzong.tracked && guangzong.roadOfficer<0 &&
                (!guangzong.alarmTurn || j.GuangzongCommandsLeft()>0))
                Assign(GuangzongRoad);
        }
        else if(j.depotInterception) Assign(intercept);
        std::array<bool,3> defending{};
        for(int a=0;a<3;++a)
        {
            if(!TacticChain::Living(w,a)) continue;
            const int at=w.armies[a].tile;
            int target=-1,dist=100000;
            for(const auto& c:j.intelligence[0].contacts)
            {
                if(!c.current || c.observed.troops<=0 || !w.Hostile(w.player,c.observed.faction) ||
                   (j.HasGrandStrategy() && c.id!=GrandCommander && c.observed.morale<25) ||
                   (j.HasGrandStrategy() && j.battleReview.grand.approach==GrandApproach::CentralAssault &&
                       !j.battleReview.grand.openedOn && w.MapDistance(c.observed.tile,j.goal)>2) ||
                   (j.depotInterception && c.id==4)) continue;
                int value=w.MapDistance(at,c.observed.tile);
                if(j.HasConvoy())
                {
                    const auto& path=j.ConvoyPath();
                    const int next=path[std::min(j.convoyStep+1,int(path.size())-1)];
                    value+=w.MapDistance(c.observed.tile,next)*2;
                }
                if(value<dist) { dist=value;target=c.id; }
            }
            if(duties[a]>=0)
            {
                if(at==duties[a])
                {
                    plan.movement.Queue(j,a,JourneyDirective::Hold);
                    defending[a]=true;plan.reasons[a]=U"旗を守る。仲間が動ける時間をつくる！";
                }
                else
                {
                    plan.movement.Queue(j,a,JourneyDirective::March,duties[a]);
                    plan.movement.orders[a].arrival=JourneyArrival::Hold;
                    if(plan.movement.Route(j,a).empty() && target>=0) plan.movement.Queue(j,a,JourneyDirective::Attack,target);
                    plan.reasons[a]=U"目標へ進む。道を塞ぐ敵は押し返す！";
                }
            }
            else if(target>=0)
            {
                plan.movement.Queue(j,a,JourneyDirective::Attack,target);
                plan.reasons[a]=j.HasConvoy() ? U"車の前を開ける。追い過ぎず、護衛を助ける！":U"敵を押さえる。仲間の一撃につなぐぞ！";
            }
            else
            {
                int goal=j.depotInterception ? intercept:j.goal;
                if(j.HasConvoy())
                {
                    const auto& path=j.ConvoyPath();
                    goal=path[std::min(j.convoyStep+2,int(path.size())-1)];
                }
                // Find a reachable flank instead of waiting on a friendly-occupied objective/cart road.
                std::vector<int> candidates{goal};
                const auto neighbours=w.MapNeighbors(goal);candidates.insert(candidates.end(),neighbours.begin(),neighbours.end());
                bool queued=false;
                for(int cell:candidates)
                {
                    if(capture && cell==j.goal && !CanOccupyObjective(w.armies[a])) continue;
                    if(j.HasConvoy() && std::find(j.ConvoyPath().begin(),j.ConvoyPath().end(),cell)!=j.ConvoyPath().end()) continue;
                    if(plan.movement.Queue(j,a,JourneyDirective::March,cell) && !plan.movement.Route(j,a).empty()) { queued=true;break; }
                }
                if(!queued) plan.movement.Queue(j,a,JourneyDirective::Hold);
                plan.reasons[a]=j.HasConvoy() ? U"車道を空け、前へ。見えていない敵にも備える！":U"目標へ進み、次の敵を探る！";
            }
        }
        CoordinateJourneyAutoEngagement(j,plan,duties);
        plan.tactics=PlanJourneyAutoTactics(j,defending);
        CoordinateJourneyAutoMovement(j,plan);
        return plan;
    }
}
