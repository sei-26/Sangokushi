#include "JourneyStrategicResponse.hpp"
#include "JourneyAutoBattle.hpp"
#include "JourneySave.hpp"
#include "JourneyEpisode.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::unique_ptr<JourneySaveData> Start()
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;
    j.ResetCampaign();j.Meet();j.mission=YellowTurbanMission;
    j.grandCampaign=j.grandStrategyCampaign=j.coalitionBattleCampaign=true;
    for(int m=0;m<YellowTurbanMission;++m) j.history[m].completed=j.history[m].won=j.history[m].campaignFlow=true;
    j.SelectDifficulty(JourneyDifficulty::Story);s->beforeBattle=j;assert(j.Prepare(1,0));return s;
}
static JourneyEnemyPlan Reserve(const StrategistJourney& j)
{
    for(const auto& p:j.ReadEnemyPlans()) if(p.army==4) return p;
    assert(false);return {};
}
static void Fight(StrategistJourney& j)
{
    for(int cycle=0;cycle<50 && j.stage==JourneyStage::Battle;++cycle)
    {
        auto p=PlanJourneyAutoBattle(j);
        if(!p.tactics.empty()) j.Execute(p.tactics);
        if(j.stage==JourneyStage::Battle) assert(p.movement.Resolve(j));
    }
    if(!j.won)
    {
        std::cerr<<"FAILED mission="<<j.mission<<" stage="<<int(j.stage)<<" turn="<<j.turn<<" plan="<<int(j.coalitionPlan)<<" goal="<<j.goal<<" rescued="<<j.battleReview.grand.coalition.rescuedOn<<"\n";
        for(int a=0;a<int(j.world.armies.size());++a)
        { const auto& u=j.world.armies[a];std::cerr<<a<<" troops="<<u.troops<<" at="<<u.tile%Width<<","<<u.tile/Width<<" morale="<<u.morale<<"\n"; }
    }
    assert(j.won);
}
int main()
{
    auto s=Start();auto& j=s->journey;
    j.world.armies[0].tile=j.world.armies[0].target=Campaign::At(10,8);
    j.world.armies[2].tile=j.world.armies[2].target=Campaign::At(10,9);
    j.world.armies[1].tile=j.world.armies[1].target=Campaign::At(3,5);
    j.RefreshIntelligence();
    const auto actual=Reserve(j);
    assert(actual.role==JourneyEnemyRole::Counterattack && (actual.targetArmy==6 || actual.target==GrandNorthPass) && actual.next!=actual.from);
    // An exposed flank is derived from the observer's board, not private units.
    auto seen=std::make_unique<Campaign>(j.world);
    JourneyEnemyPlan base;base.army=4;base.from=base.next=seen->armies[4].tile;
    std::vector<JourneyEnemyPlan> plans{base};
    ApplyGuangzongFlankResponse(*seen,j.chain,j.goal,plans);assert(plans[0].role==JourneyEnemyRole::Counterattack);
    seen->armies[1].tile=GrandNorthPass;plans={base};
    ApplyGuangzongFlankResponse(*seen,j.chain,j.goal,plans);assert(plans[0].next==base.from);
    seen->armies[1].troops=0;seen->armies[0].troops=seen->armies[2].troops=0;plans={base};
    ApplyGuangzongFlankResponse(*seen,j.chain,j.goal,plans);assert(plans[0].next==base.from);
    *seen=j.world;auto chain=j.chain;chain.disordered[4]=true;plans={base};
    ApplyGuangzongFlankResponse(*seen,chain,j.goal,plans);assert(plans[0].next==base.from);
    // Real movement still obeys hidden physical occupancy and cannot teleport.
    const int from=j.world.armies[4].tile,turn=j.turn;
    auto clear=std::make_unique<JourneySaveData>(*s);
    j.world.armies[1].tile=j.world.armies[1].target=actual.next;
    assert(j.EndTurn() && j.world.armies[4].tile!=j.world.armies[1].tile && j.world.MapDistance(from,j.world.armies[4].tile)<=1);
    assert(clear->journey.EndTurn() && clear->journey.world.armies[4].tile!=from && clear->journey.turn==turn+1);
    const auto names=RemainingOfficers(j,{{0,3,ChainTactic::Feint,-1}});
    assert(names.find(j.world.generals[0].name)==std::u32string::npos && names.find(j.world.generals[1].name)!=std::u32string::npos);
    assert(RemainingOfficers(j,{{0,3,ChainTactic::Feint,-1},{1,3,ChainTactic::Ambush,-1},{2,3,ChainTactic::Breakthrough,-1}})==U"なし");

    // Earn a real relief result, carry it through the real chapter transition,
    // and verify an actual saved allied company follows its rescuer.
    s=Start();auto& g=s->journey;assert(g.ChooseGrandApproach(GrandApproach::RelieveLeft));Fight(g);
    const auto result=g.history[YellowTurbanMission].review.grand;
    assert(result.relievedOn>0 && result.leftReturned>0 && result.reliefOfficer>=0);
    auto earned=std::make_unique<StrategistJourney>(g);
    assert(g.Continue() && g.CanChooseCoalitionPlan() && g.ChooseCoalitionPlan(CoalitionPlan::Protect));
    const auto council=BuildJourneyEpisode(g);
    bool memory=false;for(const auto& line:council.lines) memory|=line.text.find(U"広宗")!=std::u32string::npos;
    assert(memory);s->beforeBattle=g;assert(g.Prepare(1,0) && HasGuangzongVeterans(g));
    assert(g.world.armies[7].troops==3200+result.leftReturned && g.world.armies.size()==GrandBattleArmyCount);
    const int rescuer=GuangzongVeteranOfficer(g);const int origin=g.world.armies[7].tile;
    g.world.armies[rescuer].tile=g.world.armies[rescuer].target=Campaign::At(6,7);g.RefreshIntelligence();
    bool follows=false;for(const auto& p:g.PlanCoalitionAllies()) if(p.army==7)
        follows=p.target==g.world.armies[rescuer].tile && p.next!=origin && !p.occupyTarget;
    assert(follows);
    std::vector<std::uint8_t> bytes;auto restored=std::make_unique<JourneySaveData>();
    assert(EncodeJourneySave(*s,bytes) && DecodeJourneySave(bytes,*restored) && HasGuangzongVeterans(restored->journey));
    assert(restored->journey.world.armies[7].troops==3200+result.leftReturned);
    auto old=std::make_unique<JourneySaveData>(*restored);
    old->journey.world.generals[old->journey.world.armies[7].general].name=U"連合軍・後陣";
    assert(!HasGuangzongVeterans(old->journey));
    // Both coalition plans remain playable with the earned support; the army
    // is never a mandatory victory condition or an extra unit.
    Fight(restored->journey);
    assert(earned->Continue() && earned->ChooseCoalitionPlan(CoalitionPlan::Advance) && earned->Prepare(1,0));
    assert(HasGuangzongVeterans(*earned));Fight(*earned);
    std::cout<<"Strategic response: observed concentration, north cover, disorder, collision, paid action reserves, actual relief, chapter memory, veteran company, save and real victory passed\n";
}
