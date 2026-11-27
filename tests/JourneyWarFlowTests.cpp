#include "JourneyAutoBattle.hpp"
#include "JourneyBattleChoices.hpp"
#include "JourneySave.hpp"
#include "JourneyStrategicResponse.hpp"
#include "JourneyEpisode.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
int main()
{
    for(auto difficulty:{JourneyDifficulty::Story,JourneyDifficulty::Standard})
    for(auto approach:{GrandApproach::RelieveLeft,GrandApproach::CentralAssault})
    for(bool solo:{false,true})
    {
        auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;
        j.ResetCampaign();j.Meet();j.mission=YellowTurbanMission;
        j.grandCampaign=j.grandStrategyCampaign=j.coalitionBattleCampaign=true;
        for(int m=0;m<YellowTurbanMission;++m) j.history[m].completed=j.history[m].won=j.history[m].campaignFlow=true;
        assert(j.SelectDifficulty(difficulty));assert(j.Prepare(1,0));assert(j.ChooseGrandApproach(approach));
        int singles=0,teams=0;
        for(int c=0;c<45 && j.stage==JourneyStage::Battle;++c)
        {
            auto p=PlanJourneyAutoBattle(j);
            if(solo && p.tactics.size()>1)
            {
                const auto choices=ReadJourneyBattleChoices(j,p.tactics.front().enemy);
                const auto it=std::find_if(choices.begin(),choices.end(),[](const auto& choice){return choice.orders.size()==1;});
                p.tactics=it==choices.end() ? std::vector<ChainOrder>{}:it->orders;
            }
            if(!p.tactics.empty()) { singles+=p.tactics.size()==1;teams+=p.tactics.size()>1;assert(j.Execute(p.tactics).completed>0); }
            if(j.stage==JourneyStage::Battle) assert(p.movement.Resolve(j));
        }
        assert(j.won && singles>0 && (!solo || teams==0));
        const auto g=j.history[YellowTurbanMission].review.grand;
        assert(g.openedOn>0 && g.ralliedOn>0);
        if(approach==GrandApproach::CentralAssault)
        {
            assert(g.forcedSteps>0);const auto episode=BuildJourneyEpisode(j);
            bool acknowledged=false;for(const auto& line:episode.lines) acknowledged|=line.text.find(U"橋の先を開けた")!=std::u32string::npos;
            assert(acknowledged);
        }
        if(approach==GrandApproach::RelieveLeft) assert(g.relievedOn>0 && g.leftReturned>0);
        std::cout<<"difficulty="<<int(difficulty)<<" approach="<<int(approach)<<" solo="<<solo<<" turns="<<j.turn<<" survivors="<<j.survivors<<" left="<<g.leftReturned<<" singles="<<singles<<" combos="<<teams<<"\n";
        assert(j.Continue() && j.ChooseCoalitionPlan(CoalitionPlan::Protect));
        s->beforeBattle=j;assert(j.Prepare(1,0));
        assert(HasGuangzongVeterans(j)==(g.relievedOn>0 && g.leftReturned>0));
        std::vector<std::uint8_t> bytes;auto restored=std::make_unique<JourneySaveData>();
        assert(EncodeJourneySave(*s,bytes) && DecodeJourneySave(bytes,*restored));
        auto& next=restored->journey;
        for(int c=0;c<60 && next.stage==JourneyStage::Battle;++c)
        {
            auto p=PlanJourneyAutoBattle(next);
            if(!p.tactics.empty()) assert(next.Execute(p.tactics).completed>0);
            if(next.stage==JourneyStage::Battle) assert(p.movement.Resolve(next));
        }
        if(!next.won)
        {
            std::cerr<<"Bianshui failed difficulty="<<int(difficulty)<<" approach="<<int(approach)<<" solo="<<solo
                <<" stage="<<int(next.stage)<<" turns="<<next.turn<<" rescued="<<next.battleReview.grand.coalition.rescuedOn<<"\n";
            for(int a=0;a<int(next.world.armies.size());++a)
                std::cerr<<a<<" troops="<<next.world.armies[a].troops<<" tile="<<next.world.armies[a].tile<<" morale="<<next.world.armies[a].morale<<"\n";
            for(const auto& p:next.PlanCoalitionAllies())
                std::cerr<<"ally="<<p.army<<" from="<<p.from<<" next="<<p.next<<" target="<<p.target<<"\n";
        }
        assert(next.won && next.history[CoalitionMission].completed);
    }
    std::cout<<"War flow: eight real Guangzong approaches, solo-only/normal tactics, earned outcomes, saved transition and Bianshui victory passed\n";
}
