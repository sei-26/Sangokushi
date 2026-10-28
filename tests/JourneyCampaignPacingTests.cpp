#include "JourneyBattlePlayer.hpp"
#include "../NewGame/JourneyArchive.hpp"
#include "../NewGame/JourneyCampaign.hpp"
#include "../NewGame/JourneyPlanExample.hpp"
#include "../NewGame/JourneyEpisode.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static void AttackPort(StrategistJourney& j)
{
    while(j.stage==JourneyStage::Battle)
    {
        // Use observed contacts and public objective, never private enemy positions.
        std::vector<ChainOrder> combo;
        for(int army=0;army<3;++army)
            for(const auto& c:j.intelligence[0].contacts)
                if(c.current && j.world.CanStrike(army,c.observed.tile))
                { combo.push_back({army,c.id,army==2 ? ChainTactic::Breakthrough:ChainTactic::Disrupt,-1});break; }
        if(combo.size()>1) j.Execute(combo);
        for(int army=0;army<3 && j.stage==JourneyStage::Battle;++army)
        { Approach(j,army,j.goal,false);if(j.stage==JourneyStage::Battle) Fight(j,army,false); }
        if(j.stage==JourneyStage::Battle) j.EndTurn();
    }
    std::cout<<"port win="<<j.won<<" cycles="<<j.turn-1<<" loss="<<j.casualties<<" cooperation="<<j.battleReview.coordinatedOfficers<<"\n";
    assert(j.won && j.battleReview.resolution==JourneyResolution::Captured && j.world.tiles[j.goal].owner==j.world.player);
}
int main()
{
    std::cout<<std::unitbuf;
    for(auto pledge:{JourneyPledge::Preserve,JourneyPledge::Coordinate})
    {
        auto j=std::make_unique<StrategistJourney>();j->ResetCampaign();j->SelectDifficulty(JourneyDifficulty::Story);j->Meet();assert(j->Prepare(0,0));
        j->Execute({{0,3,ChainTactic::Lure,Campaign::At(8,11)},{1,3,ChainTactic::Ambush,-1},{2,3,ChainTactic::Breakthrough,-1}});
        if(j->stage==JourneyStage::Battle) j->EndTurn();
        if(j->stage==JourneyStage::Battle) j->Execute({{2,3,ChainTactic::Breakthrough,-1}});
        assert(j->won && j->CanContinue() && j->NeedsStoryReply());
        const int BeforeDeparture=j->world.day;
        assert(j->ChooseStoryReply(pledge) && j->Continue() && j->world.day==BeforeDeparture+7);
        assert(j->Definition().kind==JourneyObjective::CaptureCamp && !j->HasConvoy());
        assert(j->Definition().positions[0]==Campaign::At(9,pledge==JourneyPledge::Preserve ? 8:10));
        const auto councilScene=BuildJourneyEpisode(*j);
        assert(councilScene.lines[1].text.find(pledge==JourneyPledge::Preserve ? U"北橋":U"南橋")!=std::u32string::npos);
        assert(j->PreparationCosts(0).cargo==0 && j->PreparationCosts(0).escort==0);
        auto saved=std::make_unique<JourneySaveData>();saved->journey=*j;saved->beforeBattle=*j;
        assert(j->Prepare(1,1) && j->world.armies.size()==5);
        AttackPort(*j);saved->journey=*j;std::vector<std::uint8_t> bytes;
        assert(EncodeJourneySave(*saved,bytes) && bytes[4]==8);
        auto restored=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(bytes,*restored) && restored->journey.campaignFlow);
        const int PortDay=restored->journey.world.day;
        assert(restored->journey.CanContinue());assert(restored->journey.Continue() && restored->journey.world.day==PortDay+21);
        assert(restored->journey.mission==2 && restored->journey.RequiredHoldTurns()==2);
        auto& defense=restored->journey;assert(defense.Prepare(1,0));assert(defense.Move(0,defense.goal));
        for(int i=0;i<2 && defense.stage==JourneyStage::Battle;++i) { assert(defense.Hold(0) && defense.Hold(2));defense.EndTurn(); }
        assert(defense.won && CampaignBeat(defense).change.find(U"正式")!=std::u32string::npos);
        assert(defense.Continue() && defense.stage==JourneyStage::Complete);
    }
    auto base=std::make_unique<StrategistJourney>();base->ResetCampaign();base->Meet();base->mission=1;
    const int normal=base->TurnBudget();base->history[0].rescued=true;assert(base->TurnBudget()==normal+2);
    base->history[0].rescued=false;base->mission=2;assert(base->RequiredHoldTurns()==3);
    base->history[1].won=true;base->history[1].review.coordinatedOfficers=1;assert(base->RequiredHoldTurns()==3);
    base->history[1].review.coordinatedOfficers=3;assert(base->RequiredHoldTurns()==2);base->history[1]={};
    base->mission=1;assert(base->Prepare(1,0));
    base->world.armies[3].troops=base->world.armies[4].troops=0; // Isolate the capture condition, not a playthrough.
    base->RefreshIntelligence();assert(base->EndTurn() && base->stage==JourneyStage::Battle);
    while(base->stage==JourneyStage::Battle)
    {
        JourneyRealtime march;assert(march.Queue(*base,0,JourneyDirective::March,base->goal));
        const auto path=march.Route(*base,0);assert(path.size()>1);
        base->Move(0,path[1]);if(base->stage==JourneyStage::Battle) base->EndTurn();
    }
    assert(base->won && base->battleReview.resolution==JourneyResolution::Captured);
    auto legacy=std::make_unique<StrategistJourney>();legacy->Reset();legacy->Meet();legacy->mission=1;assert(legacy->HasConvoy() && legacy->Definition().kind==JourneyObjective::Escort);
    auto old=std::make_unique<JourneySaveData>();old->journey=*legacy;old->beforeBattle=*legacy;
    assert(legacy->Prepare(0,1) && !legacy->UpgradeCampaignAtCouncil());
    old->journey=*legacy;
    std::vector<std::uint8_t> bytes;JourneyArchive writer{false,bytes};writer.version=5;
    unsigned magic=0x53594e47,version=5,checksum=0;writer(magic,version,checksum);writer(*old);
    checksum=2166136261u;for(std::size_t i=12;i<bytes.size();++i) { checksum^=bytes[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) bytes[8+i]=static_cast<std::uint8_t>(checksum>>(i*8));
    auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(bytes,*loaded) && !loaded->journey.campaignFlow && loaded->journey.HasConvoy()); // In-flight old saves cannot change rules.
    std::cout<<"Campaign pacing PASS: actual first battle, dialogue deployment branches, capture instead of convoy, coordinated victory changes final battle, immediate objective end, saves and legacy objectives\n";
}
