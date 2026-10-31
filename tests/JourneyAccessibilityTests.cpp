#include "JourneyTestFlow.hpp"
#include "../NewGame/JourneyArchive.hpp"
#include "../NewGame/JourneyPlanExample.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::unique_ptr<StrategistJourney> Council(int mission,JourneyDifficulty difficulty)
{ auto j=std::make_unique<StrategistJourney>();j->Reset();assert(j->SelectDifficulty(difficulty));j->Meet();j->mission=mission;return j; }
static void Round(JourneySaveData& s)
{
    std::vector<std::uint8_t> bytes;assert(EncodeJourneySave(s,bytes) && bytes[4]==10);
    auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(bytes,*loaded));
    assert(loaded->journey.difficulty==s.journey.difficulty && loaded->journey.turn==s.journey.turn);
    assert(loaded->journey.assignments[1].tile==s.journey.assignments[1].tile);
}
static void SoloOpening(int policy)
{
    auto j=Council(0,JourneyDifficulty::Story);assert(j->Prepare(1,policy));
    j->EndTurn();j->EndTurn(); // Two missed chances to attack; no artificial enemy or resource changes.
    while(j->stage==JourneyStage::Battle)
    {
        for(int army=0;army<3 && j->stage==JourneyStage::Battle;++army)
        {
            if(!TacticChain::Living(j->world,army)) continue;
            auto contacts=j->intelligence[0].contacts;int target=-1;
            for(const auto& c:contacts) if(c.current && c.observed.troops>0 && (target<0 || c.id==3)) target=c.id;
            if(target<0) continue;
            JourneyRealtime route;route.Queue(*j,army,JourneyDirective::Attack,target);const auto path=route.Route(*j,army);
            if(path.size()>1) j->Move(army,path[1]);
            if(j->stage!=JourneyStage::Battle) break;
            const auto tactic=army==0 ? ChainTactic::Feint:army==1 ? ChainTactic::Ambush:ChainTactic::Breakthrough;
            j->Execute({{army,target,tactic,-1}}); // No combo, signatures or memorized opening.
        }
        if(j->stage==JourneyStage::Battle) j->EndTurn();
    }
    std::cout<<"Story solo opening policy "<<policy<<": win="<<j->won<<" cycles="<<j->turn-1<<" casualties="<<j->casualties<<"\n";
    assert(j->won);
}
int main()
{
    for(int mission=0;mission<3;++mission) for(int policy=0;policy<3;++policy)
    {
        auto story=Council(mission,JourneyDifficulty::Story),standard=Council(mission,JourneyDifficulty::Standard);
        if(mission==0) { story->SelectPromise(JourneyPromise::EscortRefugees);standard->SelectPromise(JourneyPromise::EscortRefugees); }
        const auto camp=story->world.cities[0];const auto trust=story->officerTrust;
        const auto example=PlanExample(*story);assert(!example.manualTask.empty());
        assert(story->ApplyPlanExample() && standard->ApplyPlanExample());
        assert(story->world.cities[0].gold==camp.gold && story->world.cities[0].food==camp.food && story->officerTrust==trust);
        for(int i=0;i<3;++i) assert(!story->assignments[i].completed);
        auto save=std::make_unique<JourneySaveData>();save->journey=*story;save->beforeBattle=*story;Round(*save);
        assert(story->Prepare(1,policy) && standard->Prepare(1,policy));
        assert(!story->SelectDifficulty(JourneyDifficulty::Standard) && !story->ApplyPlanExample());
        assert(story->TurnBudget()==standard->TurnBudget()+3);
        for(int i:{3,4}) assert(story->world.armies[i].troops==standard->world.armies[i].troops*80/100);
        for(int i=0;i<3;++i) assert(story->world.armies[i].troops==standard->world.armies[i].troops && story->world.armies[i].morale==standard->world.armies[i].morale);
        assert(story->world.cities[0].food==standard->world.cities[0].food && story->world.cities[0].gold==standard->world.cities[0].gold);
        save->journey=*story;save->realtime.ApplyAssignments(*story);Round(*save);
        assert(story->Withdraw());save->journey=*story;Round(*save);
        assert(story->history[mission].difficulty==JourneyDifficulty::Story);
        save->journey=*save->beforeBattle;assert(save->journey.world.cities[0].food==camp.food && save->journey.difficulty==JourneyDifficulty::Story);
    }
    // A later Story turn and late notices must be saveable, including a change back to Standard at the retry council.
    auto s=std::make_unique<JourneySaveData>();s->journey=*Council(1,JourneyDifficulty::Story);s->beforeBattle=s->journey;assert(s->journey.Prepare(1,1));
    s->journey.turn=15;
    auto before=JourneyBattleWatch::Capture(s->journey,s->realtime),after=before;before.turn=14;after.units[0].troops=400;
    assert(!s->watch.Record(before,after).empty());Round(*s);
    s->journey=*s->beforeBattle;assert(s->journey.SelectDifficulty(JourneyDifficulty::Standard));Round(*s);
    // The v4 wire layout has no difficulty. Legacy troops, progress and Story outcomes are not rewritten.
    s->watch.Reset();
    std::vector<std::uint8_t> bytes;JourneyArchive writer{false,bytes};writer.version=4;
    unsigned magic=0x53594e47,version=4,checksum=0;writer(magic,version,checksum);writer(*s);
    checksum=2166136261u;for(std::size_t i=12;i<bytes.size();++i) { checksum^=bytes[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) bytes[8+i]=static_cast<std::uint8_t>(checksum>>(i*8));
    auto legacy=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(bytes,*legacy) && legacy->journey.difficulty==JourneyDifficulty::Standard);
    for(int policy=0;policy<3;++policy) SoloOpening(policy);
    std::cout<<"Accessibility PASS: actual opposition/deadline, unpaid template, real-role deploy, late-turn saves, Standard compatibility, retry and v4 migration\n";
}
