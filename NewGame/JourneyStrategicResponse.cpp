#include "JourneyStrategicResponse.hpp"
#include <algorithm>
#include <memory>
namespace frontline
{
    void ApplyGuangzongFlankResponse(const Campaign& seen,const TacticChain& chain,int goal,std::vector<JourneyEnemyPlan>& plans)
    {
        if(seen.armies.size()<GrandBattleArmyCount) return;
        int central=0;bool northCovered=false;
        for(int a=0;a<3;++a) if(TacticChain::Living(seen,a))
        {
            central+=seen.MapDistance(seen.armies[a].tile,goal)<=3;
            northCovered|=seen.MapDistance(seen.armies[a].tile,GrandNorthPass)<=2;
        }
        if(central<2 || northCovered) return;
        for(auto& p:plans)
        {
            if(p.army!=4 || p.intent==JourneyEnemyIntent::Recover || chain.disordered[4]) continue;
            bool engaged=false;
            for(const auto& a:seen.armies) if(a.troops>0 && seen.Hostile(1,a.faction) && seen.MapDistance(p.from,a.tile)<=1) engaged=true;
            if(engaged) continue; // A side manoeuvre cannot abandon current contact.
            p.targetArmy=TacticChain::Living(seen,6) ? 6:-1;
            p.target=p.targetArmy>=0 ? seen.armies[6].tile:GrandNorthPass;
            const int next=JourneyAdvanceStep(seen,p.from,p.target,p.targetArmy<0);
            bool reserved=false;
            for(const auto& other:plans) if(other.army!=4 && other.next==next && other.next!=other.from) reserved=true;
            p.next=reserved ? p.from:next;p.role=JourneyEnemyRole::Counterattack;p.intent=JourneyEnemyIntent::Counterattack;
            auto after=std::make_unique<Campaign>(seen);after->armies[4].tile=p.next;p.danger.clear();
            for(int y=1;y<13;++y) for(int x=1;x<15;++x)
            { const int t=Campaign::At(x,y);if(t!=p.next && after->tiles[t].terrain!=Terrain::Sea && after->CanStrike(4,t)) p.danger.push_back(t); }
        }
    }
    int GuangzongVeteranOfficer(const StrategistJourney& j)
    {
        const auto& old=j.history[YellowTurbanMission];const auto& g=old.review.grand;
        return old.won && g.ordersEnabled && g.relievedOn>0 && g.leftReturned>0 && g.reliefOfficer>=0 && g.reliefOfficer<3 ? g.reliefOfficer:-1;
    }
    bool HasGuangzongVeterans(const StrategistJourney& j)
    {
        // The authored identity lives in the existing saved world. Old battles
        // keep their original rear guard; they are never replaced on load.
        return j.HasCoalitionBattle() && GuangzongVeteranOfficer(j)>=0 && j.world.armies.size()>7 &&
            j.world.generals[j.world.armies[7].general].name==U"広宗の義兵";
    }
    std::u32string RemainingOfficers(const StrategistJourney& j,const std::vector<ChainOrder>& orders)
    {
        std::u32string text;
        for(int a=0;a<3;++a)
            if(TacticChain::Living(j.world,a) && !j.chain.spent[a] &&
               std::none_of(orders.begin(),orders.end(),[&](const auto& o){return o.army==a;}))
            { if(!text.empty()) text+=U"・";text+=j.world.generals[j.world.armies[a].general].name; }
        return text.empty() ? U"なし":text;
    }
}
