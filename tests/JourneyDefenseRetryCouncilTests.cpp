#include "../NewGame/JourneyCouncilStage.hpp"
#include "../NewGame/JourneyPlanExample.hpp"
#include "../NewGame/JourneySave.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
static std::unique_ptr<JourneySaveData> Attempt(int guard)
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;
    j.ResetCampaign();j.Meet();j.mission=2;s->beforeBattle=j;s->realtimeMode=true;
    assert(j.Prepare(1,0)); // Authored contact boundary after ordinary paid deployment.
    for(int y=1;y<StrategistJourney::Rows-1;++y) for(int x=1;x<StrategistJourney::Columns-1;++x) j.world.tiles[Campaign::At(x,y)].terrain=Terrain::Plain;
    const int support=guard==0 ? 1:0,other=3-guard-support;
    const auto place=[&](int a,int tile) { j.world.armies[a].tile=j.world.armies[a].target=tile; };
    place(guard,j.goal);place(support,Campaign::At(6,6));place(other,j.Definition().secondGoal);place(3,Campaign::At(6,5));
    j.world.armies[3].troops=3000;j.world.armies[4].troops=0;j.chain.Reset(j.world);j.RefreshIntelligence();
    for(int a=0;a<3;++a) assert(s->realtime.Queue(j,a,JourneyDirective::Hold));
    assert(j.Execute({{support,3,ChainTactic::Feint,-1},{guard,3,ChainTactic::Disrupt,-1}}).completed==2);
    assert(s->realtime.Resolve(j) && (j.battleReview.spentGuardFailures&(1<<guard)) && j.Withdraw());
    assert(j.RetryAtCouncil(*s->beforeBattle) && j.retryDefense);return s;
}
int main()
{
    for(int free=0;free<3;++free)
    {
        auto s=Attempt(free);auto& j=s->journey;const auto before=Bytes(*s);const auto p=PlanExample(j);
        assert(p.remembersDefenseRetry && p.adviser==free && p.assignments[free].duty==JourneyDuty::None);
        const auto e=BuildCouncilExchange(j,-1,0);
        assert(e.subject==p.name && e.beats.size()==4 && e.beats[0].dialogue.speaker==free && e.beats[0].dialogue.text==p.offer);
        const int guard=e.beats[1].dialogue.speaker;assert(guard!=free && p.assignments[guard].duty==JourneyDuty::Hold);
        assert(e.beats[2].dialogue.speaker==3 && e.beats[2].dialogue.text.find(U"地図で見せて")!=std::u32string::npos);
        assert(e.beats[3].dialogue.speaker==free && e.beats[3].dialogue.text.find(free==2 ? U"採るって決まってから":U"採る")!=std::u32string::npos);
        if(free==0) assert(e.beats[1].dialogue.text.find(U"兄者、この案なら")!=std::u32string::npos && e.beats[1].dialogue.text.find(U"預かりましょう")!=std::u32string::npos);
        for(const auto& b:e.beats) { assert(!b.dialogue.text.empty() && b.dialogue.text.find(U"前戦の勝利")==std::u32string::npos);assert(b.dialogue.text.find(U"前は北")==std::u32string::npos); }
        for(int n=0;n<5;++n) BuildCouncilExchange(j,-1,0);assert(Bytes(*s)==before);
        auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(before,*loaded));
        const auto again=BuildCouncilExchange(loaded->journey,-1,0);assert(again.subject==e.subject && again.beats.size()==4);
        for(int n=0;n<4;++n) assert(again.beats[n].dialogue.text==e.beats[n].dialogue.text);
        const auto chosen=BuildCouncilExchange(j,-1,0,CouncilDecision::Ambush);assert(chosen.beats[0].dialogue.speaker==3 && chosen.subject!=p.name);
        j.enemyPolicy=2;const auto privateChange=BuildCouncilExchange(j,-1,0);assert(privateChange.subject==e.subject);
        for(int n=0;n<4;++n) assert(privateChange.beats[n].dialogue.text==e.beats[n].dialogue.text);
        j.retryDefense.reset();assert(!PlanExample(j).remembersDefenseRetry && BuildCouncilExchange(j,-1,0).subject!=e.subject);
        j.stage=JourneyStage::Battle;assert(BuildCouncilExchange(j,-1,0).beats.empty());
    }
    std::cout<<"Defense retry council PASS: real failed attempt, affected officer freed for support, distinct guard voice, explicit map review before adoption, unchanged save bytes, reload and no hidden-knowledge or canonical victory claim\n";
}
