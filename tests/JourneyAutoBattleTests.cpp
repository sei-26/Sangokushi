#include "JourneyAutoBattle.hpp"
#include "JourneySave.hpp"
#include <cassert>
#include <memory>
#include <iostream>
using namespace frontline;
int main()
{
    auto j=std::make_unique<StrategistJourney>();j->ResetCampaign();j->Meet();
    auto beforeBattle=std::make_unique<StrategistJourney>(*j);assert(j->Prepare(0,0));
    auto session=std::make_unique<JourneySaveData>();session->journey=*j;session->beforeBattle=*beforeBattle;
    std::vector<std::uint8_t> before,after;assert(EncodeJourneySave(*session,before));
    auto p=PlanJourneyAutoBattle(*j);session->journey=*j;assert(EncodeJourneySave(*session,after));assert(before==after);
    assert(!p.tactics.empty());const auto predicted=j->Preview(p.tactics);assert(predicted.completed==int(p.tactics.size()));
    auto fog=std::make_unique<StrategistJourney>(*j);
    for(auto& c:fog->intelligence[0].contacts)c.current=false;
    for(int a=3;a<int(fog->world.armies.size());++a) if(fog->world.Hostile(0,fog->world.armies[a].faction)) fog->world.armies[a].tile=Campaign::At(14,2);
    const auto hidden=PlanJourneyAutoBattle(*fog);assert(hidden.tactics.empty());
    for(const auto& order:hidden.movement.orders) assert(order.kind!=JourneyDirective::Attack);
    // The same visible observation produces identical decisions despite private enemy displacement.
    auto unseen=std::make_unique<StrategistJourney>(*fog);unseen->world.armies[3].tile=Campaign::At(13,3);
    const auto hidden2=PlanJourneyAutoBattle(*unseen);
    for(int a=0;a<3;++a) assert(SameJourneyOrder(hidden.movement.orders[a],hidden2.movement.orders[a]));
    const int turn=j->turn,food=j->world.armies[0].food;
    auto result=j->Execute(p.tactics);assert(result.completed>0);assert(j->chain.spent[result.events[0].army]);
    if(j->stage==JourneyStage::Battle) { assert(p.movement.Resolve(*j));assert(j->turn==turn+1);assert(j->world.armies[0].food<=food); }
    auto hold=std::make_unique<StrategistJourney>();hold->Reset();hold->mission=2;hold->Meet();assert(hold->Prepare(0,0));
    hold->world.armies[0].tile=hold->goal;hold->world.armies[1].tile=hold->Definition().secondGoal;hold->RefreshIntelligence();
    const auto guards=PlanJourneyAutoBattle(*hold);assert(guards.movement.orders[0].kind==JourneyDirective::Hold);assert(guards.movement.orders[1].kind==JourneyDirective::Hold);
    for(const auto& t:guards.tactics) assert(t.army==2);
    // Exercise actual autonomous engagements, not a fake result shortcut.
    for(int mission=0;mission<3;++mission)
    {
        auto battle=std::make_unique<StrategistJourney>();battle->ResetCampaign();battle->mission=mission;battle->Meet();
        battle->SelectDifficulty(JourneyDifficulty::Story);assert(battle->Prepare(1,0));
        int actions=0;
        for(int cycle=0;cycle<18 && battle->stage==JourneyStage::Battle;++cycle)
        {
            auto next=PlanJourneyAutoBattle(*battle);
            if(!next.tactics.empty()) { const auto result=battle->Execute(next.tactics);actions+=result.completed; }
            if(battle->stage==JourneyStage::Battle) assert(next.movement.Resolve(*battle));
        }
        assert(battle->stage==JourneyStage::Debrief);assert(battle->won);
        std::cerr<<"auto mission "<<mission<<" won="<<battle->won<<" survivors="<<battle->survivors<<"\n";
    }
    hold->stage=JourneyStage::Debrief;const auto ended=PlanJourneyAutoBattle(*hold);assert(ended.tactics.empty());
    for(const auto& order:ended.movement.orders) assert(order.kind==JourneyDirective::None);
}
