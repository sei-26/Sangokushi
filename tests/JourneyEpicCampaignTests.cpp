#include "JourneyAutoBattle.hpp"
#include "JourneySave.hpp"
#include "JourneyArchive.hpp"
#include "JourneyEpisode.hpp"
#include "JourneyBattlefield.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;

static void RoundTrip(const JourneySaveData& session)
{
    std::vector<std::uint8_t> bytes;auto loaded=std::make_unique<JourneySaveData>();
    assert(EncodeJourneySave(session,bytes) && DecodeJourneySave(bytes,*loaded));
    assert(loaded->journey.mission==session.journey.mission && loaded->journey.shortPrelude==session.journey.shortPrelude);
    assert(loaded->journey.battleReview.grand.yangren.counterOn==session.journey.battleReview.grand.yangren.counterOn);
}
static void Play(JourneySaveData& session)
{
    auto& j=session.journey;
    for(int cycle=0;cycle<36 && j.stage==JourneyStage::Battle;++cycle)
    {
        auto plan=PlanJourneyAutoBattle(j);
        if(!plan.tactics.empty()) j.Execute(plan.tactics);
        if(j.stage==JourneyStage::Battle) assert(plan.movement.Resolve(j));
    }
}
static std::unique_ptr<JourneySaveData> Yangren(bool modern=true,JourneyDifficulty difficulty=JourneyDifficulty::Story)
{
    auto session=std::make_unique<JourneySaveData>();auto& j=session->journey;
    j.ResetCampaign();j.Meet();j.mission=YangrenMission;j.grandCampaign=true;j.grandStrategyCampaign=true;
    j.yangrenStrategyCampaign=modern;j.coalitionPlan=CoalitionPlan::Advance;
    for(int m=0;m<YangrenMission;++m) j.history[m].completed=j.history[m].won=j.history[m].campaignFlow=true;
    j.SelectDifficulty(difficulty);session->beforeBattle=j;assert(j.Prepare(1,0));return session;
}
int main()
{
    auto first=std::make_unique<JourneySaveData>();auto& j=first->journey;
    j.ResetCampaign();j.Meet();j.shortPrelude=j.grandCampaign=j.grandStrategyCampaign=j.yangrenStrategyCampaign=true;
    j.SelectDifficulty(JourneyDifficulty::Story);first->beforeBattle=j;assert(j.Prepare(1,0));Play(*first);assert(j.won);
    assert(j.ChooseStoryReply(JourneyPledge::Coordinate));
    auto extended=std::make_unique<JourneySaveData>(*first);
    assert(extended->journey.ContinuePrelude() && extended->journey.mission==1 && !extended->journey.shortPrelude);
    const int food=j.world.cities[0].food,troops=j.world.cities[0].troops,trust=j.officerTrust[0];
    assert(j.Continue() && j.mission==YellowTurbanMission && j.stage==JourneyStage::Preparation);
    assert(j.world.cities[0].food==food && j.world.cities[0].troops==troops && j.officerTrust[0]==trust);
    for(int m:{1,2}) assert(j.history[m].completed && !j.history[m].won && !j.history[m].story.recorded &&
        j.history[m].review.resolution==JourneyResolution::JoinedMainForce);
    assert(j.CurrentPledge()==JourneyPledge::Coordinate && !j.CanJoinMainForce());
    assert(j.CampaignRoad().find(U"【黄巾の乱】")!=std::u32string::npos && BuildJourneyEpisode(j).lines.size()>=8);
    RoundTrip(*first);
    auto malformed=std::make_unique<JourneySaveData>(*first);malformed->journey.history[1].won=true;
    assert(!ValidJourneySave(*malformed));
    auto setback=std::make_unique<JourneySaveData>();auto& failed=setback->journey;
    failed.ResetCampaign();failed.Meet();failed.grandCampaign=failed.shortPrelude=true;
    setback->beforeBattle=failed;assert(failed.Prepare(1,0) && failed.Withdraw());
    assert(failed.CanContinue() && failed.Continue() && failed.mission==YellowTurbanMission);
    assert(!failed.history[0].won && failed.history[0].withdrawn &&
        failed.history[1].review.resolution==JourneyResolution::JoinedMainForce && !failed.CanJoinMainForce());
    RoundTrip(*setback);
    for(auto choice:{YangrenApproach::Forest,YangrenApproach::Advance})
        for(auto difficulty:{JourneyDifficulty::Story,JourneyDifficulty::Standard})
        {
            auto session=Yangren(true,difficulty);auto& battle=session->journey;
            assert(battle.HasYangrenStrategy() && battle.Battlefield()==JourneyMap::YangrenCounter);
            for(auto row:BattlefieldDefinition(battle.Battlefield()).rows) assert(row.size()==14);
            assert(battle.world.armies[8].arm==Arm::Cavalry && battle.ChooseYangrenApproach(choice));
            assert(!battle.ChooseYangrenApproach(YangrenApproach::Advance));
            const int start=battle.world.armies[5].tile,reserve=battle.world.armies[8].tile;
            assert(battle.EndTurn() && battle.world.armies[8].tile==reserve);
            if(choice==YangrenApproach::Forest) assert(battle.world.armies[5].tile==start);
            else assert(battle.world.armies[5].tile!=start);
            RoundTrip(*session);Play(*session);
            const auto& y=battle.battleReview.grand.yangren;
            std::cerr<<"yangren approach="<<int(choice)<<" difficulty="<<int(difficulty)<<" won="<<battle.won<<" turn="<<battle.turn<<" counter="<<y.counterOn<<" repelled="<<y.repelledOn<<" main="<<battle.world.armies[5].troops<<" HQ="<<battle.world.armies[10].troops<<"\n";
            assert(battle.won && y.counterOn && y.repelledOn && battle.world.armies[10].troops>0);
            RoundTrip(*session);assert(battle.Continue() && battle.CanChooseLuoyangPlan(LuoyangPlan::SouthGate));
            assert(BuildJourneyEpisode(battle).lines.size()>=5);RoundTrip(*session);
        }
    int battles=1;
    for(int transition=0;transition<25;++transition)
    {
        if(j.CanChooseCoalitionPlan()) assert(j.ChooseCoalitionPlan(CoalitionPlan::Advance));
        else if(j.CanChooseLuoyangPlan(LuoyangPlan::WesternDepot)) assert(j.ChooseLuoyangPlan(LuoyangPlan::WesternDepot));
        else if(j.CanChooseLuoyangPlan(LuoyangPlan::SouthGate)) assert(j.ChooseLuoyangPlan(LuoyangPlan::SouthGate));
        else if(j.stage==JourneyStage::Preparation)
        {
            if(j.world.cities[0].troops<4500) j.Reinforce();
            first->beforeBattle=j;assert(j.Prepare(1,j.enemyPolicy));
            if(j.CanChooseGrandApproach()) j.ChooseGrandApproach(GrandApproach::RelieveLeft);
            if(j.CanChooseYangrenApproach()) j.ChooseYangrenApproach(YangrenApproach::Forest);
            Play(*first);std::cerr<<"epic campaign mission="<<j.mission<<" won="<<j.won<<"\n";
            assert(j.won);++battles;RoundTrip(*first);
        }
        else if(j.CanContinue()) assert(j.Continue());
        else break;
    }
    assert(battles==5 && j.history[LuoyangMission].completed && !j.history[1].won && j.history[5].review.grand.yangren.repelledOn);
    auto old=Yangren(false);std::vector<std::uint8_t> bytes;unsigned magic=0x53594e47,version=24,checksum=0;
    JourneyArchive archive{false,bytes};archive.version=version;archive(magic,version,checksum);archive(*old);
    checksum=2166136261u;for(std::size_t i=12;i<bytes.size();++i) { checksum^=bytes[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) bytes[8+i]=std::uint8_t(checksum>>(i*8));
    auto restored=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(bytes,*restored));
    assert(!restored->journey.HasYangrenStrategy() && !restored->journey.shortPrelude && restored->journey.Battlefield()==JourneyMap::Yangren);
    assert(restored->beforeBattle && !restored->beforeBattle->yangrenStrategyCampaign);
    restored->journey.yangrenStrategyCampaign=true;
    assert(!restored->journey.HasYangrenStrategy() && restored->journey.Battlefield()==JourneyMap::Yangren);
    RoundTrip(*restored);
    std::cout<<"Epic campaign: PASS\n";
}
