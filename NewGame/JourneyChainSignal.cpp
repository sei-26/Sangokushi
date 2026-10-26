#include "JourneyChainSignal.hpp"

namespace frontline
{
    ChainSignalRead JourneyChainSignal::Assess(const StrategistJourney& journey, const std::vector<ChainOrder>& orders)
    {
        ChainSignalRead read;read.total=static_cast<int>(orders.size());
        const auto invalid=[&](const char32_t* reason)
        { read.state=ChainSignalState::Invalid;read.reason=reason;return read; };
        if (journey.stage!=JourneyStage::Battle) return invalid(U"戦闘が終了したため、合図待ちを解除した。");
        if (orders.empty() || orders.size()>3) return invalid(U"１〜３手の戦法を予約してから合図を設定する。");
        std::array<bool,3> actors{};
        for (const auto& order : orders)
        {
            read.blockingArmy=order.army;
            if (order.army<0 || order.army>=3 || actors[order.army]) return invalid(U"各武将に１手ずつ予約しよう。");
            actors[order.army]=true;
            if (!TacticChain::Living(journey.world,order.army)) return invalid(U"担当部隊が戦場にいないため、合図待ちを解除した。");
            if (!TacticChain::CanUse(journey.world,order.army,order.tactic)) return invalid(U"担当できない戦法が含まれているため、合図待ちを解除した。");
            const auto& definition=TacticChain::Definition(order.tactic);
            if (definition.unique && journey.chain.signatureUsed[order.army]) return invalid(U"使用済みの固有戦法があるため、合図待ちを解除した。");
            if (!definition.targetsEnemy) continue;
            const auto* contact=journey.intelligence[0].Find(order.enemy);
            if (!contact || !contact->current) return invalid(U"対象を見失ったため、合図待ちを解除した。予約を残して指揮を停止。");
            if (contact->observed.troops<=0 || !journey.world.Hostile(journey.world.player,contact->observed.faction))
                return invalid(U"対象の敵がいなくなったため、合図待ちを解除した。");
        }
        const auto preview=journey.Preview(orders);
        read.completed=preview.completed;
        read.state=read.completed==read.total ? ChainSignalState::Ready : ChainSignalState::Waiting;
        read.blockingArmy=read.state==ChainSignalState::Ready ? -1 : orders[read.completed].army;
        read.reason=read.state==ChainSignalState::Ready ? U"全手が成立する見込み。再開後、動きの完了時に一度だけ発動。" :
            preview.events.empty() ? U"予約を確認しよう。" : preview.events.back().reason;
        return read;
    }
    bool JourneyChainSignal::Arm(const StrategistJourney& journey, const std::vector<ChainOrder>& orders)
    {
        if (Assess(journey,orders).state==ChainSignalState::Invalid) return false;
        sequence=orders;return true;
    }
    ChainSignalRead JourneyChainSignal::Inspect(const StrategistJourney& journey, const std::vector<ChainOrder>& orders) const
    {
        if (!Armed()) return {};
        bool same=sequence.size()==orders.size();
        for (std::size_t i=0;same && i<orders.size();++i)
            same=sequence[i].army==orders[i].army && sequence[i].enemy==orders[i].enemy &&
                sequence[i].tactic==orders[i].tactic && sequence[i].destination==orders[i].destination;
        if (!same) return {ChainSignalState::Invalid,0,static_cast<int>(orders.size()),-1,U"予約を変更したため、合図待ちを解除した。組み直した後に再設定しよう。"};
        return Assess(journey,orders);
    }
    bool JourneyChainSignal::Release(const StrategistJourney& journey, const std::vector<ChainOrder>& orders, bool running)
    {
        if (!running || Inspect(journey,orders).state!=ChainSignalState::Ready) return false;
        Cancel();return true;
    }
    std::array<bool,3> JourneyChainSignal::ReservedAttacks() const
    {
        std::array<bool,3> reserved{};
        for (const auto& order : sequence) reserved[order.army]=true;
        return reserved;
    }
}
