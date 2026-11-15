#include "JourneyCouncilStage.hpp"
#include "JourneySave.hpp"
#include "JourneyBattlefield.hpp"
#include <cassert>
#include <memory>
using namespace frontline;
int main()
{
    auto save=std::make_unique<JourneySaveData>();save->journey.ResetCampaign();save->journey.Meet();
    auto& j=save->journey;std::vector<std::uint8_t> before,after;assert(EncodeJourneySave(*save,before));
    for(int m=0;m<3;++m)
    {
        j.mission=m;
        const auto brief=BuildCouncilBattleBrief(j);
        assert(brief.title && brief.opening);
        for(const auto& role:brief.roles)
        {
            assert(role.title && role.advice);
            assert(j.PlanningTerrain(role.tile)!=Terrain::River && j.PlanningTerrain(role.tile)!=Terrain::Mountain);
        }
        for(int who=-1;who<3;++who)
        {
            auto e=BuildCouncilExchange(j,who,0);assert(!e.beats.empty());
            for(const auto& b:e.beats) assert(b.dialogue.speaker>=0 && b.dialogue.speaker<=3 && !b.dialogue.text.empty());
            if(who==-1) assert(e.beats.size()==1 && e.beats.front().dialogue.text==brief.opening);
            if(who>=0)
                assert(e.beats.size()==2 && e.beats.front().dialogue.text==brief.roles[who].advice);
        }
        if(m==1) assert(j.PlanningTerrain(brief.roles[2].tile)==Terrain::Bridge);
        if(m==2) assert(brief.roles[0].tile==j.Definition().goal && brief.roles[2].tile==j.Definition().secondGoal);
    }
    j.mission=1;
    j.history[0].story.recorded=true;
    j.history[0].story.reply=JourneyPledge::Preserve;
    const auto north=BuildCouncilBattleBrief(j);
    assert(j.Battlefield()==JourneyMap::PortNorth && north.roles[2].tile==Campaign::At(11,8));
    assert(j.PlanningTerrain(north.roles[2].tile)==Terrain::Bridge);
    assert(BuildCouncilExchange(j,2,0,CouncilDecision::Ambush).beats.size()==2);
    assert(BuildCouncilExchange(j,2,1,CouncilDecision::Reserves).beats.size()==2);
    j.history[0]={};
    j.mission=2;
    j.history[1].won=true;
    j.history[1].review.coordinatedOfficers=3;
    assert(j.RequiredHoldTurns()==2 && std::u32string_view(BuildCouncilBattleBrief(j).opening).find(U"二指揮")!=std::u32string_view::npos);
    j.history[1]={};
    j.mission=0;assert(EncodeJourneySave(*save,after));assert(before==after);
    const auto guan=BuildCouncilExchange(j,1,0),zhang=BuildCouncilExchange(j,2,0);
    assert(guan.subject!=zhang.subject && guan.beats.front().dialogue.speaker==1 && zhang.beats.front().dialogue.speaker==2);
    assert(guan.beats[1].dialogue.warmMask==7 && zhang.beats[0].dialogue.warmMask==7);
    const int food=j.world.cities[0].food,gold=j.world.cities[0].gold,trust=j.trust;
    auto adopted=BuildCouncilExchange(j,1,0,CouncilDecision::Ambush);assert(adopted.beats.front().dialogue.speaker==3);
    bool other=false;for(const auto& b:adopted.beats)other|=b.dialogue.speaker==2;assert(other);
    assert(BuildCouncilExchange(j,1,1,CouncilDecision::Ambush).beats.empty());
    assert(BuildCouncilExchange(j,0,0,CouncilDecision::Escort).beats.empty());
    assert(j.SelectPromise(JourneyPromise::EscortRefugees));assert(!BuildCouncilExchange(j,0,0,CouncilDecision::Escort).beats.empty());
    const auto road=BuildCouncilBattleBrief(j);
    assert(road.roles[0].tile==j.ConvoyPath().back() && road.roles[2].tile==j.ConvoyPath()[3]);
    assert(j.SelectConvoyRoute(JourneyConvoyRoute::SouthernWood));
    const auto wood=BuildCouncilBattleBrief(j);
    assert(wood.roles[2].tile==j.ConvoyPath()[3] && wood.roles[2].tile!=road.roles[2].tile);
    assert(BuildCouncilExchange(j,2,0).beats.front().dialogue.text==wood.roles[2].advice);
    assert(j.SelectConvoyRoute(JourneyConvoyRoute::Road));
    assert(j.world.cities[0].food==food && j.world.cities[0].gold==gold && j.trust==trust);
    // Only the existing authoritative deployment pays the selected preparation and escort.
    const auto cost=j.PreparationCosts(1);assert(j.Prepare(1,0));assert(j.reserveFood==1800 && j.HasConvoy());
    assert(j.world.cities[0].food==food-cost.TotalFood() && j.world.cities[0].gold==gold-cost.gold);
    const auto first=j.Execute({{0,3,ChainTactic::Feint,-1}});
    assert(first.completed==1 && CouncilBattleVoice(j,first.events[0],0).empty());
    j.chain.BeginTurn(j.world);assert(j.Resupply());
    const auto supplied=j.Execute({{0,3,ChainTactic::Feint,-1}});
    assert(supplied.completed==1 && !CouncilBattleVoice(j,supplied.events[0],0).empty());
    auto failed=supplied.events[0];failed.success=false;assert(CouncilBattleVoice(j,failed,0).empty());
    j.chain.BeginTurn(j.world);const auto later=j.Execute({{0,3,ChainTactic::Feint,-1}});
    assert(later.completed==1 && CouncilBattleVoice(j,later.events[0],0).empty());
    assert(BuildCouncilExchange(j,0,1).beats.empty());
    j.stage=JourneyStage::Preparation;j.mission=1;j.history[0].completed=true;j.history[0].casualties=1200;j.history[0].survivors=3000;
    const auto costly=BuildCouncilExchange(j,-1,0);assert(costly.beats.front().dialogue.speaker==1);for(const auto& beat:costly.beats) assert(beat.dialogue.warmMask==0);
    const auto a=BuildCouncilExchange(j,1,0);j.enemyPolicy=2;const auto b=BuildCouncilExchange(j,1,0);
    assert(a.beats.size()==b.beats.size());for(std::size_t n=0;n<a.beats.size();++n)assert(a.beats[n].dialogue.text==b.beats[n].dialogue.text);
}
