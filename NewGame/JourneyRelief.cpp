#include "JourneyRealtime.hpp"
namespace frontline
{
    bool ReliefPending(const JourneyReliefRecord& r)
    { return r.outcome==JourneyReliefOutcome::Ordered || r.outcome==JourneyReliefOutcome::Departed || r.outcome==JourneyReliefOutcome::Arrived; }
    void StrategistJourney::RecordReliefOrder(int outgoing,int incoming,int source,int destination,bool realtime)
    {
        if(stage!=JourneyStage::Battle || outgoing<0 || outgoing>=3 || incoming<0 || incoming>=3 || incoming==outgoing ||
            !TacticChain::Living(world,outgoing) || !TacticChain::Living(world,incoming) || world.armies[outgoing].tile!=source ||
            !OnMap(destination) || world.MapDistance(source,destination)!=1 || world.MapDistance(world.armies[incoming].tile,source)!=1 ||
            chain.moved[outgoing] || chain.moved[incoming]) return;
        const bool beacon=Definition().kind==JourneyObjective::HoldPositions && (source==goal || source==Definition().secondGoal);
        if(!beacon && !(assignments[outgoing].duty==JourneyDuty::Hold && assignments[outgoing].tile==source)) return;
        battleReview.reliefTracked=true;
        JourneyReliefRecord record;record.outgoing=outgoing;record.incoming=incoming;record.source=source;record.destination=destination;
        record.issuedTurn=turn;record.realtime=realtime;
        for(auto& old:battleReview.relief) if(ReliefPending(old) && old.source==source)
        {
            if(old.outcome==JourneyReliefOutcome::Ordered) { old=record;return; }
            old.outcome=JourneyReliefOutcome::Unconfirmed;
        }
        if(battleReview.relief.size()>=32)
        {
            auto old=std::find_if(battleReview.relief.begin(),battleReview.relief.end(),[](const auto& r) { return !ReliefPending(r); });
            if(old==battleReview.relief.end()) return;
            battleReview.relief.erase(old);
        }
        battleReview.relief.push_back(record);
    }
    void StrategistJourney::RecordReliefMove(int army,int from,int to)
    {
        for(auto& r:battleReview.relief) if(ReliefPending(r))
        {
            if(army==r.outgoing && !r.departedTurn && from==r.source)
            {
                if(to==r.destination) { r.departedTurn=turn;r.outcome=JourneyReliefOutcome::Departed; }
                else r.outcome=JourneyReliefOutcome::Unconfirmed;
            }
            if(army==r.incoming)
            {
                if(r.departedTurn && !r.arrivedTurn && to==r.source) { r.arrivedTurn=turn;r.outcome=JourneyReliefOutcome::Arrived; }
                else if(to!=r.source) r.outcome=JourneyReliefOutcome::Unconfirmed;
            }
        }
    }
    void StrategistJourney::ValidateReliefOrders(const JourneyRealtime& clock)
    {
        for(auto& r:battleReview.relief) if(r.realtime && ReliefPending(r))
        {
            auto MarchTo=[&](int actor,int tile) { const auto& o=clock.orders[actor];return o.kind==JourneyDirective::March && o.destination==tile; };
            if((!r.departedTurn && !MarchTo(r.outgoing,r.destination)) || (!r.arrivedTurn && !MarchTo(r.incoming,r.source)))
                r.outcome=JourneyReliefOutcome::Unconfirmed;
        }
    }
    void StrategistJourney::CheckReliefOutcome()
    {
        for(auto& r:battleReview.relief) if(ReliefPending(r) && r.departedTurn)
        {
            const bool secure=PositionSecured(r.source);
            if(!r.arrivedTurn) { if(!secure) ++r.gapCycles;continue; }
            r.checkedTurn=turn-1;
            const bool held=TacticChain::Living(world,r.incoming) && world.armies[r.incoming].tile==r.source && secure;
            const bool beacon=Definition().kind==JourneyObjective::HoldPositions && (r.source==goal || r.source==Definition().secondGoal);
            r.allSecured=held && (!beacon || (PositionSecured(goal) && PositionSecured(Definition().secondGoal)));
            r.outcome=!held ? JourneyReliefOutcome::Lost:r.gapCycles>0 ? JourneyReliefOutcome::Recovered:JourneyReliefOutcome::Held;
        }
    }
    void StrategistJourney::FinalizeRelief()
    { for(auto& r:battleReview.relief) if(ReliefPending(r)) r.outcome=JourneyReliefOutcome::Unconfirmed; }
    std::u32string ReliefReviewText(const StrategistJourney& j)
    {
        const auto& r=j.history[j.mission].review;if(!r.reliefTracked || r.relief.empty()) return {};
        const auto& last=r.relief.back();const std::u32string names[]={U"劉備",U"関羽",U"張飛"};
        const auto number=[](int v) { const auto s=std::to_string(v);return std::u32string(s.begin(),s.end()); };
        auto text=U"交代："+names[last.outgoing]+U"→"+names[last.incoming]+U"。";
        if(last.outcome==JourneyReliefOutcome::Held) text+=U"交代直後の更新で守備を確認。";
        else if(last.outcome==JourneyReliefOutcome::Recovered) text+=number(last.gapCycles)+U"回の空白後、後任の到着と守備を確認。";
        else if(last.outcome==JourneyReliefOutcome::Lost) text+=U"後任は到着したが、次の更新では守備条件を満たせなかった。";
        else text+=last.arrivedTurn>0 ? U"2隊は移動。次の守備を確認する前に戦いを終えた。":last.departedTurn>0 ? U"前進済み、後任の到着は未完。":U"指示のみで、実際の交代は未完。";
        if((last.outcome==JourneyReliefOutcome::Held || last.outcome==JourneyReliefOutcome::Recovered) && !last.allSecured)
            text+=U"もう一方の烽火は維持条件を満たしていなかった。";
        return text;
    }
}
