#include "StrategistJourney.hpp"
#include "JourneyBattlefield.hpp"
#include "HexGrid.hpp"
#include <queue>
namespace frontline
{
    int StrategistJourney::TurnBudget() const
    {
        return (IsRescue() ? 10:Definition().limit)+CombatSettings(difficulty).extraCycles+
            (campaignFlow && mission==1 && history[0].rescued ? 2:0)+(PlansGrandBattle() ? 8:0)+(HasGrandStrategy() ? 6:0);
    }
	JourneySupplies StrategistJourney::PreparationCosts(int choice) const
	{
		const bool escort = Definition().kind == JourneyObjective::Escort;
        const int setupGold=campaignFlow ? 0:(choice == 0 ? 200:80);
		return {(choice == 0 ? 3000 : 4800) + (HasConvoy() ? 900 : 0), setupGold+(IsRescue() ? ConvoyPlan(convoyRoute).guideGold : 0),
		        escort ? 2000 : 0, HasConvoy() ? 600 : 0};
	}

    int StrategistJourney::PreparationMorale(int choice,int officer) const
    {
        const auto& previous=history[YellowTurbanMission];
        const auto& grand=previous.review.grand;
        const int thanks=mission==CoalitionMission && previous.won && grand.relievedOn && grand.leftReturned>0 &&
            grand.reliefOfficer==officer ? 10:0;
        return std::clamp((!campaignFlow && choice==1 ? 100:90)+StoryMoraleBonus(officer)+thanks,0,100);
    }
    std::array<int,3> StrategistJourney::PreparationPositions(int choice) const
    {
        const auto& authored=Definition().positions;
        std::array<int,3> result{authored[0],authored[1],authored[2]};
        if(campaignFlow && mission==YellowTurbanMission)
        {
            result[0]=Campaign::At(9,8);
            result[2]=Campaign::At(10,10);
        }
        if(HasGrandStrategy()) result={Campaign::At(6,8),Campaign::At(6,5),Campaign::At(7,10)};
        if(HasYangrenStrategy() && result[1]==Campaign::At(7,7)) result[1]=Campaign::At(7,6);
        if(mission==CoalitionMission && !legacyBattlefield)
        {
            const auto& support=GuangzongSupport();
            const bool maintain=coalitionPlan==CoalitionPlan::Protect;
            if(support.swift) result[2]=Campaign::At(maintain ? 8:10,maintain ? 9:7);
            if(support.roadOfficer>=0) result[1]=Campaign::At(maintain ? 9:10,maintain ? 7:5);
            const auto& previous=history[YellowTurbanMission];const auto& grand=previous.review.grand;
            if(previous.won && grand.relievedOn && grand.leftReturned>0 && grand.reliefOfficer>=0)
                result[grand.reliefOfficer]=Campaign::At(maintain ? 8:10,maintain ? 7:5);
        }
        if(HasCoalitionBattle())
        {
            result={Campaign::At(5,6),Campaign::At(5,8),Campaign::At(6,9)};
            if(GuangzongSupport().swift) result[2]=Campaign::At(6,7);
            const auto& g=history[YellowTurbanMission].review.grand;
            if(history[YellowTurbanMission].won && g.relievedOn && g.leftReturned>0 && g.reliefOfficer>=0)
                result[g.reliefOfficer]=Campaign::At(6,4);
        }
        if(!campaignFlow || choice!=0 || assignments[1].duty!=JourneyDuty::Ambush) return result;
        const int destination=assignments[1].tile;
        if(!OnMap(destination) || destination==result[0] || destination==result[2] ||
           PlanningTerrain(destination)!=Terrain::Forest) return result;
        // Two public-terrain steps, never enemy positions/policy or the previous battlefield.
        std::array<int,Width*Height> distance;distance.fill(-1);
        std::queue<int> pending;pending.push(result[1]);distance[result[1]]=0;
        while(!pending.empty())
        {
            const int from=pending.front();pending.pop();
            if(from==destination) { result[1]=destination;break; }
            if(distance[from]>=2) continue;
            for(const auto& xy:hexgrid::Neighbors(from%Width,from/Width))
            {
                if(xy.first<1 || xy.first>14 || xy.second<1 || xy.second>12) continue;
                const int next=Campaign::At(xy.first,xy.second);
                const auto terrain=PlanningTerrain(next);
                if(distance[next]>=0 || terrain==Terrain::River || terrain==Terrain::Sea ||
                   next==Campaign::At(14,3) || next==Campaign::At(2,mission==2 ? 6:10)) continue;
                distance[next]=distance[from]+1;pending.push(next);
            }
        }
        return result;
    }
    bool StrategistJourney::CanResupply() const
    {
        if(stage!=JourneyStage::Battle || !campaignFlow || preparation!=1 || reserveFood!=1800) return false;
        for(int i=0;i<3;++i) if(TacticChain::Living(world,i)) return true;
        return false;
    }
    bool StrategistJourney::Resupply()
    {
        if(!CanResupply()) return false;
        int living=0;for(int i=0;i<3;++i) if(TacticChain::Living(world,i)) ++living;
        const int portion=reserveFood/living;
        battleReview.preparationTracked=true;battleReview.resupplyTurn=turn;
        for(int i=0;i<3;++i) if(TacticChain::Living(world,i)) battleReview.resupplyMorale+=std::min(20,100-world.armies[i].morale);
        for(int i=0;i<3;++i) if(TacticChain::Living(world,i))
        { world.armies[i].food+=portion;world.armies[i].morale=std::min(100,world.armies[i].morale+20); }
        reserveFood=0;++world.revision;
        // No extra action, resurrection, signature reset, objective tick or hidden observation.
        Report(U"主人公「今だ、予備の糧を回せ！ ここから、もう一度つなぐ！」予備糧1,800を生存する主力で分配、士気＋20（上限100）。行動権・固有戦法の回数は戻らない。補給は使い切った。");
        return true;
    }
}
