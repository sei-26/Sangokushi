#include "JourneyBattleWatch.hpp"

namespace frontline
{
    JourneyWatchSnapshot JourneyBattleWatch::Capture(const StrategistJourney& journey,const JourneyRealtime& clock)
    {
        JourneyWatchSnapshot snapshot;snapshot.turn=journey.turn;snapshot.battle=journey.stage==JourneyStage::Battle;snapshot.orders=clock.orders;
        for (int id=0;id<6 && id<static_cast<int>(journey.world.armies.size());++id)
        {
            const Army* unit=nullptr;
            if (id<3 || (id==StrategistJourney::ConvoyArmy && journey.HasConvoy())) unit=&journey.world.armies[id];
            else
            {
                const auto* contact=journey.intelligence[0].Find(id);
                if (contact && contact->current) unit=&contact->observed;
            }
            if (!unit) continue;
            snapshot.units[id]={id,unit->tile,unit->troops,unit->morale,unit->cargoFood,journey.world.generals[unit->general].name};
            if (id<3)
            { snapshot.paths[id]=ReadJourneyOrder(journey,clock,id);snapshot.moved[id]=journey.chain.moved[id]; }
        }
        return snapshot;
    }
    std::vector<JourneyBattleNotice> JourneyBattleWatch::Record(const JourneyWatchSnapshot& before,const JourneyWatchSnapshot& after)
    {
        std::vector<JourneyBattleNotice> notices;
        if (!before.battle) return notices;
        const auto add=[&](JourneyNoticeKind kind,const JourneyWatchUnit& unit,const char32_t* text)
        { notices.push_back({kind,unit.id,unit.tile,after.turn,unit.name,text}); };
        for (int id=0;id<6;++id)
        {
            const auto& old=before.units[id];const auto& now=after.units[id];
            if (now.id<0) continue;
            if (id>=3 && id!=StrategistJourney::ConvoyArmy)
            {
                if (now.troops>0 && (old.id<0 || old.troops<=0)) add(JourneyNoticeKind::Contact,now,U"敵を捕捉。構えと進路を確認しよう。");
                continue;
            }
            if (old.id<0 || old.troops<=0) continue;
            if (now.troops<=0) add(JourneyNoticeKind::Defeated,now,U"部隊が壊滅。残る隊の役割を組み直そう。");
            else if (id==StrategistJourney::ConvoyArmy)
            {
                if (now.troops<old.troops || now.cargo<old.cargo) add(JourneyNoticeKind::ConvoyHit,now,U"護送車に被害。護衛と退路を確認しよう。");
            }
            else if ((old.troops>=500 && now.troops<500) || (old.morale>=30 && now.morale<30))
                add(JourneyNoticeKind::Critical,now,U"兵500未満、または士気30未満へ低下。救援・固守・撤収を判断しよう。");
        }
        for (int id=0;id<3;++id)
        {
            const auto& a=before.orders[id];const auto& b=after.orders[id];
            const auto& old=before.units[id];const auto& now=after.units[id];
            const bool same=SameJourneyOrder(a,b);
            if (same && b.kind==JourneyDirective::Attack && before.paths[id].status!=JourneyRouteStatus::LostTarget && after.paths[id].status==JourneyRouteStatus::LostTarget)
                add(JourneyNoticeKind::TargetLost,now,U"追撃対象を見失った。未知の位置は追わず、再捕捉を待っている。");
            if (after.turn<=before.turn) continue; // Tactic playback is not a second command cycle.
            const auto& last=lastOrders[id];
            if (!tracking[id] || !SameJourneyOrder(last,a)) stalled[id]=0;
            tracking[id]=true;lastOrders[id]=b;
            const bool travel=before.paths[id].status==JourneyRouteStatus::Moving || before.paths[id].status==JourneyRouteStatus::NoRoute;
            if (!same || old.id<0 || now.id<0 || now.troops<=0 || !travel || before.moved[id] || old.tile!=now.tile)
            { stalled[id]=0;continue; }
            ++stalled[id];
            if (stalled[id]==2) add(JourneyNoticeKind::Stalled,now,U"２周期続けて行軍が進まない。進路・味方の配置・命令先を見直そう。");
        }
        for (const auto& notice : notices) history.insert(history.begin(),notice);
        if (history.size()>HistoryLimit) history.resize(HistoryLimit);
        return notices;
    }
}
