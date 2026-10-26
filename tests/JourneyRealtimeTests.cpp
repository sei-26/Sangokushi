#include "../NewGame/JourneyRealtime.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
using namespace frontline;

static std::unique_ptr<StrategistJourney> CreateRealtimeMission(int index=1)
{
    auto j=std::make_unique<StrategistJourney>();j->Reset();j->Meet();j->mission=index;
    assert(j->Prepare(0,1));return j;
}
static void Quiet(StrategistJourney& j)
{
    j.world.armies[3].troops=j.world.armies[4].troops=0;j.RefreshIntelligence();
}
static void Same(const StrategistJourney& a,const StrategistJourney& b)
{
    assert(a.turn==b.turn && a.stage==b.stage && a.convoyStep==b.convoyStep && a.heldTurns==b.heldTurns);
    for (int i=0;i<static_cast<int>(a.world.armies.size());++i)
    {
        const auto& x=a.world.armies[i];const auto& y=b.world.armies[i];
        assert(x.tile==y.tile && x.troops==y.troops && x.morale==y.morale && x.food==y.food && x.cargoFood==y.cargoFood);
    }
    assert(a.chain.spent==b.chain.spent && a.chain.signatureUsed==b.chain.signatureUsed);
}
int main()
{
    {
        JourneyRealtime clock;clock.Advance(500);assert(clock.elapsed==0 && !clock.Due());
        clock.paused=false;clock.Advance(std::numeric_limits<double>::quiet_NaN());clock.Advance(-2);assert(clock.elapsed==0);
        for (int i=0;i<48;++i) clock.Advance(.25);
        assert(clock.Due() && clock.activeSeconds==12);clock.Consume();assert(!clock.Due() && clock.pulses==1);
        clock.speed=2;clock.Advance(6);assert(clock.Due());clock.Consume();clock.paused=true;
        clock.Advance(100);assert(clock.pulses==2 && clock.elapsed==0);
        clock.Consume(true);assert(clock.paused && clock.pulses==3 && clock.activeSeconds==36);
    }
    {
        auto j=CreateRealtimeMission();Quiet(*j);JourneyRealtime rt;
        j->world.armies[0].tile=Campaign::At(2,7);j->RefreshIntelligence();
        const int revision=j->world.revision,food=j->world.armies[0].food;
        assert(!rt.Queue(*j,0,JourneyDirective::March,Campaign::At(11,4)));
        assert(!rt.Queue(*j,0,JourneyDirective::March,Campaign::At(2,10)));
        assert(!rt.Queue(*j,0,JourneyDirective::March,Campaign::At(15,8)));
        assert(rt.Queue(*j,0,JourneyDirective::March,Campaign::At(2,4)));
        auto route=rt.Route(*j,0);assert(route.size()>=4 && route.back()==Campaign::At(2,4));
        assert(j->world.revision==revision && j->world.armies[0].food==food && j->turn==1);
        const int hidden=route[1];j->world.armies[4].tile=hidden;j->world.armies[4].troops=500;
        assert(rt.Route(*j,0)==route); // Actual hidden occupancy cannot change a preview.
        j->world.armies[4].troops=0;
        for (int i=0;i<6 && rt.orders[0].kind!=JourneyDirective::None;++i)
        {
            const int before=j->world.armies[0].tile;assert(rt.Resolve(*j));
            assert(j->world.MapDistance(before,j->world.armies[0].tile)<=1);
        }
        assert(j->world.armies[0].tile==Campaign::At(2,4) && rt.orders[0].kind==JourneyDirective::None);
    }
    {
        auto j=CreateRealtimeMission();Quiet(*j);JourneyRealtime rt;
        j->world.armies[0].tile=Campaign::At(4,3);j->world.armies[1].tile=Campaign::At(5,3);j->chain.BeginTurn(j->world);j->RefreshIntelligence();
        assert(rt.Queue(*j,0,JourneyDirective::March,Campaign::At(5,4)));
        assert(rt.Queue(*j,1,JourneyDirective::March,Campaign::At(5,4)));
        assert(rt.Resolve(*j));
        assert(j->world.armies[0].tile==Campaign::At(4,3) && j->world.armies[1].tile==Campaign::At(5,3));
        assert(j->turn==2); // Both competing moves wait; neither receives iteration priority.
        rt.Cancel(1);assert(rt.Resolve(*j));assert(j->world.armies[0].tile==Campaign::At(5,4));
    }
    {
        auto manual=CreateRealtimeMission(0),live=std::make_unique<StrategistJourney>(*manual);JourneyRealtime rt;
        for (int step=0;step<3 && manual->stage==JourneyStage::Battle;++step)
        { assert(manual->EndTurn());assert(rt.Resolve(*live));Same(*manual,*live); }
    }
    {
        auto a=CreateRealtimeMission(2),b=std::make_unique<StrategistJourney>(*a);JourneyRealtime x,y;x.paused=y.paused=false;
        for (int i=0;i<144;++i) { x.Advance(.25);if (x.Due()) { x.Resolve(*a);x.Consume(); } }
        for (int i=0;i<288;++i) { y.Advance(.125);if (y.Due()) { y.Resolve(*b);y.Consume(); } }
        assert(x.pulses==3 && y.pulses==3);Same(*a,*b);
    }
    {
        auto j=CreateRealtimeMission(0);JourneyRealtime rt;
        assert(!rt.Queue(*j,0,JourneyDirective::Attack,4));
        assert(rt.Queue(*j,0,JourneyDirective::Attack,3));
        for (auto& c : j->intelligence[0].contacts) if (c.id==3) c.current=false;
        assert(rt.Route(*j,0).empty());
        assert(!rt.Queue(*j,0,JourneyDirective::Ambush));
        rt.Reset();assert(rt.paused && rt.orders[0].kind==JourneyDirective::None && rt.pulses==0);
    }
    std::cout<<"Journey realtime: PASS (pause, speed, deterministic clock, persistent paths, conflict, privacy, legacy equivalence)\n";
}
