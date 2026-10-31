#include "../NewGame/JourneyArchive.hpp"
#include "../NewGame/JourneyPlanExample.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
static std::vector<std::uint8_t> JourneyBytes(const StrategistJourney& j)
{ auto s=std::make_unique<JourneySaveData>();s->journey=j;return Bytes(*s); }
static std::unique_ptr<JourneySaveData> Attempt(bool realtime)
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=2;s->beforeBattle=j;
    assert(j.Prepare(0,1) && j.Move(1,Campaign::At(7,6)) && j.Move(0,Campaign::At(8,8)) && j.EndTurn());
    if(realtime)
    {
        j.world.armies[3].tile=j.world.armies[3].target=Campaign::At(13,12);j.world.armies[4].tile=j.world.armies[4].target=Campaign::At(13,2);j.RefreshIntelligence();
    }
    else { j.world.armies[3].tile=j.world.armies[3].target=Campaign::At(9,10);j.RefreshIntelligence(); }
    j.RecordReliefOrder(2,0,Campaign::At(8,9),Campaign::At(9,9),realtime);
    if(realtime)
    {
        s->realtimeMode=true;s->realtime.Queue(j,2,JourneyDirective::March,Campaign::At(9,9));s->realtime.Queue(j,0,JourneyDirective::March,Campaign::At(8,9));
        assert(s->realtime.Resolve(j) && s->realtime.Resolve(j));
    }
    else assert(j.Move(2,Campaign::At(9,9)) && j.Move(0,Campaign::At(8,9)) && j.EndTurn());
    assert(j.Withdraw());return s;
}
static void Same(const JourneyPlanExample& a,const JourneyPlanExample& b)
{
    assert(a.name==b.name && a.reason==b.reason && a.manualTask==b.manualTask && a.caution==b.caution && a.purpose==b.purpose && a.adviser==b.adviser && a.warning==b.warning);
    for(int i=0;i<3;++i) assert(a.assignments[i].tile==b.assignments[i].tile && a.assignments[i].duty==b.assignments[i].duty);
}
static std::vector<std::uint8_t> Old(const JourneySaveData& s,unsigned v)
{
    auto copy=std::make_unique<JourneySaveData>(s);std::vector<std::uint8_t> bytes;JourneyArchive w{false,bytes};w.version=v;
    unsigned magic=0x53594e47,checksum=0;w(magic,v,checksum);w(*copy);checksum=2166136261u;
    for(std::size_t i=12;i<bytes.size();++i) { checksum^=bytes[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) bytes[8+i]=static_cast<std::uint8_t>(checksum>>(8*i));return bytes;
}
int main()
{
    for(bool realtime:{false,true})
    {
        auto s=Attempt(realtime);auto& j=s->journey;const auto camp=s->beforeBattle->world.cities[0];const auto trust=s->beforeBattle->officerTrust;
        const auto expected=JourneyBytes(*s->beforeBattle);assert(j.RetryAtCouncil(*s->beforeBattle));
        assert(j.retryRelief && j.retryRelief->outcome==(realtime ? JourneyReliefOutcome::Recovered:JourneyReliefOutcome::Lost));
        assert(!j.history[2].completed && j.officerTrust==trust && j.world.cities[0].food==camp.food);
        auto neutral=std::make_unique<StrategistJourney>(j);neutral->retryRelief.reset();assert(JourneyBytes(*neutral)==expected);
        const auto before=Bytes(*s);const auto plan=PlanExample(j);assert(!plan.reason.empty() && plan.adviser==0 && !plan.warning.empty());
        assert(plan.reason.find(U"前回の試み")!=std::u32string::npos && plan.caution.find(U"最短2周期・空白1周期")!=std::u32string::npos);
        assert(plan.assignments[2].tile==Campaign::At(8,9) && plan.assignments[1].tile==j.Definition().goal);
        const int waiting=plan.assignments[0].tile;
        assert(j.world.MapDistance(waiting,plan.assignments[2].tile)==1 && waiting!=j.Definition().goal);
        for(int i=0;i<3;++i) assert(j.AssignmentProblem(i,plan.assignments[i].tile,plan.assignments[i].duty).empty() && !plan.assignments[i].completed);
        for(int i=0;i<20;++i) Same(plan,PlanExample(j));assert(Bytes(*s)==before);
        auto other=std::make_unique<StrategistJourney>(j);other->enemyPolicy=2;other->world.generals[3].leadership=1;for(auto& t:other->world.tiles) t.terrain=Terrain::Sea;Same(plan,PlanExample(*other));
        auto restored=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(before,*restored));Same(plan,PlanExample(restored->journey));assert(Bytes(*restored)==before);
        for(unsigned version:{1u,2u,3u,4u,5u,6u,7u,8u,9u})
        { auto old=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Old(*s,version),*old));assert(!old->journey.retryRelief && PlanExample(old->journey).reason.empty()); }
        const auto originalAssignments=j.assignments;const auto originalMessage=j.message;assert(j.ApplyPlanExample());
        // Strictly restore only duties/message; every resource, story and lesson byte stays unchanged.
        auto normalized=std::make_unique<JourneySaveData>(*s);normalized->journey.assignments=originalAssignments;normalized->journey.message=originalMessage;assert(Bytes(*normalized)==before);
        assert(j.AssignDuty(0,j.Definition().positions[0],JourneyDuty::Hold));assert(j.ApplyPlanExample());
        s->beforeBattle=j;const auto bill=j.PreparationCosts(1);assert(j.Prepare(1,1));
        assert(!j.retryRelief && j.heldTurns==0 && j.world.cities[0].food==camp.food-bill.TotalFood() && j.world.cities[0].gold==camp.gold-bill.gold);
        for(auto& a:j.assignments) assert(!a.completed);
        s->realtime.Reset();s->realtime.ApplyAssignments(j);assert(s->realtime.orders[0].destination==waiting && s->realtime.orders[2].destination==Campaign::At(8,9));
        j.world.armies[3].troops=j.world.armies[4].troops=0;j.RefreshIntelligence();
        for(int i=0;i<6 && j.stage==JourneyStage::Battle && !j.assignments[0].completed;++i) assert(s->realtime.Resolve(j));
        assert(j.assignments[0].completed && j.world.armies[0].tile==waiting && j.battleReview.relief.empty());
        assert(j.Withdraw() && j.RetryAtCouncil(*s->beforeBattle) && !j.retryRelief); // No relief in this attempt: no stale lesson is re-created.
    }
    // Version 9 already stored real battle relief: a newly chosen retry may use that evidence.
    auto oldAttempt=Attempt(true);auto oldDebrief=std::make_unique<JourneySaveData>();
    assert(DecodeJourneySave(Old(*oldAttempt,9),*oldDebrief) && !oldDebrief->journey.retryRelief);
    assert(oldDebrief->journey.RetryAtCouncil(*oldDebrief->beforeBattle) && oldDebrief->journey.retryRelief);
    assert(PlanExample(oldDebrief->journey).reason.find(U"前回の試み")!=std::u32string::npos);
    // Settled-history fixtures validate adaptation to another authored map, never a claimed playthrough.
    auto seed=Attempt(true);const auto evidence=seed->journey.battleReview;
    for(int mission:{1,2})
    {
        auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=mission;
        auto& h=j.history[mission-1];h.completed=h.won=true;h.review=evidence;
        const auto p=PlanExample(j);assert(!p.reason.empty() && p.reason.find(U"前の座標は流用しない")!=std::u32string::npos);
        assert(j.ApplyPlanExample());for(int i=0;i<3;++i) assert(j.AssignmentProblem(i,p.assignments[i].tile,p.assignments[i].duty).empty());
        h.review.reliefTracked=false;h.review.relief.clear();assert(PlanExample(j).reason.empty());
    }
    // Actor permutations and both beacon posts remain legal, including a successor starting on the post.
    for(int outgoing=0;outgoing<3;++outgoing) for(int incoming=0;incoming<3;++incoming) if(outgoing!=incoming)
        for(int source:{Campaign::At(7,6),Campaign::At(8,9)})
        {
            auto council=std::make_unique<StrategistJourney>(*seed->beforeBattle);
            auto r=evidence.relief[0];r.outgoing=outgoing;r.incoming=incoming;r.source=source;r.destination=source+1;council->retryRelief=r;
            const auto p=PlanExample(*council);assert(!p.reason.empty());
            for(int i=0;i<3;++i) assert(council->AssignmentProblem(i,p.assignments[i].tile,p.assignments[i].duty).empty());
            assert(council->world.MapDistance(p.assignments[incoming].tile,source)==1);
            council->retryRelief->source=Campaign::At(8,8);assert(PlanExample(*council).reason.empty()); // Do not abandon both objective posts for a non-objective lesson.
        }
    auto bad=Attempt(true);assert(bad->journey.RetryAtCouncil(*bad->beforeBattle));
    bad->journey.retryRelief->incoming=2;assert(!ValidJourneySave(*bad));
    bad=Attempt(true);assert(bad->journey.RetryAtCouncil(*bad->beforeBattle));bad->journey.stage=JourneyStage::Battle;assert(!ValidJourneySave(*bad));
    auto wrong=std::make_unique<StrategistJourney>(*bad->beforeBattle);wrong->mission=1;const auto untouched=Bytes(*seed);assert(!seed->journey.RetryAtCouncil(*wrong) && Bytes(*seed)==untouched);
    std::cout<<"Relief advice PASS: actual failed/recovered attempts, exact unpaid rollback plus lesson, immutable editable proposal, real paid deployment/arrival, no stale lesson, public terrain, old 1-9 and invalid lessons\n";
}
