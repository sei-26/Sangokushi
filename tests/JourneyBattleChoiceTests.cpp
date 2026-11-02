#include "JourneyBattleChoices.hpp"
#include "JourneySave.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
int main()
{
    auto j=std::make_unique<StrategistJourney>();j->ResetCampaign();j->Meet();
    auto save=std::make_unique<JourneySaveData>();save->beforeBattle=*j;assert(j->Prepare(0,0));save->journey=*j;
    std::vector<std::uint8_t> before,after;assert(EncodeJourneySave(*save,before));
    const auto key=BattleChoiceContext(*j,3);const auto choices=ReadJourneyBattleChoices(*j,3);
    assert(choices.size()==2 && choices[0].orders.size()==1 && choices[1].orders.size()>1 && choices[1].orders.size()<=3);
    assert(choices[1].damage>choices[0].damage && choices[1].moraleCost>choices[0].moraleCost);
    save->journey=*j;assert(EncodeJourneySave(*save,after) && before==after);
    for(const auto& choice:choices)
    {
        assert(choice.target==3 && choice.damage>0 && choice.damage<=j->intelligence[0].Find(3)->observed.troops);
        auto actual=std::make_unique<StrategistJourney>(*j);
        const int troops=actual->world.armies[3].troops;auto result=actual->Execute(choice.orders);
        assert(result.completed==int(choice.orders.size()) && troops-actual->world.armies[3].troops==choice.damage);
        int cost=0;
        for(const auto& order:choice.orders)
        {
            assert(!TacticChain::Definition(order.tactic).unique);
            if(order.tactic==ChainTactic::Lure) assert(choice.orders.size()>1 && j->intelligence[0].visible[order.destination]);
            assert(actual->chain.spent[order.army] && actual->world.armies[order.army].morale==j->world.armies[order.army].morale-TacticChain::Definition(order.tactic).moraleCost);
            cost+=TacticChain::Definition(order.tactic).moraleCost;
        }
        assert(cost==choice.moraleCost);
        int lost=0;for(int a=0;a<3;++a) lost+=j->world.armies[a].troops-actual->world.armies[a].troops;
        assert(lost==choice.friendlyLoss);
        for(int a=0;a<3;++a) if(std::none_of(choice.orders.begin(),choice.orders.end(),[&](const ChainOrder& o) { return o.army==a; })) assert(!actual->chain.spent[a]);
    }
    auto privateWorld=std::make_unique<StrategistJourney>(*j);
    privateWorld->world.armies[4].tile=Campaign::At(8,2);privateWorld->world.armies[4].troops=9999;
    assert(BattleChoiceContext(*privateWorld,3)==key);
    auto equivalent=ReadJourneyBattleChoices(*privateWorld,3);
    assert(equivalent.size()==choices.size());
    for(int c=0;c<int(choices.size());++c) assert(equivalent[c].damage==choices[c].damage && equivalent[c].moraleCost==choices[c].moraleCost && equivalent[c].orders.size()==choices[c].orders.size());
    for(auto& c:privateWorld->intelligence[0].contacts)c.current=false;
    assert(ReadJourneyBattleChoices(*privateWorld,3).empty());
    j->chain.spent[0]=true;assert(BattleChoiceContext(*j,3)!=key);
    j->chain.spent[1]=j->chain.spent[2]=true;assert(ReadJourneyBattleChoices(*j,3).empty());
    auto hold=std::make_unique<StrategistJourney>();hold->ResetCampaign();hold->mission=2;hold->Meet();assert(hold->Prepare(1,0));
    hold->world.armies[0].tile=hold->world.armies[0].target=hold->goal;hold->RefreshIntelligence();
    for(const auto& choice:ReadJourneyBattleChoices(*hold,3)) for(const auto& order:choice.orders) assert(order.army!=0);
    std::cout<<"battle choices: actual damage/costs, unused officers, immutable reads, fog privacy, spent guards\n";
}
