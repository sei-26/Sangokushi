#include "../NewGame/JourneySave.hpp"
#include "../NewGame/JourneyEpisode.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <iostream>
using namespace frontline;
static std::unique_ptr<StrategistJourney> Win(int loss=0,bool linked=false)
{
    auto j=std::make_unique<StrategistJourney>();j->Reset();j->Meet();assert(j->Prepare(1,1));
    j->world.armies[0].troops-=loss;
    if(linked)
    {
        const int cells[]={Campaign::At(9,9),Campaign::At(10,8),Campaign::At(8,9),Campaign::At(9,8)};
        for(int i=0;i<4;++i) j->world.armies[i].tile=j->world.armies[i].target=cells[i];
        j->world.armies[3].troops=600;j->RefreshIntelligence();
        assert(j->Execute({{0,3,ChainTactic::Feint,-1},{1,3,ChainTactic::Feint,-1},{2,3,ChainTactic::Feint,-1}}).completed==3);
    }
    else
    {
        j->world.armies[3].troops=460;j->RefreshIntelligence();assert(j->Strike(0,3));
    }
    assert(j->won && j->stage==JourneyStage::Debrief);return j;
}
static void DeliverWithoutFighting(StrategistJourney& j)
{
    j.world.armies[3].troops=j.world.armies[4].troops=0;j.RefreshIntelligence();
    for(int i=0;i<6 && j.stage==JourneyStage::Battle;++i) assert(j.EndTurn());
    assert(j.won);
}
int main()
{
    auto council=std::make_unique<StrategistJourney>();council->Reset();council->Meet();council->SelectPromise(JourneyPromise::EscortRefugees);
    const auto firstEpisode=BuildJourneyEpisode(*council);assert(firstEpisode.lines.size()>=10);
    assert(firstEpisode.title.find(U"泥の道")!=std::u32string::npos);
    std::u32string firstText;for(const auto& l:firstEpisode.lines) firstText+=l.text;
    assert(firstText.find(U"天下より、今晩")!=std::u32string::npos);
    assert(firstText.find(U"車ごと送り届ける")!=std::u32string::npos);
    council->SelectPromise(JourneyPromise::OpenRoad);
    const auto roadEpisode=BuildJourneyEpisode(*council);
    assert(roadEpisode.lines.back().text.find(U"終わったとは言わない")!=std::u32string::npos);
    auto careful=Win(),costly=Win(1200),linked=Win(0,true);
    const auto quiet=BuildJourneyEpisode(*careful),heavy=BuildJourneyEpisode(*costly);
    std::u32string quietText,heavyText;
    for(const auto& l:quiet.lines) quietText+=l.text;
    for(const auto& l:heavy.lines) heavyText+=l.text;
    assert(quietText.find(U"道が空いた")!=std::u32string::npos && quietText.find(U"最後の所で")==std::u32string::npos);
    assert(heavyText.find(U"余るはず")!=std::u32string::npos && quietText!=heavyText);
    auto legacyCostly=std::make_unique<StrategistJourney>(*costly);legacyCostly->history[0].story={};
    const auto legacyEpisode=BuildJourneyEpisode(*legacyCostly);
    assert(legacyEpisode.lines[1].text==heavy.lines[1].text); // Old saves still know their real casualties.
    // A chapter is derived from public/settled facts, not from hidden enemy positions or plans.
    auto hidden=std::make_unique<StrategistJourney>(*careful);
    hidden->world.armies[4].tile=Campaign::At(12,3);hidden->enemyPolicy=2;
    const auto hiddenEpisode=BuildJourneyEpisode(*hidden);assert(hiddenEpisode.lines.size()==quiet.lines.size());
    for(size_t i=0;i<quiet.lines.size();++i) assert(hiddenEpisode.lines[i].text==quiet.lines[i].text);

    assert(careful->history[0].story.lowLoss && costly->history[0].story.costly);
    assert(costly->officerTrust[1]<0 && careful->officerTrust[1]>costly->officerTrust[1]);
    assert(linked->history[0].story.coordinatedOfficers==7 && linked->officerTrust[2]>careful->officerTrust[2]);
    const auto goodScene=JourneyConversation(*careful,1),costlyScene=JourneyConversation(*costly,1);
    assert(goodScene.size()>=4 && goodScene[0].text!=costlyScene[0].text);
    for(auto* j:{careful.get(),costly.get(),linked.get()})
    {
        for(int who=0;who<3;++who)
        {
            const auto scene=JourneyConversation(*j,who);bool hero=false,officer=false;
            for(const auto& line:scene) { hero|=line.speaker==3;officer|=line.speaker==who; }
            assert(hero && officer);
        }
        const auto trust=j->officerTrust;
        assert(j->NeedsStoryReply() && !j->Continue());
        assert(j->ChooseStoryReply(JourneyPledge::Preserve) && j->officerTrust==trust);
        assert(!j->ChooseStoryReply(JourneyPledge::Coordinate));
        assert(!j->EndTurn() && j->officerTrust==trust);
    }
    // A player's words affect the next real mission, rather than granting points on click.
    assert(careful->Continue() && careful->CurrentPledge()==JourneyPledge::Preserve);
    const auto nextEpisode=BuildJourneyEpisode(*careful);
    std::u32string nextText;for(const auto& l:nextEpisode.lines) nextText+=l.text;
    assert(nextText.find(U"九割")!=std::u32string::npos && nextEpisode.title!=firstEpisode.title);
    const auto prior=careful->officerTrust;assert(careful->Prepare(0,1));
    assert(BuildJourneyEpisode(*careful).lines.empty());
    for(int i=0;i<3;++i) assert(careful->world.armies[i].morale==90+careful->StoryMoraleBonus(i));
    DeliverWithoutFighting(*careful);
    assert(careful->history[1].story.pledgeKept && careful->officerTrust[1]>prior[1]);
    auto broken=Win();assert(broken->ChooseStoryReply(JourneyPledge::Coordinate) && broken->Continue());
    assert(broken->CurrentPledge()==JourneyPledge::Coordinate && broken->Prepare(0,1));
    DeliverWithoutFighting(*broken);
    assert(broken->won && !broken->history[1].story.pledgeKept && broken->history[1].story.trustDelta[2]<0);
    assert(JourneyConversation(*broken,2)[0].text!=JourneyConversation(*careful,2)[0].text);
    // Breaking the preservation vow must discuss the lost soldiers, not an unpromised chain.
    auto lost=Win();assert(lost->ChooseStoryReply(JourneyPledge::Preserve) && lost->Continue() && lost->Prepare(0,1));
    lost->world.armies[0].troops-=600;DeliverWithoutFighting(*lost);
    assert(!lost->history[1].story.pledgeKept);
    const auto lostScene=JourneyConversation(*lost,2);
    assert(lostScene[0].text.find(U"連れて帰る")!=std::u32string::npos);
    assert(lostScene[0].text!=JourneyConversation(*broken,2)[0].text);
    // A co-operative promise is settled from actual successful, damaging actions only.
    auto coordinated=Win();assert(coordinated->ChooseStoryReply(JourneyPledge::Coordinate) && coordinated->Continue() && coordinated->Prepare(1,1));
    const int cells[]={Campaign::At(9,9),Campaign::At(10,8),Campaign::At(8,9),Campaign::At(9,8)};
    for(int i=0;i<4;++i) coordinated->world.armies[i].tile=coordinated->world.armies[i].target=cells[i];
    coordinated->RefreshIntelligence();const auto observed=coordinated->battleReview.coordinatedOfficers;
    const std::vector<ChainOrder> orders{{0,3,ChainTactic::Feint,-1},{1,3,ChainTactic::Disrupt,-1}};
    assert(coordinated->Preview(orders).completed==2 && coordinated->battleReview.coordinatedOfficers==observed);
    assert(coordinated->Execute(orders).completed==2 && coordinated->battleReview.coordinatedOfficers==3);
    // Get the main-force screen off the convoy road, then let it reach the exit.
    coordinated->world.armies[0].tile=Campaign::At(7,7);coordinated->world.armies[1].tile=Campaign::At(7,8);
    DeliverWithoutFighting(*coordinated);assert(coordinated->history[1].story.pledgeKept);
    // Both old schemas restore without fabricated trust or promises.
    for(const auto name:{"journey-v1-chain.bin","journey-v2-combo.bin"})
    {
        std::ifstream f(std::filesystem::path(__FILE__).parent_path()/"fixtures"/name,std::ios::binary);assert(f);
        std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(f)),{});
        auto saved=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(bytes,*saved));
        assert((saved->journey.officerTrust==std::array<int,3>{}) && saved->journey.CurrentPledge()==JourneyPledge::None);
        assert(!saved->journey.history[0].story.recorded && saved->journey.battleReview.coordinatedOfficers==0);
    }
    // Pending and committed responses, and trust, survive actual save/load without re-awarding.
    auto save=std::make_unique<JourneySaveData>();save->journey.Reset();save->journey.Meet();save->beforeBattle=save->journey;
    auto fresh=Win();save->journey=*fresh;
    std::vector<std::uint8_t> bytes;assert(EncodeJourneySave(*save,bytes) && bytes[4]==9);
    auto restored=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(bytes,*restored) && restored->journey.NeedsStoryReply());
    const auto trust=restored->journey.officerTrust;assert(restored->journey.ChooseStoryReply(JourneyPledge::Coordinate));
    assert(EncodeJourneySave(*restored,bytes) && DecodeJourneySave(bytes,*save));
    assert(save->journey.officerTrust==trust && !save->journey.NeedsStoryReply() && !save->journey.ChooseStoryReply(JourneyPledge::Preserve));
    std::vector<std::uint8_t> readBefore,readAfter;assert(EncodeJourneySave(*save,readBefore));
    for(int i=0;i<3;++i) assert(!BuildJourneyEpisode(save->journey).lines.empty());
    assert(EncodeJourneySave(*save,readAfter) && readBefore==readAfter);
    save->journey.officerTrust[0]=101;assert(!EncodeJourneySave(*save,bytes));
    std::cout<<"Battle story PASS: distinct victories, officer-specific trust, protagonist dialogue, real vows, morale carryover, immutable previews, v1/v2/v3 saves, one-shot reply\n";
}
