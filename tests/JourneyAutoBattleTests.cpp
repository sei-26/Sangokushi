#include "JourneyAutoBattle.hpp"
#include "JourneySave.hpp"
#include "JourneyArchive.hpp"
#include "JourneyEpisode.hpp"
#include "JourneyCampaign.hpp"
#include "JourneyBattlefield.hpp"
#include "JourneyPlanExample.hpp"
#include <cassert>
#include <memory>
#include <iostream>
using namespace frontline;
int main()
{
    auto j=std::make_unique<StrategistJourney>();j->ResetCampaign();j->Meet();
    auto beforeBattle=std::make_unique<StrategistJourney>(*j);assert(j->Prepare(0,0));
    auto session=std::make_unique<JourneySaveData>();session->journey=*j;session->beforeBattle=*beforeBattle;
    std::vector<std::uint8_t> before,after;assert(EncodeJourneySave(*session,before));
    auto p=PlanJourneyAutoBattle(*j);session->journey=*j;assert(EncodeJourneySave(*session,after));assert(before==after);
    assert(!p.tactics.empty());const auto predicted=j->Preview(p.tactics);assert(predicted.completed==int(p.tactics.size()));
    auto fog=std::make_unique<StrategistJourney>(*j);
    for(auto& c:fog->intelligence[0].contacts)c.current=false;
    for(int a=3;a<int(fog->world.armies.size());++a) if(fog->world.Hostile(0,fog->world.armies[a].faction)) fog->world.armies[a].tile=Campaign::At(14,2);
    const auto hidden=PlanJourneyAutoBattle(*fog);assert(hidden.tactics.empty());
    for(const auto& order:hidden.movement.orders) assert(order.kind!=JourneyDirective::Attack);
    // The same visible observation produces identical decisions despite private enemy displacement.
    auto unseen=std::make_unique<StrategistJourney>(*fog);unseen->world.armies[3].tile=Campaign::At(13,3);
    const auto hidden2=PlanJourneyAutoBattle(*unseen);
    for(int a=0;a<3;++a) assert(SameJourneyOrder(hidden.movement.orders[a],hidden2.movement.orders[a]));
    const int turn=j->turn,food=j->world.armies[0].food;
    auto result=j->Execute(p.tactics);assert(result.completed>0);assert(j->chain.spent[result.events[0].army]);
    if(j->stage==JourneyStage::Battle) { assert(p.movement.Resolve(*j));assert(j->turn==turn+1);assert(j->world.armies[0].food<=food); }
    auto hold=std::make_unique<StrategistJourney>();hold->Reset();hold->mission=2;hold->Meet();assert(hold->Prepare(0,0));
    hold->world.armies[0].tile=hold->goal;hold->world.armies[1].tile=hold->Definition().secondGoal;hold->RefreshIntelligence();
    const auto guards=PlanJourneyAutoBattle(*hold);assert(guards.movement.orders[0].kind==JourneyDirective::Hold);assert(guards.movement.orders[1].kind==JourneyDirective::Hold);
    for(const auto& t:guards.tactics) assert(t.army==2);
    // A bridgehead fights beside the bridge rather than holding up its allies.
    auto bridge=std::make_unique<StrategistJourney>();bridge->ResetCampaign();bridge->mission=1;bridge->Meet();assert(bridge->Prepare(1,0));
    bridge->world.armies[0].tile=bridge->world.armies[0].target=Campaign::At(11,11);
    bridge->world.armies[1].tile=bridge->world.armies[1].target=Campaign::At(9,10);
    bridge->world.armies[2].tile=bridge->world.armies[2].target=Campaign::At(9,11);bridge->RefreshIntelligence();
    auto crossing=PlanJourneyAutoBattle(*bridge);const auto route=crossing.movement.Route(*bridge,0);
    assert(crossing.movement.orders[0].kind==JourneyDirective::March && route.size()==2);
    assert(bridge->world.tiles[route[1]].terrain!=Terrain::Bridge && route[1]%Width>=12);
    std::vector<int> arrivals;
    for(int a=0;a<3;++a)
    {
        const auto path=crossing.movement.Route(*bridge,a);if(path.size()<2) continue;
        assert(std::find(arrivals.begin(),arrivals.end(),path[1])==arrivals.end());arrivals.push_back(path[1]);
    }
    assert(crossing.movement.Resolve(*bridge) && bridge->world.armies[0].tile==route[1]);
    // Exercise actual autonomous engagements, not a fake result shortcut.
    for(int mission=0;mission<3;++mission)
    {
        auto battle=std::make_unique<StrategistJourney>();battle->ResetCampaign();battle->mission=mission;battle->Meet();
        battle->SelectDifficulty(JourneyDifficulty::Story);assert(battle->Prepare(1,0));
        int actions=0;
        for(int cycle=0;cycle<18 && battle->stage==JourneyStage::Battle;++cycle)
        {
            auto next=PlanJourneyAutoBattle(*battle);
            if(!next.tactics.empty()) { const auto result=battle->Execute(next.tactics);actions+=result.completed; }
            if(battle->stage==JourneyStage::Battle) assert(next.movement.Resolve(*battle));
        }
        assert(battle->stage==JourneyStage::Debrief);assert(battle->won);
        std::cerr<<"auto mission "<<mission<<" won="<<battle->won<<" survivors="<<battle->survivors<<"\n";
    }
    // A complete new-player campaign carries its real losses/resources forward
    // and uses the next council's default enemy policy, rather than resetting battles.
    auto career=std::make_unique<StrategistJourney>();career->ResetCampaign();career->SelectDifficulty(JourneyDifficulty::Story);career->Meet();
    assert(BuildCoalitionEpisode(*career).lines.empty());
    for(int mission=0;mission<3;++mission)
    {
        assert(career->mission==mission && career->Prepare(1,career->enemyPolicy));
        for(int cycle=0;cycle<18 && career->stage==JourneyStage::Battle;++cycle)
        {
            auto next=PlanJourneyAutoBattle(*career);
            if(!next.tactics.empty()) career->Execute(next.tactics);
            if(career->stage==JourneyStage::Battle) assert(next.movement.Resolve(*career));
        }
        std::cerr<<"campaign auto mission="<<mission<<" policy="<<career->enemyPolicy<<" won="<<career->won<<" troops="<<career->world.cities[0].troops<<" food="<<career->world.cities[0].food<<"\n";
        assert(career->won && career->CanContinue());
        if(career->NeedsStoryReply()) assert(career->ChooseStoryReply(JourneyPledge::Coordinate));
        assert(career->Continue());
    }
    assert(career->stage==JourneyStage::Complete && career->history[0].won && career->history[1].won && career->history[2].won);
    assert(CampaignBeat(*career).phase.find(U"序章 完")!=std::u32string::npos);
    const auto ending=BuildJourneyEpisode(*career);
    assert(!ending.lines.empty() && ending.lines.back().direction.find(U"序章、完")!=std::u32string::npos);
    assert(BuildCoalitionEpisode(*career).lines.empty() && career->CanBeginYellowTurban());

    auto old=std::make_unique<JourneySaveData>();
    old->journey=*career;
    std::vector<std::uint8_t> previous;
    JourneyArchive writer{false,previous};writer.version=12;
    unsigned magic=0x53594e47,version=12,checksum=0;
    writer(magic,version,checksum);writer(*old);
    checksum=2166136261u;
    for(std::size_t i=12;i<previous.size();++i) { checksum^=previous[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) previous[8+i]=static_cast<std::uint8_t>(checksum>>(i*8));
    assert(DecodeJourneySave(previous,*old) && old->journey.CanBeginYellowTurban());
    const auto camp=old->journey.world.cities[0];
    const auto trust=old->journey.officerTrust;
    assert(!old->journey.history[YellowTurbanMission].completed && old->journey.Continue());
    assert(old->journey.stage==JourneyStage::Preparation && old->journey.mission==YellowTurbanMission);
    assert(old->journey.world.cities[0].troops==camp.troops && old->journey.world.cities[0].food==camp.food && old->journey.officerTrust==trust);
    assert(BuildJourneyEpisode(old->journey).lines[0].text.find(U"鄒靖")!=std::u32string::npos);
    assert(BuildCoalitionEpisode(old->journey).lines.empty() && !old->journey.CanContinue());
    auto& previousReview=old->journey.history[YellowTurbanMission-1].review;
    previousReview.relief.push_back({2,0,Campaign::At(8,9),Campaign::At(9,9),1,1,2,3,1,true,true,JourneyReliefOutcome::Recovered});
    assert(PlanExample(old->journey).name.find(U"前陣")!=std::u32string::npos);
    previousReview.relief.pop_back();
    assert(old->journey.ApplyPlanExample());
    old->beforeBattle=old->journey;
    assert(old->journey.Prepare(1,old->journey.enemyPolicy) && old->journey.goal==old->journey.Definition().goal && !old->journey.HasConvoy());
    assert(old->journey.world.generals[3].name==EpisodeSpeakerName(ZhangLiang) &&
        old->journey.world.generals[3].trait==Trait::Resolute && old->journey.world.generals[3].tactic==Tactic::Rally);
    {
        auto pressure=std::make_unique<StrategistJourney>(old->journey);
        auto& front=pressure->world.armies[3];
        pressure->world.armies[0].tile=Campaign::At(11,9);
        front.tile=Campaign::At(12,8);
        front.morale=75;
        pressure->RefreshIntelligence();
        auto disrupted=std::make_unique<StrategistJourney>(*pressure);
        disrupted->chain.disordered[3]=true;
        assert(disrupted->EndTurn() && disrupted->world.tacticEvents.empty());
        assert(pressure->EndTurn() &&
            pressure->world.tacticEvents.size()==1 && pressure->world.tacticEvents[0].general==3);
        assert(pressure->world.armies[3].morale>disrupted->world.armies[3].morale);
        assert(pressure->EndTurn() && pressure->world.tacticEvents.empty());
    }
    assert(BattlefieldTerrain(YellowTurbanMission,8,8)==Terrain::Forest);
    assert(EncodeJourneySave(*old,after) && DecodeJourneySave(after,*old));
    assert(old->journey.Withdraw() && !old->journey.CanContinue() && BuildCoalitionEpisode(old->journey).lines.empty());
    assert(old->journey.RetryAtCouncil(*old->beforeBattle));
    assert(old->journey.world.cities[0].food==camp.food && !old->journey.history[YellowTurbanMission].completed);
    assert(career->Continue());
    assert(career->Prepare(1,career->enemyPolicy));
    for(int cycle=0;cycle<18 && career->stage==JourneyStage::Battle;++cycle)
    {
        auto next=PlanJourneyAutoBattle(*career);
        if(!next.tactics.empty()) career->Execute(next.tactics);
        if(career->stage==JourneyStage::Battle) assert(next.movement.Resolve(*career));
    }
    assert(career->won && career->history[YellowTurbanMission].review.resolution==JourneyResolution::Captured);
    assert(BuildCoalitionEpisode(*career).lines.empty() && career->Continue());
    const auto yellowEnding=BuildJourneyEpisode(*career);
    assert(yellowEnding.lines.front().text.find(U"皇甫嵩")!=std::u32string::npos);
    session->beforeBattle.reset();session->journey=*career;
    assert(EncodeJourneySave(*session,after) && DecodeJourneySave(after,*old));
    assert(old->journey.history[YellowTurbanMission].won && BuildCoalitionEpisode(old->journey).lines.size()==14);
    const auto nextChapter=BuildCoalitionEpisode(*career);
    assert(nextChapter.title.find(U"第二章　反董卓連合")!=std::u32string::npos && nextChapter.lines.size()==14);
    assert(nextChapter.lines.front().text.find(U"何進")!=std::u32string::npos);
    assert(nextChapter.lines[7].text.find(U"袁紹")!=std::u32string::npos && nextChapter.lines.back().speaker==3);
    for(int speaker=CaoCao;speaker<=LuBu;++speaker)
        assert(std::any_of(nextChapter.lines.begin(),nextChapter.lines.end(),
            [speaker](const auto& line) { return line.speaker==speaker; }));
    auto earlierResult=std::make_unique<StrategistJourney>(*career);
    earlierResult->history[0].rescued=true;
    assert(BuildCoalitionEpisode(*earlierResult).lines[6].text!=nextChapter.lines[6].text);
    earlierResult->campaignFlow=false;
    assert(BuildCoalitionEpisode(*earlierResult).lines.empty());
    previous.clear();
    JourneyArchive previousCoalitionWriter{false,previous};previousCoalitionWriter.version=13;
    version=13;checksum=0;
    previousCoalitionWriter(magic,version,checksum);previousCoalitionWriter(*session);
    checksum=2166136261u;
    for(std::size_t i=12;i<previous.size();++i) { checksum^=previous[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) previous[8+i]=static_cast<std::uint8_t>(checksum>>(i*8));
    for(auto plan:{CoalitionPlan::Advance,CoalitionPlan::Protect})
    {
        assert(DecodeJourneySave(previous,*old));
        auto& battle=old->journey;
        const auto campBefore=battle.world.cities[0];
        const auto officersBefore=battle.officerTrust;
        const bool protect=plan==CoalitionPlan::Protect;
        assert(!battle.ChooseCoalitionPlan(CoalitionPlan::None) && battle.ChooseCoalitionPlan(plan));
        assert(!battle.CanChooseCoalitionPlan() && battle.mission==CoalitionMission && battle.stage==JourneyStage::Preparation);
        assert(battle.world.cities[0].gold==campBefore.gold && battle.world.cities[0].food==campBefore.food &&
            battle.world.cities[0].troops==campBefore.troops && battle.officerTrust==officersBefore);
        assert(battle.Definition().kind==(protect ? JourneyObjective::HoldPositions:JourneyObjective::CaptureCamp));
        assert(battle.ApplyPlanExample());
        old->beforeBattle=battle;
        assert(battle.Prepare(1,battle.enemyPolicy));
        assert(!battle.HasConvoy() && battle.world.armies[0].tile==Campaign::At(protect ? 8:9,6));
        assert(EncodeJourneySave(*old,before) && DecodeJourneySave(before,*old) && battle.coalitionPlan==plan);
        assert(battle.Withdraw() && !battle.CanContinue() && battle.RetryAtCouncil(*old->beforeBattle));
        assert(battle.coalitionPlan==plan && battle.world.cities[0].food==campBefore.food && battle.Prepare(1,battle.enemyPolicy));
        for(int cycle=0;cycle<18 && battle.stage==JourneyStage::Battle;++cycle)
        {
            auto next=PlanJourneyAutoBattle(battle);
            if(!next.tactics.empty()) battle.Execute(next.tactics);
            if(battle.stage==JourneyStage::Battle) assert(next.movement.Resolve(battle));
        }
        std::cerr<<"coalition plan="<<int(plan)<<" won="<<battle.won<<" turn="<<battle.turn<<" held="<<battle.heldTurns<<" survivors="<<battle.survivors<<"\n";
        assert(battle.won && battle.history[CoalitionMission].review.resolution==(protect ? JourneyResolution::Held:JourneyResolution::Captured));
        assert(battle.Continue() && battle.stage==JourneyStage::Complete);
        assert(EncodeJourneySave(*old,before) && DecodeJourneySave(before,*old) && battle.coalitionPlan==plan);
        assert(BuildJourneyEpisode(battle).lines[1].text.find(protect ? U"張邈":U"曹操")!=std::u32string::npos);
        if(protect)
        {
            previous.clear();
            JourneyArchive version15{false,previous};version15.version=15;
            version=15;checksum=0;version15(magic,version,checksum);version15(*old);
            checksum=2166136261u;
            for(std::size_t i=12;i<previous.size();++i) { checksum^=previous[i];checksum*=16777619u; }
            for(int i=0;i<4;++i) previous[8+i]=static_cast<std::uint8_t>(checksum>>(i*8));
            assert(DecodeJourneySave(previous,*old) && battle.CanBeginYangren());
        }
        const auto afterSuanzao=battle.world.cities[0];
        assert(battle.CanBeginYangren() && battle.Continue() && battle.mission==YangrenMission);
        assert(battle.Battlefield()==JourneyMap::Yangren && battle.world.cities[0].troops==afterSuanzao.troops &&
            battle.world.cities[0].food==afterSuanzao.food && battle.world.cities[0].gold==afterSuanzao.gold);
        assert(BuildJourneyEpisode(battle).lines[1].speaker==(protect ? ZhangMiao:CaoCao) && battle.ApplyPlanExample());
        old->beforeBattle=battle;
        assert(battle.Prepare(1,battle.enemyPolicy));
        assert(!battle.HasConvoy() && battle.world.armies[3].arm==Arm::Cavalry &&
            battle.world.generals[3].name==EpisodeSpeakerName(LuBu) &&
            battle.world.generals[5].name==EpisodeSpeakerName(SunJian));
        assert(battle.world.armies[5].tile==SunJianStart(protect) &&
            battle.world.armies[1].tile==Campaign::At(protect ? 7:8,protect ? 7:6));
        assert(EncodeJourneySave(*old,before) && DecodeJourneySave(before,*old));
        {
            auto probe=std::make_unique<StrategistJourney>(battle);
            probe->world.armies[0].tile=probe->goal;
            probe->world.armies[3].troops=probe->world.armies[4].troops=0;
            assert(probe->PositionSecured(probe->goal) && !probe->SunJianAtRally() &&
                probe->EndTurn() && probe->stage==JourneyStage::Battle);
            probe->world.armies[5].tile=YangrenRally;
            assert(probe->EndTurn() && probe->won);
            assert(probe->survivors==probe->world.armies[0].troops+probe->world.armies[1].troops+probe->world.armies[2].troops);
        }
        {
            auto cavalry=std::make_unique<StrategistJourney>(battle);
            cavalry->world.armies[0].tile=Campaign::At(12,8);
            const int contact=cavalry->world.armies[3].tile;
            assert(cavalry->world.MapDistance(cavalry->world.armies[0].tile,contact)==1);
            auto forest=std::make_unique<StrategistJourney>(*cavalry);
            forest->world.tiles[contact].terrain=Terrain::Forest;
            auto disrupted=std::make_unique<StrategistJourney>(*cavalry);
            disrupted->chain.disordered[3]=true;
            assert(cavalry->EndTurn() && !cavalry->world.tacticEvents.empty() &&
                cavalry->world.tacticEvents.front().tactic==Tactic::MountedCharge);
            assert(forest->EndTurn() && forest->world.tacticEvents.empty());
            assert(disrupted->EndTurn() && disrupted->world.tacticEvents.empty());
        }
        if(protect)
        {
            // Exercise a real halt, then clear the same enemy with the two
            // ordinary-attack entry points. The march, not the kill, confirms it.
            auto stalled=std::make_unique<JourneySaveData>(*old);
            auto& j=stalled->journey;
            auto& a=j.world.armies;
            a[0].tile=a[0].target=j.goal;
            a[2].tile=a[2].target=Campaign::At(9,9);
            a[3].troops=0;
            a[4].tile=a[4].target=Campaign::At(9,10);
            const int start=a[5].tile;
            assert(j.world.MapDistance(a[4].tile,start)==1 && j.world.CanStrike(2,a[4].tile));
            assert(j.EndTurn() && j.battleReview.yangren.blockers==2 && a[5].tile==start);
            a[4].troops=50;
            j.RefreshIntelligence();
            const auto writeLegacy=[&](JourneySaveData& source,unsigned version)
            {
                std::vector<std::uint8_t> bytes;
                JourneyArchive archive{false,bytes};archive.version=version;
                unsigned checksum=0;
                archive(magic,version,checksum);archive(source);
                checksum=2166136261u;
                for(std::size_t i=12;i<bytes.size();++i) { checksum^=bytes[i];checksum*=16777619u; }
                for(int i=0;i<4;++i) bytes[8+i]=static_cast<std::uint8_t>(checksum>>(i*8));
                return bytes;
            };
            auto legacyActive=std::make_unique<JourneySaveData>();
            assert(DecodeJourneySave(writeLegacy(*stalled,16),*legacyActive));
            assert(legacyActive->journey.battleReview.yangren.tracked && !legacyActive->journey.battleReview.yangren.wasBlocked);
            auto queued=std::make_unique<StrategistJourney>(j);
            JourneyRealtime orders;
            assert(orders.Queue(*queued,2,JourneyDirective::Attack,4) && orders.Resolve(*queued));
            assert(queued->battleReview.yangren.roadOfficer==2 && queued->world.armies[5].tile!=start);
            auto pulled=std::make_unique<StrategistJourney>(j);
            auto& pulledUnits=pulled->world.armies;
            pulledUnits[0].tile=pulledUnits[0].target=Campaign::At(10,10);
            pulledUnits[4].stance=battle::Stance::Balanced;
            pulled->RefreshIntelligence();
            const std::vector<ChainOrder> lureRoad{{0,4,ChainTactic::Lure,Campaign::At(11,10)}};
            assert(pulled->Preview(lureRoad).completed==1 && pulled->battleReview.yangren.pendingOfficer==-1);
            assert(pulled->Execute(lureRoad).completed==1 && pulled->battleReview.yangren.pendingOfficer==0 &&
                pulled->battleReview.yangren.roadOfficer==-1 && pulledUnits[4].troops>0);
            assert(pulled->EndTurn() && pulled->battleReview.yangren.roadOfficer==0 && pulledUnits[5].tile!=start);
            auto drift=std::make_unique<StrategistJourney>(j);
            drift->world.armies[4].troops=0;
            assert(drift->EndTurn() && drift->world.armies[5].tile!=start);
            assert(drift->battleReview.yangren.roadOfficer==-1);

            assert(j.Strike(2,4) && a[4].troops==0 && j.battleReview.yangren.pendingOfficer==2 &&
                j.battleReview.yangren.roadOfficer==-1 && a[5].tile==start);
            auto failed=std::make_unique<StrategistJourney>(j);
            assert(failed->Withdraw() && failed->battleReview.yangren.TrustGain(failed->won)==0 &&
                failed->battleReview.yangren.roadOfficer==-1 &&
                failed->RetryAtCouncil(*stalled->beforeBattle) && !failed->battleReview.yangren.tracked &&
                failed->Prepare(1,failed->enemyPolicy) && failed->battleReview.yangren.tracked &&
                !failed->battleReview.yangren.wasBlocked);
            std::vector<std::uint8_t> bytes;
            assert(EncodeJourneySave(*stalled,bytes) && DecodeJourneySave(bytes,*stalled));
            assert(j.battleReview.yangren.pendingOfficer==2);
            for(int i=0;i<8 && j.stage==JourneyStage::Battle;++i) assert(j.EndTurn());
            const auto result=j.history[YangrenMission].review.yangren;
            assert(j.won && result.roadOfficer==2 && result.TrustGain(j.won)==5);
            assert(BuildJourneyEpisode(j).lines.front().text.find(U"張飛")!=std::u32string::npos);
            assert(EncodeJourneySave(*stalled,bytes) && DecodeJourneySave(bytes,*stalled) &&
                j.history[YangrenMission].review.yangren==result);
            assert(j.Continue());
            auto sameVersion=std::make_unique<JourneySaveData>();
            assert(DecodeJourneySave(writeLegacy(*stalled,17),*sameVersion) &&
                sameVersion->journey.CanChooseLuoyangPlan(LuoyangPlan::WesternDepot));
            assert(DecodeJourneySave(writeLegacy(*stalled,16),*stalled) && j.won && !j.history[YangrenMission].review.yangren.tracked);
            assert(BuildJourneyEpisode(j).lines.front().text.find(U"張飛")==std::u32string::npos);
            assert(j.CanChooseLuoyangPlan(LuoyangPlan::SouthGate) &&
                !j.CanChooseLuoyangPlan(LuoyangPlan::WesternDepot));
        }
        {
            auto lure=std::make_unique<StrategistJourney>(battle);
            auto& w=lure->world;
            w.armies[0].tile=w.armies[0].target=Campaign::At(12,8);
            const int plain=w.armies[3].tile, forest=w.armies[0].tile;
            w.tiles[forest].terrain=Terrain::Forest;
            int retreat=-1;
            for(int tile:w.MapNeighbors(forest))
                if(w.MapDistance(plain,tile)>1 && TacticChain::Free(w,tile)) { retreat=tile;break; }
            assert(retreat>=0 && w.tiles[plain].terrain==Terrain::Plain);
            auto disrupted=std::make_unique<StrategistJourney>(*lure);
            std::swap(disrupted->world.armies[0].tile,disrupted->world.armies[1].tile);
            disrupted->RefreshIntelligence();
            assert(disrupted->Execute({{1,3,ChainTactic::AzureDragon,-1}}).completed==1);
            assert(disrupted->battleReview.yangren.counter==YangrenCounter::Disrupted &&
                disrupted->battleReview.yangren.cavalryOfficer==1 && disrupted->chain.disordered[3]);
            lure->RefreshIntelligence();
            const std::vector<ChainOrder> tactics{{0,3,ChainTactic::Lure,retreat}};
            assert(lure->Preview(tactics).completed==1 && lure->battleReview.yangren.counter==YangrenCounter::None);
            assert(lure->Execute(tactics).completed==1 && w.armies[3].tile==forest);
            assert(lure->battleReview.yangren.counter==YangrenCounter::ForestLure &&
                lure->battleReview.yangren.cavalryOfficer==0 && lure->EndTurn() && w.tacticEvents.empty());
            for(int cycle=0;cycle<18 && lure->stage==JourneyStage::Battle;++cycle)
            {
                auto next=PlanJourneyAutoBattle(*lure);
                if(!next.tactics.empty()) lure->Execute(next.tactics);
                if(lure->stage==JourneyStage::Battle) assert(next.movement.Resolve(*lure));
            }
            assert(lure->won);
            const auto& review=lure->history[YangrenMission].review.yangren;
            assert(review.counter==YangrenCounter::ForestLure && review.cavalryOfficer==0 &&
                review.TrustGain(lure->won)==(review.roadOfficer>=0 ? 7:4));
            const auto epilogue=BuildJourneyEpisode(*lure);
            assert(std::any_of(epilogue.lines.begin(),epilogue.lines.end(),[](const auto& line)
            {
                return line.speaker==SunJian && line.text.find(U"劉備が呂布を林に")!=std::u32string::npos;
            }));
        }
        for(int cycle=0;cycle<18 && battle.stage==JourneyStage::Battle;++cycle)
        {
            auto next=PlanJourneyAutoBattle(battle);
            if(!next.tactics.empty()) battle.Execute(next.tactics);
            if(battle.stage==JourneyStage::Battle) assert(next.movement.Resolve(battle));
        }
        std::cerr<<"yangren plan="<<int(plan)<<" won="<<battle.won<<" turn="<<battle.turn<<" allied="<<battle.world.armies[5].troops<<"\n";
        assert(battle.won && battle.SunJianAtRally() && battle.Continue() && battle.stage==JourneyStage::Complete &&
            EncodeJourneySave(*old,before) && DecodeJourneySave(before,*old) && !battle.CanContinue());
        for(const auto route:{LuoyangPlan::SouthGate,LuoyangPlan::WesternDepot})
        {
            auto next=std::make_unique<JourneySaveData>(*old);
            auto& j=next->journey;
            // The completed road-relief fixture above earns +5. Use that evidence
            // for the branch contract independently of what the auto battle did.
            auto& review=j.history[YangrenMission].review.yangren;
            review={};
            review.tracked=true;
            if(route==LuoyangPlan::WesternDepot) { review.wasBlocked=true;review.roadOfficer=2; }
            j.battleReview.yangren=review;
            const bool west=route==LuoyangPlan::WesternDepot;
            if(!west) assert(!j.ChooseLuoyangPlan(LuoyangPlan::WesternDepot));
            const auto camp=j.world.cities[0];
            const auto relationships=j.officerTrust;
            const auto council=BuildJourneyEpisode(j);
            assert(council.lines[council.lines.size()-2].speaker==SunJian);
            assert((council.lines[council.lines.size()-2].text.find(U"荷駄")!=std::u32string::npos)==west);
            assert(j.ChooseLuoyangPlan(route) && !j.ChooseLuoyangPlan(route));
            assert(j.stage==JourneyStage::Preparation && j.mission==LuoyangMission &&
                j.Battlefield()==(west ? JourneyMap::LuoyangDepot:JourneyMap::LuoyangGate));
            assert(j.world.cities[0].food==camp.food && j.world.cities[0].gold==camp.gold &&
                j.world.cities[0].troops==camp.troops && j.officerTrust==relationships);
            assert(j.SelectDifficulty(protect ? JourneyDifficulty::Standard:JourneyDifficulty::Story));
            assert(j.ApplyPlanExample());
            next->beforeBattle=j;
            assert(j.Prepare(1,j.enemyPolicy));
            std::vector<std::uint8_t> bytes;
            assert(EncodeJourneySave(*next,bytes) && DecodeJourneySave(bytes,*next));
            auto retry=std::make_unique<StrategistJourney>(j);
            assert(retry->Withdraw() && retry->RetryAtCouncil(*next->beforeBattle) && retry->luoyangPlan==route &&
                retry->world.cities[0].food==camp.food && retry->SunJianTrust()==j.SunJianTrust());
            for(int i=0;i<18 && j.stage==JourneyStage::Battle;++i)
            {
                auto orders=PlanJourneyAutoBattle(j);
                if(!orders.tactics.empty()) j.Execute(orders.tactics);
                if(j.stage==JourneyStage::Battle) assert(orders.movement.Resolve(j));
            }
            std::cerr<<"luoyang plan="<<int(route)<<" won="<<j.won<<" turn="<<j.turn<<" survivors="<<j.survivors<<"\n";
            assert(j.won && BuildJourneyEpisode(j).lines.front().text.find(west ? U"荷駄":U"南門")!=std::u32string::npos);
            // Isolate the payout from ordinary food consumption with an immediate
            // public Evaluate, triggered by marching the last step onto the goal.
            auto payout=std::make_unique<StrategistJourney>(*next->beforeBattle);
            assert(payout->Prepare(1,payout->enemyPolicy));
            auto& w=payout->world;
            w.armies[3].troops=w.armies[4].troops=0;
            const int marchFrom=Campaign::At(west ? 9:10,west ? 7:5);
            w.armies[0].tile=w.armies[0].target=marchFrom;
            payout->RefreshIntelligence();
            int expectedFood=w.cities[0].food+payout->reserveFood+(west ? WesternDepotFood:0);
            for(int army=0;army<3;++army) expectedFood+=w.armies[army].food;
            assert(payout->Move(0,payout->goal) && payout->won && w.cities[0].food==expectedFood);
            assert(j.Continue() && EncodeJourneySave(*next,bytes) && DecodeJourneySave(bytes,*next) &&
                j.luoyangPlan==route && j.history[LuoyangMission].won && !j.CanContinue());
        }
        battle.ResetCampaign();
        assert(battle.coalitionPlan==CoalitionPlan::None && !battle.history[CoalitionMission].completed && EncodeJourneySave(*old,before));
    }
    hold->stage=JourneyStage::Debrief;const auto ended=PlanJourneyAutoBattle(*hold);assert(ended.tactics.empty());
    for(const auto& order:ended.movement.orders) assert(order.kind==JourneyDirective::None);
}
