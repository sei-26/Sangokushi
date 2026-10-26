#include "../NewGame/JourneyChainSignal.hpp"
#include "../NewGame/JourneyRealtime.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;

static std::unique_ptr<StrategistJourney> Formation()
{
    auto j=std::make_unique<StrategistJourney>();j->Reset();j->Meet();assert(j->Prepare(1,1));
    const int positions[]={Campaign::At(9,9),Campaign::At(10,8),Campaign::At(8,9),Campaign::At(9,8)};
    for (int i=0;i<4;++i) j->world.armies[i].tile=j->world.armies[i].target=positions[i];
    j->RefreshIntelligence();return j;
}
static std::vector<ChainOrder> Sequence()
{ return {{0,3,ChainTactic::Feint,-1},{1,3,ChainTactic::Disrupt,-1},{2,3,ChainTactic::Breakthrough,-1}}; }
int main()
{
    {
        auto j=Formation();const auto orders=Sequence();JourneyChainSignal signal;
        const auto spent=j->chain.spent,unique=j->chain.signatureUsed;const int revision=j->world.revision;
        const int troops=j->world.armies[3].troops,morale=j->world.armies[0].morale;
        assert(signal.Arm(*j,orders));assert(signal.Inspect(*j,orders).state==ChainSignalState::Ready);
        for (int i=0;i<30;++i) assert(!signal.Release(*j,orders,false));
        assert(j->world.revision==revision && j->world.armies[3].troops==troops && j->world.armies[0].morale==morale);
        assert(j->chain.spent==spent && j->chain.signatureUsed==unique);
        assert(signal.Release(*j,orders,true));assert(!signal.Armed() && !signal.Release(*j,orders,true));
        const auto result=j->Execute(orders);assert(result.completed==3 && j->world.armies[3].troops<troops);
    }
    {
        auto j=Formation();auto orders=Sequence();JourneyChainSignal signal;
        j->world.armies[2].tile=Campaign::At(6,7);j->RefreshIntelligence();
        assert(signal.Arm(*j,orders));const auto waiting=signal.Inspect(*j,orders);
        assert(waiting.state==ChainSignalState::Waiting && waiting.completed==2 && waiting.blockingArmy==2 && !waiting.reason.empty());
        const int enemy=j->world.armies[3].troops;
        assert(!signal.Release(*j,orders,true) && j->world.armies[3].troops==enemy); // Never execute the feasible prefix.
        j->world.armies[2].tile=Campaign::At(8,9);j->RefreshIntelligence();
        assert(signal.Inspect(*j,orders).state==ChainSignalState::Ready);
        auto changed=orders;std::swap(changed[0],changed[1]);
        assert(signal.Inspect(*j,changed).state==ChainSignalState::Invalid && !signal.Release(*j,changed,true));
        changed=orders;changed[0].destination=Campaign::At(8,10);
        assert(signal.Inspect(*j,changed).state==ChainSignalState::Invalid);
        signal.Cancel();assert(!signal.Armed() && !signal.Release(*j,orders,true));
    }
    {
        auto j=Formation();auto orders=Sequence();JourneyChainSignal signal;assert(signal.Arm(*j,orders));
        const auto before=signal.Inspect(*j,orders);
        j->world.armies[4].tile=Campaign::At(9,7);j->world.armies[4].troops=3000;
        const auto after=signal.Inspect(*j,orders);
        assert(after.state==before.state && after.completed==before.completed && after.reason==before.reason);
        auto blocked=std::make_unique<StrategistJourney>(*j);auto released=signal;
        assert(released.Release(*blocked,orders,true));
        assert(blocked->Execute(orders).completed==2 && !blocked->chain.spent[2]); // Real hidden occupancy can still stop the predicted chain.
        for (auto& c : j->intelligence[0].contacts) if (c.id==3) c.current=false;
        assert(signal.Inspect(*j,orders).state==ChainSignalState::Invalid && !signal.Release(*j,orders,true));
        assert(!JourneyChainSignal{}.Arm(*j,orders)); // A remembered contact is not a current authorization.
    }
    {
        auto j=Formation();auto orders=Sequence();JourneyChainSignal signal;
        assert(!signal.Arm(*j,{}));assert(!signal.Arm(*j,{{0,3,ChainTactic::Feint,-1},{0,3,ChainTactic::Feint,-1}}));
        assert(!signal.Arm(*j,{{-1,3,ChainTactic::Feint,-1}}));
        assert(!signal.Arm(*j,{{0,-1,ChainTactic::AzureDragon,-1}}));
        j->chain.spent[1]=true;assert(signal.Arm(*j,orders));
        assert(signal.Inspect(*j,orders).state==ChainSignalState::Waiting);
        j->chain.BeginTurn(j->world);assert(signal.Inspect(*j,orders).state==ChainSignalState::Ready);
        j->world.armies[2].troops=0;assert(signal.Inspect(*j,orders).state==ChainSignalState::Invalid);
    }
    {
        auto j=Formation();JourneyChainSignal signal;
        const std::vector<ChainOrder> orders{{0,-1,ChainTactic::Oath,-1},{2,3,ChainTactic::SerpentRoar,-1},{1,3,ChainTactic::AzureDragon,-1}};
        j->world.armies[1].morale=15; // The preceding oath supplies this actor's morale cost.
        assert(signal.Arm(*j,orders) && signal.Inspect(*j,orders).state==ChainSignalState::Ready);
        j->chain.signatureUsed[1]=true;assert(signal.Inspect(*j,orders).state==ChainSignalState::Invalid);
        signal.Cancel();assert(!signal.Arm(*j,orders));
        j->chain.signatureUsed[1]=false;assert(signal.Arm(*j,orders));
        j->Withdraw();assert(signal.Inspect(*j,orders).state==ChainSignalState::Invalid);
    }
    {
        auto normal=Formation(),reserved=std::make_unique<StrategistJourney>(*normal);
        JourneyRealtime a,b;JourneyChainSignal signal;const auto orders=Sequence();assert(signal.Arm(*reserved,orders));
        const int day=reserved->world.day;
        assert(a.Queue(*normal,0,JourneyDirective::Attack,3));assert(b.Queue(*reserved,0,JourneyDirective::Attack,3));
        assert(a.Resolve(*normal));assert(b.Resolve(*reserved,signal.ReservedAttacks()));
        assert(normal->world.armies[3].troops<reserved->world.armies[3].troops); // Forego the extra standing strike while waiting.
        assert(reserved->turn==2 && reserved->world.day==day+1); // Reserving the strike does not freeze the battlefield.
    }
    {
        auto j=std::make_unique<StrategistJourney>();j->Reset();j->Meet();assert(j->Prepare(0,1));
        JourneyRealtime rt;JourneyChainSignal signal;const auto orders=Sequence();
        assert(rt.Queue(*j,1,JourneyDirective::March,Campaign::At(9,9)));
        assert(rt.Queue(*j,2,JourneyDirective::March,Campaign::At(9,11)));
        assert(signal.Arm(*j,orders));int cycles=0;
        while (cycles<4 && signal.Inspect(*j,orders).state==ChainSignalState::Waiting)
        { assert(rt.Resolve(*j,signal.ReservedAttacks()));++cycles; }
        assert(cycles>0 && signal.Release(*j,orders,true));
        assert(j->Execute(orders).completed==3); // Reproducible setup from the unmodified first battle.
    }
    std::cout<<"Journey chain signal: PASS (full sequence, paused authorization, one shot, privacy, stale edits, shared costs, reserved attack)\n";
}
