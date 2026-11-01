#include "JourneyAutoBattle.hpp"
#include "JourneySave.hpp"
#include "JourneyEpisode.hpp"
#include "JourneyCampaign.hpp"
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
    // A bridgehead fights beside the bridge rather than holding up its allies.
    auto bridge=std::make_unique<StrategistJourney>();bridge->ResetCampaign();bridge->mission=1;bridge->Meet();assert(bridge->Prepare(1,0));
    bridge->world.armies[0].tile=bridge->world.armies[0].target=Campaign::At(11,11);
    bridge->world.armies[1].tile=bridge->world.armies[1].target=Campaign::At(9,10);
    bridge->world.armies[2].tile=bridge->world.armies[2].target=Campaign::At(9,11);bridge->RefreshIntelligence();
    auto crossing=PlanJourneyAutoBattle(*bridge);const auto route=crossing.movement.Route(*bridge,0);
    assert(crossing.movement.orders[0].kind==JourneyDirective::March && route.size()==2);
    assert(bridge->world.tiles[route[1]].terrain!=Terrain::Bridge && route[1]%Width>=12);
    std::vector<int> arrivals;
    for(int a=0;a<3;++a)
    {
        const auto path=crossing.movement.Route(*bridge,a);if(path.size()<2) continue;
        assert(std::find(arrivals.begin(),arrivals.end(),path[1])==arrivals.end());arrivals.push_back(path[1]);
    }
    assert(crossing.movement.Resolve(*bridge) && bridge->world.armies[0].tile==route[1]);
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
    // A complete new-player campaign carries its real losses/resources forward
    // and uses the next council's default enemy policy, rather than resetting battles.
    auto career=std::make_unique<StrategistJourney>();career->ResetCampaign();career->SelectDifficulty(JourneyDifficulty::Story);career->Meet();
    for(int mission=0;mission<3;++mission)
    {
        assert(career->mission==mission && career->Prepare(1,career->enemyPolicy));
        for(int cycle=0;cycle<18 && career->stage==JourneyStage::Battle;++cycle)
        {
            auto next=PlanJourneyAutoBattle(*career);
            if(!next.tactics.empty()) career->Execute(next.tactics);
            if(career->stage==JourneyStage::Battle) assert(next.movement.Resolve(*career));
        }
        std::cerr<<"campaign auto mission="<<mission<<" policy="<<career->enemyPolicy<<" won="<<career->won<<" troops="<<career->world.cities[0].troops<<" food="<<career->world.cities[0].food<<"\n";
        assert(career->won && career->CanContinue());
        if(career->NeedsStoryReply()) assert(career->ChooseStoryReply(JourneyPledge::Coordinate));
        assert(career->Continue());
    }
    assert(career->stage==JourneyStage::Complete && career->history[0].won && career->history[1].won && career->history[2].won);
    assert(CampaignBeat(*career).phase.find(U"序章 完")!=std::u32string::npos);
    const auto ending=BuildJourneyEpisode(*career);
    assert(!ending.lines.empty() && ending.lines.back().direction.find(U"序章、完")!=std::u32string::npos);
    hold->stage=JourneyStage::Debrief;const auto ended=PlanJourneyAutoBattle(*hold);assert(ended.tactics.empty());
    for(const auto& order:ended.movement.orders) assert(order.kind==JourneyDirective::None);
}
