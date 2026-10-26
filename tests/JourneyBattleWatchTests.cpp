#include "../NewGame/JourneyBattleWatch.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::unique_ptr<StrategistJourney> WatchMission(int index=1)
{
    auto j=std::make_unique<StrategistJourney>();j->Reset();j->Meet();j->mission=index;assert(j->Prepare(0,1));return j;
}
static int Count(const std::vector<JourneyBattleNotice>& events,JourneyNoticeKind kind)
{ return static_cast<int>(std::count_if(events.begin(),events.end(),[&](const auto& e) { return e.kind==kind; })); }
int main()
{
    {
        auto j=WatchMission();JourneyRealtime rt;
        assert(rt.Queue(*j,0,JourneyDirective::March,Campaign::At(9,12)));
        const int revision=j->world.revision;
        const auto a=JourneyBattleWatch::Capture(*j,rt);
        assert(a.units[4].id<0 && a.units[5].id==5 && a.paths[0].status==JourneyRouteStatus::Moving);
        j->world.armies[4].tile=Campaign::At(9,12);j->world.armies[4].troops=9999;
        const auto b=JourneyBattleWatch::Capture(*j,rt);
        assert(b.units[4].id<0 && b.paths[0].route==a.paths[0].route && j->world.revision==revision);
        JourneyBattleWatch watch;assert(watch.Record(a,b).empty());
        assert(rt.Queue(*j,0,JourneyDirective::March,j->world.armies[1].tile));
        assert(ReadJourneyOrder(*j,rt,0).status==JourneyRouteStatus::NoRoute);
        assert(ReadJourneyOrder(*j,rt,-1).status==JourneyRouteStatus::Idle);
    }
    {
        auto j=WatchMission();JourneyRealtime rt;JourneyBattleWatch watch;
        auto before=JourneyBattleWatch::Capture(*j,rt),after=before;
        before.units[0].troops=500;after.units[0].troops=499;++after.turn;
        const auto events=watch.Record(before,after);
        assert(Count(events,JourneyNoticeKind::Critical)==1 && events[0].army==0);
        auto next=after;++next.turn;assert(watch.Record(after,next).empty());
        before=next;after=next;before.units[1].morale=30;after.units[1].morale=29;++after.turn;
        assert(Count(watch.Record(before,after),JourneyNoticeKind::Critical)==1);
        before=after;after.units[1].troops=0;++after.turn;
        assert(Count(watch.Record(before,after),JourneyNoticeKind::Defeated)==1);
        before=after;after.units[5].troops-=20;++after.turn;
        assert(Count(watch.Record(before,after),JourneyNoticeKind::ConvoyHit)==1);
        before=after;after.units[5].cargo-=40;++after.turn;
        assert(Count(watch.Record(before,after),JourneyNoticeKind::ConvoyHit)==1);
    }
    {
        auto j=WatchMission();JourneyRealtime rt;JourneyBattleWatch watch;
        j->world.armies[3].troops=j->world.armies[4].troops=0;
        j->world.armies[0].tile=Campaign::At(4,3);j->world.armies[1].tile=Campaign::At(5,3);
        j->chain.BeginTurn(j->world);j->RefreshIntelligence();
        assert(rt.Queue(*j,0,JourneyDirective::March,Campaign::At(5,4)));
        assert(rt.Queue(*j,1,JourneyDirective::March,Campaign::At(5,4)));
        auto cycle=[&]
        { auto before=JourneyBattleWatch::Capture(*j,rt);assert(rt.Resolve(*j));return watch.Record(before,JourneyBattleWatch::Capture(*j,rt)); };
        auto steady=JourneyBattleWatch::Capture(*j,rt);
        for (int i=0;i<5;++i) assert(watch.Record(steady,steady).empty());
        assert(cycle().empty());assert(Count(cycle(),JourneyNoticeKind::Stalled)==2);assert(cycle().empty());
        rt.Cancel(1);assert(cycle().empty());assert(j->world.armies[0].tile==Campaign::At(5,4));
        assert(watch.History().size()==2);watch.Reset();assert(watch.History().empty());
    }
    {
        auto j=WatchMission();JourneyRealtime rt;JourneyBattleWatch watch;
        assert(rt.Queue(*j,0,JourneyDirective::Attack,3));
        auto before=JourneyBattleWatch::Capture(*j,rt);
        for (auto& c : j->intelligence[0].contacts) if (c.id==3) c.current=false;
        auto after=JourneyBattleWatch::Capture(*j,rt);
        assert(after.paths[0].status==JourneyRouteStatus::LostTarget && after.units[3].id<0);
        assert(Count(watch.Record(before,after),JourneyNoticeKind::TargetLost)==1);
        assert(watch.Record(after,after).empty());
        j->RefreshIntelligence();
        auto found=JourneyBattleWatch::Capture(*j,rt);
        assert(Count(watch.Record(after,found),JourneyNoticeKind::Contact)==1);
    }
    {
        auto j=WatchMission();JourneyRealtime rt;JourneyBattleWatch watch;
        auto before=JourneyBattleWatch::Capture(*j,rt),after=before;
        for (int i=0;i<20;++i)
        {
            before.units[3].id=-1;after.turn=100+i;
            assert(Count(watch.Record(before,after),JourneyNoticeKind::Contact)==1);
        }
        assert(watch.History().size()==JourneyBattleWatch::HistoryLimit && watch.History().front().turn==119);
        before.battle=false;assert(watch.Record(before,after).empty());
    }
    {
        auto j=WatchMission();JourneyRealtime rt;JourneyBattleWatch watch;
        auto before=JourneyBattleWatch::Capture(*j,rt);
        before.orders[0]={JourneyDirective::March,Campaign::At(4,4),-1};before.moved[0]=false;
        before.paths[0].status=JourneyRouteStatus::NoRoute;
        auto after=before;++after.turn;assert(watch.Record(before,after).empty());
        before=after;before.orders[0].destination=Campaign::At(4,5);after=before;++after.turn;
        assert(watch.Record(before,after).empty()); // Changing destination restarts the two-cycle observation.
        before=after;++after.turn;assert(Count(watch.Record(before,after),JourneyNoticeKind::Stalled)==1);
    }
    std::cout<<"Journey battle watch: PASS (observed snapshots, route state, thresholds, convoy, two-cycle stalls, recovery, bounded history)\n";
}
