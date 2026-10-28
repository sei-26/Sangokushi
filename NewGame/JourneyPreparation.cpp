#include "StrategistJourney.hpp"
#include "JourneyBattlefield.hpp"
#include "HexGrid.hpp"
#include <queue>
namespace frontline
{
	JourneySupplies StrategistJourney::PreparationCosts(int choice) const
	{
		const bool escort = Definition().kind == JourneyObjective::Escort;
		return {(choice == 0 ? 3000 : 4800) + (HasConvoy() ? 900 : 0), (choice == 0 ? 200 : 80)+(IsRescue() ? ConvoyPlan(convoyRoute).guideGold : 0),
		        escort ? 2000 : 0, HasConvoy() ? 600 : 0};
	}

    int StrategistJourney::PreparationMorale(int choice,int officer) const
    { return std::clamp((!campaignFlow && choice==1 ? 100:90)+StoryMoraleBonus(officer),0,100); }
    std::array<int,3> StrategistJourney::PreparationPositions(int choice) const
    {
        const auto& authored=Definition().positions;
        std::array<int,3> result{authored[0],authored[1],authored[2]};
        if(!campaignFlow || choice!=0 || assignments[1].duty!=JourneyDuty::Ambush) return result;
        const int destination=assignments[1].tile;
        if(!OnMap(destination) || destination==result[0] || destination==result[2] ||
           BattlefieldTerrain(mission,destination%Width,destination/Width,IsRescue())!=Terrain::Forest) return result;
        // Two public-terrain steps, never enemy positions/policy or the previous battlefield.
        std::array<int,Width*Height> distance;distance.fill(-1);
        std::queue<int> pending;pending.push(authored[1]);distance[authored[1]]=0;
        while(!pending.empty())
        {
            const int from=pending.front();pending.pop();
            if(from==destination) { result[1]=destination;break; }
            if(distance[from]>=2) continue;
            for(const auto& xy:hexgrid::Neighbors(from%Width,from/Width))
            {
                if(xy.first<1 || xy.first>14 || xy.second<1 || xy.second>12) continue;
                const int next=Campaign::At(xy.first,xy.second);
                const auto terrain=BattlefieldTerrain(mission,xy.first,xy.second,IsRescue());
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
        for(int i=0;i<3;++i) if(TacticChain::Living(world,i))
        { world.armies[i].food+=portion;world.armies[i].morale=std::min(100,world.armies[i].morale+20); }
        reserveFood=0;++world.revision;
        // No extra action, resurrection, signature reset, objective tick or hidden observation.
        Report(U"主人公「今だ、予備の糧を回せ！ ここから、もう一度つなぐ！」予備糧1,800を生存する主力で分配、士気＋20（上限100）。行動権・固有戦法の回数は戻らない。補給は使い切った。");
        return true;
    }
}
