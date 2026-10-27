#include "JourneyTestFlow.hpp"
#include "../NewGame/StrategistJourney.hpp"
#include "../NewGame/JourneyRealtime.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::unique_ptr<StrategistJourney> Opening(JourneyConvoyRoute route,int prep=0,int policy=1)
{
    auto j=std::make_unique<StrategistJourney>();j->Reset();j->Meet();
    assert(j->SelectPromise(JourneyPromise::EscortRefugees) && j->SelectConvoyRoute(route) && j->Prepare(prep,policy));return j;
}
static void FightVisible(StrategistJourney& j,int army)
{
    const auto contacts=j.intelligence[0].contacts;
    for (const auto& c : contacts) if (c.current && j.world.CanStrike(army,c.observed.tile) && j.Strike(army,c.id)) return;
    if (j.chain.moved[army]) return;
    int target=-1,range=100;
    for (const auto& c : contacts) if (c.current && j.world.MapDistance(j.world.armies[army].tile,c.observed.tile)<range)
    { range=j.world.MapDistance(j.world.armies[army].tile,c.observed.tile);target=c.observed.tile; }
    if (target<0) return;
    int next=-1,best=range;
    for (int tile : j.world.MapNeighbors(j.world.armies[army].tile))
        if (StrategistJourney::OnMap(tile) && tile/Width!=11 && j.intelligence[0].visible[tile] && TacticChain::Free(j.world,tile) && j.world.MapDistance(tile,target)<best)
        { best=j.world.MapDistance(tile,target);next=tile; }
    if (next>=0) j.Move(army,next);
    for (const auto& c : j.intelligence[0].contacts) if (c.current && j.world.CanStrike(army,c.observed.tile) && j.Strike(army,c.id)) return;
}
int main()
{
    std::cout<<std::unitbuf;
    for (auto route : {JourneyConvoyRoute::Road,JourneyConvoyRoute::SouthernWood})
    {
        auto j=Opening(route);const auto& path=j->ConvoyPath();
        assert(path.front()==j->world.armies[5].tile && path.back()==j->goal);
        for (std::size_t i=1;i<path.size();++i) assert(j->world.MapDistance(path[i-1],path[i])==1 && TacticChain::Free(j->world,path[i]));
    }
    {
        auto j=std::make_unique<StrategistJourney>();j->Reset();j->Meet();j->SelectPromise(JourneyPromise::EscortRefugees);
        const auto gold=j->world.cities[0].gold;const int road=j->PreparationCosts(0).gold;
        assert(j->SelectConvoyRoute(JourneyConvoyRoute::SouthernWood));assert(j->world.cities[0].gold==gold && j->PreparationCosts(0).gold==road+120);
        assert(j->Prepare(0,1) && j->world.cities[0].gold==gold-road-120);
        assert(!j->SelectConvoyRoute(JourneyConvoyRoute::Road) && !j->Prepare(0,1));
        j->Withdraw();assert(j->battleReview.resolution==JourneyResolution::Withdrawn && j->history[0].route==JourneyConvoyRoute::SouthernWood);
    }
    {
        auto j=std::make_unique<StrategistJourney>();j->Reset();j->Meet();j->SelectPromise(JourneyPromise::EscortRefugees);
        j->world.cities[0].gold=319;j->SelectConvoyRoute(JourneyConvoyRoute::SouthernWood);
        assert(!j->CanPrepare(0) && !j->Prepare(0,1) && j->world.cities[0].gold==319 && j->world.armies.empty());
        assert(j->SelectConvoyRoute(JourneyConvoyRoute::Road) && j->CanPrepare(0));
        j->SelectPromise(JourneyPromise::OpenRoad);assert(!j->SelectConvoyRoute(JourneyConvoyRoute::SouthernWood));
    }
    {
        auto j=Opening(JourneyConvoyRoute::Road);
        assert(j->Execute({{0,3,ChainTactic::Lure,Campaign::At(8,11)}}).completed==0);
        assert(j->Execute({{0,3,ChainTactic::Feint,-1}}).completed==1);
        assert(j->battleReview.tacticAttempts==2 && j->battleReview.tacticSteps==1 && j->battleReview.longestChain==1);
    }
    {
        // Isolate traffic and expiry for report accuracy, not a victory script.
        auto j=Opening(JourneyConvoyRoute::Road);
        j->world.armies[3].troops=j->world.armies[4].troops=0;j->RefreshIntelligence();
        assert(j->Move(0,Campaign::At(8,11)));
        for (int i=0;i<3;++i) assert(j->EndTurn());
        assert(j->battleReview.enemyStops==0 && j->battleReview.trafficStops==2);
        assert(j->Withdraw() && j->history[0].review.trafficStops==2 && j->BattleReviewLesson().find(U"通路")!=std::u32string::npos);
        assert(!j->EndTurn() && j->battleReview.trafficStops==2);
    }
    {
        auto j=Opening(JourneyConvoyRoute::Road);
        for (int i=0;i<3;++i) j->world.armies[i].tile=Campaign::At(2+i,3);
        j->world.armies[3].tile=Campaign::At(9,11);j->world.armies[4].troops=0;j->RefreshIntelligence();
        while (j->stage==JourneyStage::Battle) assert(j->EndTurn());
        assert(!j->won && j->battleReview.resolution==JourneyResolution::Deadline && j->battleReview.enemyStops>0);
        assert(j->BattleReviewLesson().find(U"前路の敵")!=std::u32string::npos);
    }
    for (auto route : {JourneyConvoyRoute::Road,JourneyConvoyRoute::SouthernWood})
    {
        auto j=Opening(route);
        JourneyRealtime escort;bool passedForest=false,reserveHandled=false;
        if (route==JourneyConvoyRoute::SouthernWood) assert(escort.Queue(*j,2,JourneyDirective::March,Campaign::At(8,12)));
        for (int phase=1;phase<=10 && j->stage==JourneyStage::Battle;++phase)
        {
            if (route==JourneyConvoyRoute::Road)
            {
                if (phase==1) { assert(j->Move(0,Campaign::At(9,9)));assert(j->Move(2,Campaign::At(9,10))); }
                if (phase==2)
                {
                    assert(j->Move(0,Campaign::At(10,9)));assert(j->Move(1,Campaign::At(9,9)));
                    const std::vector<ChainOrder> chain{{0,3,ChainTactic::Feint,-1},{1,3,ChainTactic::Disrupt,-1},{2,3,ChainTactic::Breakthrough,-1}};
                    assert(j->Execute(chain).completed==3);
                }
                if (phase>=2) for (int army=0;army<3 && j->stage==JourneyStage::Battle;++army) FightVisible(*j,army);
            }
            else
            {
                if (phase==1) assert(j->Move(0,Campaign::At(8,11)));
                if (phase==3) assert(j->Move(0,Campaign::At(9,11)));
                if (!passedForest && j->world.armies[2].tile==Campaign::At(8,12))
                { passedForest=true;assert(escort.Queue(*j,2,JourneyDirective::March,Campaign::At(12,11))); }
                auto* reserve=j->intelligence[0].Find(4);
                const bool engaged=reserve && reserve->current && j->world.CanStrike(2,reserve->observed.tile);
                const auto path=escort.Route(*j,2);
                if (!reserveHandled && !engaged && path.size()>1) j->Move(2,path[1]);
                reserve=j->intelligence[0].Find(4);
                if (reserve && reserve->current && j->world.CanStrike(2,reserve->observed.tile))
                {
                    const std::vector<ChainOrder> push{{2,4,ChainTactic::Breakthrough,-1}};
                    if (j->Preview(push).completed==1 && j->Preview(push).events[0].from!=j->Preview(push).events[0].to) { j->Execute(push);reserveHandled=true; }
                    else if (!j->chain.signatureUsed[2])
                    {
                        const std::vector<ChainOrder> stop{{0,4,ChainTactic::Feint,-1},{2,4,ChainTactic::SerpentRoar,-1}};
                        if (j->Preview(stop).completed==2) j->Execute(stop);
                    }
                    else j->Strike(2,4);
                }
                if (j->stage==JourneyStage::Battle && !j->chain.spent[0] && !j->chain.moved[0]) j->Hold(0);
            }
            if (j->stage==JourneyStage::Battle) j->EndTurn();
            std::cout<<"route "<<static_cast<int>(route)<<" turn "<<j->turn<<" cart "<<j->convoyStep<<" troops "<<j->world.armies[0].troops+j->world.armies[1].troops+j->world.armies[2].troops<<" zhang "<<j->world.armies[2].tile%Width<<","<<j->world.armies[2].tile/Width<<" front "<<j->world.armies[3].troops<<" reserve "<<j->world.armies[4].tile%Width<<","<<j->world.armies[4].tile/Width<<"/"<<j->world.armies[4].morale<<"\n";
        }
        assert(j->won && j->history[0].rescued && j->battleReview.resolution==JourneyResolution::Delivered);
        assert(route==JourneyConvoyRoute::Road ? j->battleReview.longestChain==3 : (j->battleReview.longestChain==2 && j->world.armies[3].troops>450 && j->world.armies[3].morale>30));
        assert(ContinueJourneyWithPledge(*j) && j->history[0].review.resolution==JourneyResolution::Delivered && j->Prepare(0,1));
        assert(j->world.generals[5].name==U"季安の糧車");
    }
    std::cout<<"Opening routes: PASS (real costs, geometry, distinct public-command victories, objective review, carryover)\n";
}
