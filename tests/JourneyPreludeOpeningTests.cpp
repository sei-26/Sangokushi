#include "JourneyPreludeOpening.hpp"
#include "JourneySave.hpp"
#include "JourneyAutoBattle.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
int main()
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;
    j.ResetCampaign();j.Meet();j.grandCampaign=j.shortPrelude=true;
    std::vector<std::uint8_t> before,after;assert(EncodeJourneySave(*s,before));
    assert(PreludeOpeningCouncil(j).beats.size()==3 && !MatchesPreludeOpening(j,0));
    PreludeOpening(j.preparation);PreludeOpeningCouncil(j,1);
    assert(EncodeJourneySave(*s,after) && before==after);
    assert(!AdoptPreludeOpening(j,-1) && !AdoptPreludeOpening(j,2));
    assert(EncodeJourneySave(*s,after) && before==after);
    for(int choice:{0,1})
    {
        assert(AdoptPreludeOpening(j,choice) && MatchesPreludeOpening(j,choice));s->preparation=choice;
        assert(j.world.cities[0].food==18000 && j.world.cities[0].gold==900 && j.world.cities[0].troops==6000 && j.trust==0);
        assert(EncodeJourneySave(*s,before));auto load=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(before,*load));
        assert(MatchesPreludeOpening(load->journey,choice) && load->preparation==choice);
        auto battle=std::make_unique<JourneySaveData>(*s);auto& b=battle->journey;b.SelectDifficulty(JourneyDifficulty::Story);
        const auto cost=b.PreparationCosts(choice);battle->beforeBattle=b;assert(b.Prepare(choice,0));
        assert(b.world.cities[0].food==18000-cost.TotalFood() && b.world.cities[0].gold==900-cost.gold);
        battle->realtime.Reset();battle->realtime.ApplyAssignments(b);
        if(choice==0) assert(b.chain.concealed[1] && b.assignments[0].duty==JourneyDuty::None);
        else assert(battle->realtime.orders[0].kind==JourneyDirective::March && battle->realtime.orders[0].destination==b.goal && !b.chain.concealed[1]);
        assert(!AdoptPreludeOpening(b,choice));
        for(int cycle=0;cycle<24 && b.stage==JourneyStage::Battle;++cycle)
        { auto p=PlanJourneyAutoBattle(b);if(!p.tactics.empty()) b.Execute(p.tactics);if(b.stage==JourneyStage::Battle) p.movement.Resolve(b); }
        assert(b.won && b.history[0].completed);
        const auto scene=PreludeOpeningAftermath(b);assert(scene.lines.size()==4 && scene.lines.back().text.find(U"黄巾")!=std::u32string::npos);
        assert(EncodeJourneySave(*battle,before));BuildJourneyEpisode(b);assert(EncodeJourneySave(*battle,after) && before==after);
        assert(b.Continue() && b.mission==YellowTurbanMission && !b.history[1].won && !b.history[2].won);
    }
    // A genuinely different manual win: Zhang Fei draws pursuit south while Liu Bei takes the exit.
    // No relocation, edited enemies, free damage or objective override.
    for(auto difficulty:{JourneyDifficulty::Story,JourneyDifficulty::Standard})
        for(int policy:{0,1,2})
        {
            auto exit=std::make_unique<JourneySaveData>();auto& b=exit->journey;b.ResetCampaign();b.Meet();b.SelectDifficulty(difficulty);
            assert(AdoptPreludeOpening(b,1));exit->preparation=1;exit->beforeBattle=b;assert(b.Prepare(1,policy));
            exit->realtime.ApplyAssignments(b);assert(exit->realtime.Queue(b,2,JourneyDirective::March,Campaign::At(12,11)));
            for(int cycle=0;cycle<12 && b.stage==JourneyStage::Battle;++cycle) assert(exit->realtime.Resolve(b));
            assert(b.won && b.battleReview.resolution==JourneyResolution::ExitSecured && b.world.armies[3].troops>450 && b.world.armies[0].tile==b.goal);
            assert(PreludeOpeningAftermath(b).lines[0].text.find(U"敵の旗は残っている")!=std::u32string::npos);
            assert(EncodeJourneySave(*exit,before));auto load=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(before,*load));
            assert(BuildJourneyEpisode(load->journey).lines[0].text==BuildJourneyEpisode(b).lines[0].text);
        }
    auto failed=std::make_unique<JourneySaveData>();auto& f=failed->journey;f.ResetCampaign();f.Meet();failed->beforeBattle=f;
    assert(f.Prepare(0,0) && f.Withdraw());
    assert(PreludeOpeningAftermath(f).lines[0].speaker==3 && PreludeOpeningAftermath(f).lines[0].text.find(U"開けなかった")!=std::u32string::npos);
    auto rescued=std::make_unique<JourneySaveData>();auto& r=rescued->journey;r.ResetCampaign();r.Meet();r.SelectDifficulty(JourneyDifficulty::Story);
    assert(r.SelectPromise(JourneyPromise::EscortRefugees));rescued->beforeBattle=r;assert(r.Prepare(1,0));
    for(int cycle=0;cycle<24 && r.stage==JourneyStage::Battle;++cycle)
    { auto p=PlanJourneyAutoBattle(r);if(!p.tactics.empty()) r.Execute(p.tactics);if(r.stage==JourneyStage::Battle) p.movement.Resolve(r); }
    assert(r.won && r.battleReview.resolution==JourneyResolution::Delivered && PreludeOpeningAftermath(r).lines[0].speaker==4);
    assert(j.SelectPromise(JourneyPromise::EscortRefugees));
    assert(!CanChoosePreludeOpening(j) && !AdoptPreludeOpening(j,0));
    assert(!BuildCouncilExchange(j,-1,0).beats.empty());
    j.SelectPromise(JourneyPromise::OpenRoad);j.world.cities[0].food=0;
    assert(EncodeJourneySave(*s,before) && !AdoptPreludeOpening(j,0));assert(EncodeJourneySave(*s,after) && before==after);
    std::cout<<"Prelude opening: paid actual plans, immutable reading, save/load, real victories and next historical battle pass\n";
}
