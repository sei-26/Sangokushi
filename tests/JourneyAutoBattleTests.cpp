#include "JourneyAutoBattle.hpp"
#include "JourneySave.hpp"
#include "JourneyArchive.hpp"
#include "JourneyEpisode.hpp"
#include "JourneyCampaign.hpp"
#include "JourneyBattlefield.hpp"
#include "JourneyBattleChoices.hpp"
#include "JourneyPlanExample.hpp"
#include <cassert>
#include <memory>
#include <iostream>
#include <chrono>
using namespace frontline;

static std::unique_ptr<StrategistJourney> OpenAutoField(int mission)
{
    auto journey=std::make_unique<StrategistJourney>();
    journey->ResetCampaign();journey->mission=mission;journey->Meet();
    assert(journey->Prepare(1,0));
    for(int y=0;y<StrategistJourney::Rows;++y)
        for(int x=0;x<StrategistJourney::Columns;++x)
            journey->world.tiles[Campaign::At(x,y)].terrain=Terrain::Plain;
    for(int army=3;army<int(journey->world.armies.size());++army) journey->world.armies[army].troops=0;
    return journey;
}

static void VerifyAutoTeamMovement()
{
    // A geometrically close officer is trapped by water. Choose the officer
    // with a real route to the beacon and leave the other beacon staffed.
    auto defense=OpenAutoField(2);
    auto& world=defense->world;
    world.armies[0].tile=world.armies[0].target=Campaign::At(7,4);
    world.armies[1].tile=world.armies[1].target=Campaign::At(10,6);
    world.armies[2].tile=world.armies[2].target=defense->Definition().secondGoal;
    for(int tile:world.MapNeighbors(world.armies[0].tile)) world.tiles[tile].terrain=Terrain::River;
    defense->RefreshIntelligence();
    const auto duties=PlanJourneyAutoBattle(*defense);
    assert(duties.movement.orders[1].kind==JourneyDirective::March &&
        duties.movement.orders[1].destination==defense->goal && !duties.movement.Route(*defense,1).empty());
    assert(duties.movement.orders[2].kind==JourneyDirective::Hold);

    // Two equally reachable fronts must not split a healthy assault group.
    auto assault=OpenAutoField(0);
    auto& field=assault->world;
    for(int officer=0;officer<3;++officer)
        field.armies[officer].tile=field.armies[officer].target=Campaign::At(6,6+officer);
    field.armies[3].tile=field.armies[3].target=Campaign::At(9,6);
    field.armies[4].tile=field.armies[4].target=Campaign::At(9,8);
    field.armies[3].troops=field.armies[4].troops=2000;
    assault->RefreshIntelligence();
    const auto coordinated=PlanJourneyAutoBattle(*assault);
    int focus=-1,participants=0;
    for(const auto& order:coordinated.movement.orders)
        if(order.kind==JourneyDirective::Attack)
        {
            if(focus<0) focus=order.target;
            assert(order.target==focus);++participants;
        }
    assert(participants>=2);

    // Private enemy displacement cannot alter the observed team decision.
    auto hidden=std::make_unique<StrategistJourney>(*assault);
    hidden->world.armies[3].tile=Campaign::At(14,2);
    hidden->world.armies[4].tile=Campaign::At(14,3);
    const auto same=PlanJourneyAutoBattle(*hidden);
    for(int officer=0;officer<3;++officer)
        assert(SameJourneyOrder(coordinated.movement.orders[officer],same.movement.orders[officer]));

    // A nearer but unreachable enemy must not pin the entire attack group.
    auto isolated=OpenAutoField(0);
    auto& map=isolated->world;
    const std::array<int,3> starts{Campaign::At(5,8),Campaign::At(6,9),Campaign::At(7,9)};
    for(int officer=0;officer<3;++officer)
    {
        map.armies[officer].tile=map.armies[officer].target=starts[officer];
        map.armies[officer].scouting=true;
    }
    map.armies[3].tile=map.armies[3].target=Campaign::At(5,6);
    map.armies[4].tile=map.armies[4].target=Campaign::At(9,9);
    map.armies[3].troops=map.armies[4].troops=2000;
    for(int tile:map.MapNeighbors(map.armies[3].tile)) map.tiles[tile].terrain=Terrain::River;
    isolated->RefreshIntelligence();
    assert(isolated->intelligence[0].Find(3) && isolated->intelligence[0].Find(3)->current);
    const auto reachable=PlanJourneyAutoBattle(*isolated);
    assert(reachable.movement.orders[0].kind==JourneyDirective::Attack && reachable.movement.orders[0].target==4 &&
        !reachable.movement.Route(*isolated,0).empty());
    map.armies[4].tile=map.armies[4].target=Campaign::At(14,12);
    isolated->intelligence[0].Observe(map,0,true);
    const auto distant=PlanJourneyAutoBattle(*isolated);
    assert(distant.movement.orders[0].kind==JourneyDirective::Attack && distant.movement.orders[0].target==4 &&
        !distant.movement.Route(*isolated,0).empty());

    // Two attacks originally share their next cell. The second officer can
    // approach the other flank instead of losing the entire movement pulse.
    auto flanking=std::make_unique<StrategistJourney>(*assault);
    auto& flank=flanking->world;
    flank.armies[0].tile=flank.armies[0].target=Campaign::At(7,6);
    flank.armies[1].tile=flank.armies[1].target=Campaign::At(7,5);
    flank.armies[2].troops=flank.armies[4].troops=0;
    flank.armies[3].tile=flank.armies[3].target=Campaign::At(9,7);
    flanking->chain.disordered[3]=true;
    flanking->RefreshIntelligence();
    JourneyRealtime direct;
    assert(direct.Queue(*flanking,0,JourneyDirective::Attack,3) && direct.Queue(*flanking,1,JourneyDirective::Attack,3));
    const auto originalFirst=direct.Route(*flanking,0),originalSecond=direct.Route(*flanking,1);
    assert(originalFirst.size()>1 && originalSecond.size()>1 && originalFirst[1]==originalSecond[1]);
    auto spread=PlanJourneyAutoBattle(*flanking);
    const auto first=spread.movement.Route(*flanking,0),second=spread.movement.Route(*flanking,1);
    assert(first.size()>1 && second.size()>1 && first[1]!=second[1] && spread.tactics.empty());
    assert(spread.movement.Resolve(*flanking) && flank.armies[0].tile==first[1] && flank.armies[1].tile==second[1]);
}

static void VerifyAutoEscortGroup()
{
    for(auto difficulty:{JourneyDifficulty::Story,JourneyDifficulty::Standard})
        for(int policy:{0,1,2})
        {
            auto battle=std::make_unique<StrategistJourney>();
            battle->ResetCampaign();battle->Meet();
            assert(battle->SelectDifficulty(difficulty) && battle->SelectPromise(JourneyPromise::EscortRefugees) &&
                battle->Prepare(1,policy));
            for(int cycle=0;cycle<18 && battle->stage==JourneyStage::Battle;++cycle)
            {
                auto plan=PlanJourneyAutoBattle(*battle);
                if(!plan.tactics.empty()) battle->Execute(plan.tactics);
                if(battle->stage==JourneyStage::Battle) assert(plan.movement.Resolve(*battle));
            }
            assert(battle->won && battle->history[0].rescued &&
                battle->convoyStep==int(battle->ConvoyPath().size())-1);
        }
}

static void VerifyAutoCombos()
{
    auto session=std::make_unique<StrategistJourney>();
    session->ResetCampaign();session->Meet();assert(session->Prepare(1,0));
    auto& j=*session;
    auto& w=j.world;
    for(int y=0;y<StrategistJourney::Rows;++y)
        for(int x=0;x<StrategistJourney::Columns;++x) w.tiles[Campaign::At(x,y)].terrain=Terrain::Plain;
    for(int a=3;a<int(w.armies.size());++a) w.armies[a].troops=0;
    const int target=Campaign::At(7,7);
    const auto neighbours=w.MapNeighbors(target);
    for(int a=0;a<3;++a)
    {
        w.armies[a].tile=w.armies[a].target=neighbours[a];
        w.armies[a].troops=1400;w.armies[a].morale=100;
    }
    w.armies[3].tile=w.armies[3].target=target;
    w.armies[3].troops=10000;w.armies[3].morale=100;
    w.armies[3].stance=battle::Stance::Guard;
    j.chain.Reset(w);j.RefreshIntelligence();

    // A high-damage first strike used to strand the finishers after a push.
    const auto started=std::chrono::steady_clock::now();
    const auto all=PlanJourneyAutoBattle(j);
    const auto elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
    const auto prediction=j.Preview(all.tactics);
    assert(prediction.completed==3 && prediction.events.back().comboPercent==150);
    int total=0;for(const auto& event:prediction.events) total+=event.damage;
    const auto solo=j.Preview({{1,3,ChainTactic::AzureDragon,-1}});
    assert(total>solo.events.front().damage);
    std::cerr<<"auto combo completed="<<prediction.completed<<" damage="<<total<<" planning_ms="<<elapsed<<"\n";

    // Liu Bei cannot reach the enemy, but his rally still starts a three-officer
    // chain at full morale and strengthens both adjacent finishers.
    int support=-1;
    for(int tile:w.MapNeighbors(w.armies[1].tile))
        if(w.MapDistance(tile,target)==2 && w.MapDistance(tile,w.armies[2].tile)<=2 && TacticChain::Free(w,tile))
        { support=tile;break; }
    assert(support>=0);
    w.armies[0].tile=w.armies[0].target=support;j.RefreshIntelligence();
    const auto rally=PlanJourneyAutoBattle(j);
    assert(rally.tactics.size()==3 && rally.tactics.front().tactic==ChainTactic::Oath);
    auto unseen=std::make_unique<StrategistJourney>(j);
    unseen->world.armies[3].tile=Campaign::At(14,2);
    const auto sameObservation=PlanJourneyAutoBattle(*unseen);
    assert(sameObservation.tactics.size()==rally.tactics.size());
    for(std::size_t i=0;i<rally.tactics.size();++i)
        assert(sameObservation.tactics[i].army==rally.tactics[i].army &&
            sameObservation.tactics[i].enemy==rally.tactics[i].enemy && sameObservation.tactics[i].tactic==rally.tactics[i].tactic);
    const auto expected=j.Preview(rally.tactics);
    assert(expected.completed==3 && expected.events[1].comboPercent==125 && expected.events[2].comboPercent==150);
    int withRally=0;for(const auto& event:expected.events) withRally+=event.damage;
    for(int first:{1,2})
        for(auto opening:TacticChain::Loadout(w,first))
            for(auto finishing:TacticChain::Loadout(w,3-first))
            {
                const auto alternative=j.Preview({{first,3,opening,-1},{3-first,3,finishing,-1}});
                int damage=0;for(const auto& event:alternative.events) damage+=event.damage;
                assert(withRally>damage);
            }
    auto used=std::make_unique<StrategistJourney>(j);
    used->chain.signatureUsed[0]=true;
    const auto withoutRally=PlanJourneyAutoBattle(*used);
    assert(withoutRally.tactics.size()==2);
    auto exhausted=std::make_unique<StrategistJourney>(j);
    exhausted->chain.spent[1]=true;
    for(const auto& order:PlanJourneyAutoBattle(*exhausted).tactics) assert(order.army!=1);

    // Planning leaves the real state untouched. Actual execution must produce
    // the same combo without refilling morale, signatures or action budgets.
    assert(!j.chain.spent[0] && !j.chain.signatureUsed[0] && w.armies[3].troops==10000);
    const auto actual=j.Execute(rally.tactics);
    assert(actual.completed==3 && j.chain.signatureUsed[0]);
    for(int a=0;a<3;++a) assert(j.chain.spent[a] && actual.events[a].damage==expected.events[a].damage);
    assert(PlanJourneyAutoBattle(j).tactics.empty());

    // Finishing a nearly defeated enemy needs one affordable action, not a
    // rally solely to manufacture a higher combo count.
    j.chain.Reset(w);w.armies[3].troops=1;j.RefreshIntelligence();
    const auto finish=PlanJourneyAutoBattle(j);
    assert(finish.tactics.size()==1 && finish.tactics.front().tactic!=ChainTactic::Oath);
    j.chain.Reset(w);w.armies[3].troops=0;j.RefreshIntelligence();
    assert(PlanJourneyAutoBattle(j).tactics.empty());
}

static void VerifyCampaignSetbacks()
{
    auto session=std::make_unique<JourneySaveData>();
    auto& j=session->journey;
    j.ResetCampaign();j.Meet();
    auto& camp=j.world.cities[0];
    camp.troops=camp.food=camp.gold=0;
    auto Round=[&]
    {
        std::vector<std::uint8_t> bytes;
        auto loaded=std::make_unique<JourneySaveData>();
        assert(EncodeJourneySave(*session,bytes) && DecodeJourneySave(bytes,*loaded));
        assert(loaded->journey.stage==j.stage && loaded->journey.mission==j.mission && loaded->journey.won==j.won);
    };
    auto StayInCamp=[&]
    {
        assert(!j.CanPrepare(0) && !j.CanPrepare(1) && j.AcceptCampaignSetback());
        const auto& record=j.history[j.mission];
        assert(record.completed && !record.won && !record.rescued && record.review.resolution==JourneyResolution::Unfought);
        assert(!j.AcceptCampaignSetback() && !BuildJourneyEpisode(j).lines.empty());
        Round();
    };
    for(int mission=0;mission<YellowTurbanMission;++mission)
    {
        assert(j.mission==mission);
        StayInCamp();
        assert(j.world.cities[0].troops==0 && j.world.cities[0].food==0);
        assert(j.Continue());Round();
    }
    assert(j.mission==YellowTurbanMission && j.CanPrepare(0) && j.world.cities[0].gold==0);
    j.world.cities[0].food=0;
    StayInCamp();
    assert(j.CanChooseCoalitionPlan() && !BuildCoalitionEpisode(j).lines.empty());
    assert(j.ChooseCoalitionPlan(CoalitionPlan::Protect) && j.CanPrepare(0));
    session->beforeBattle=j;
    assert(j.Prepare(0,1) && j.world.cities[0].gold==0 && !j.CanAcceptCampaignSetback());
    j.world.armies[0].troops-=142;
    assert(j.Withdraw());Round();
    const int casualties=j.casualties;
    const auto settled=j.world.cities[0];
    auto retry=std::make_unique<StrategistJourney>(j);
    assert(retry->RetryAtCouncil(*session->beforeBattle) && retry->CanPrepare(0));
    assert(!j.CanContinue() && j.AcceptCampaignSetback() && !j.RetryAtCouncil(*session->beforeBattle));
    session->beforeBattle.reset();
    assert(j.history[CoalitionMission].withdrawn && j.history[CoalitionMission].casualties==casualties &&
        j.world.cities[0].troops==settled.troops && j.world.cities[0].food==settled.food);
    assert(j.CanBeginYangren() && j.Continue() && !j.HasCoalitionGuide());Round();
    j.world.cities[0].food=0;
    StayInCamp();
    assert(!j.SunJianTrust() && !j.CanChooseLuoyangPlan(LuoyangPlan::WesternDepot) && j.ChooseLuoyangPlan(LuoyangPlan::SouthGate));
    StayInCamp();
    assert(j.stage==JourneyStage::Complete && !j.CanContinue() && !j.CanAcceptCampaignSetback());
}

static void VerifyGuangzongChoices(const JourneySaveData& deployed)
{
    auto session=std::make_unique<JourneySaveData>(deployed);
    auto& battle=session->journey;
    const int nearFront=Campaign::At(11,8);
    battle.world.armies[0].tile=battle.world.armies[0].target=Campaign::At(10,8);
    battle.world.armies[2].tile=battle.world.armies[2].target=Campaign::At(10,10);
    battle.RefreshIntelligence();
    assert(!battle.GuangzongCommandsLeft() && battle.Move(0,nearFront));
    assert(battle.GuangzongCommandsLeft()==GuangzongWindow);
    const auto known=battle.ReadEnemyPlans();
    const auto reserve=std::find_if(known.begin(),known.end(),[](const auto& plan){return plan.army==4;});
    assert(reserve!=known.end() && reserve->target==GuangzongRoad);
    auto advancing=std::make_unique<StrategistJourney>(battle);
    assert(advancing->EndTurn() && advancing->world.armies[4].tile!=GuangzongReserve);
    for(int i=1;i<GuangzongWindow;++i) assert(advancing->EndTurn());
    assert(!advancing->GuangzongCommandsLeft());
    auto realtime=std::make_unique<StrategistJourney>(battle);
    JourneyRealtime movement;
    assert(movement.Queue(*realtime,2,JourneyDirective::March,GuangzongRoad));
    assert(movement.Resolve(*realtime) && realtime->battleReview.guangzong.roadOfficer==2);

    std::vector<std::uint8_t> bytes;
    assert(EncodeJourneySave(*session,bytes) && DecodeJourneySave(bytes,*session));
    assert(battle.GuangzongCommandsLeft()==GuangzongWindow);
    for(const bool aidStreet:{false,true})
    {
        for(const bool swift:{false,true})
        {
            auto victory=std::make_unique<StrategistJourney>(battle);
            if(aidStreet) assert(victory->Move(2,GuangzongRoad));
            assert(victory->battleReview.guangzong.roadOfficer==(aidStreet ? 2:-1));
            // Set up the last move after the two enemy detachments have been fought off.
            victory->world.armies[3].troops=victory->world.armies[4].troops=0;
            if(!swift)
                while(victory->GuangzongCommandsLeft()) assert(victory->EndTurn());
            victory->world.armies[1].tile=victory->world.armies[1].target=Campaign::At(11,7);
            victory->RefreshIntelligence();
            assert(victory->Move(1,victory->goal) && victory->won && victory->Continue());
            assert(victory->GuangzongSupport().swift==swift);
            assert(victory->GuangzongSupport().roadOfficer==(aidStreet ? 2:-1));
            for(const auto plan:{CoalitionPlan::Advance,CoalitionPlan::Protect})
            {
                auto next=std::make_unique<StrategistJourney>(*victory);
                assert(next->ChooseCoalitionPlan(plan));
                const auto starts=next->PreparationPositions(1);
                assert((starts[1]!=next->Definition().positions[1])==aidStreet);
                assert((starts[2]!=next->Definition().positions[2])==swift);
                for(int tile:starts) assert(next->PlanningTerrain(tile)!=Terrain::River);
                assert(next->Prepare(1,next->enemyPolicy));
                for(int i=0;i<3;++i) assert(next->world.armies[i].tile==starts[i]);
            }
        }
    }
}

static void PlayGuangzongWithThreeOfficers(const StrategistJourney& council)
{
    auto battle=std::make_unique<StrategistJourney>(council);
    battle->SelectDifficulty(JourneyDifficulty::Standard);
    assert(battle->Prepare(1,1));
    auto move=[&](int officer,int x,int y){assert(battle->Move(officer,Campaign::At(x,y)));};
    move(0,10,8);move(1,9,8);move(2,10,11);
    assert(battle->battleReview.guangzong.roadOfficer==2 && battle->EndTurn());
    move(0,11,8);move(1,10,8);move(2,10,10);
    assert(battle->EndTurn());
    move(1,10,7);move(2,10,9);
    assert(battle->Execute({{0,3,ChainTactic::Disrupt}}).completed==1 && battle->EndTurn());
    move(1,11,7);move(2,11,9);
    const auto combo=battle->Execute({{0,3,ChainTactic::Disrupt},{1,3,ChainTactic::AzureDragon},{2,3,ChainTactic::Breakthrough}});
    assert(combo.completed==3 && battle->won && battle->turn==4 &&
        battle->GuangzongSupport().swift && battle->GuangzongSupport().roadOfficer==2);
}

int main()
{
    VerifyAutoTeamMovement();
    VerifyAutoEscortGroup();
    VerifyAutoCombos();
    VerifyCampaignSetbacks();
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
    PlayGuangzongWithThreeOfficers(old->journey);
    old->beforeBattle=old->journey;
    assert(old->journey.Prepare(1,old->journey.enemyPolicy) && old->journey.goal==old->journey.Definition().goal && !old->journey.HasConvoy());
    assert(old->journey.world.generals[3].name==EpisodeSpeakerName(ZhangLiang) &&
        old->journey.world.generals[3].trait==Trait::Resolute && old->journey.world.generals[3].tactic==Tactic::Rally);
    VerifyGuangzongChoices(*old);
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
            if(west)
            {
                const int roadblock=DepotColumnStart+1;
                auto oldBattle=std::make_unique<JourneySaveData>(*next);
                auto& oldJourney=oldBattle->journey;
                const auto& previousDefinition=LuoyangMissionDefinition(LuoyangPlan::WesternDepot);
                oldJourney.goal=previousDefinition.goal;
                oldJourney.world.armies[4].arm=Arm::Spear;
                for(int id=3;id<=4;++id)
                    oldJourney.world.armies[id].tile=oldJourney.world.armies[id].target=previousDefinition.positions[id];
                oldJourney.RefreshIntelligence();
                std::vector<std::uint8_t> oldBytes;
                JourneyArchive archive{false,oldBytes};archive.version=18;
                unsigned oldVersion=18,oldChecksum=0;
                archive(magic,oldVersion,oldChecksum);archive(*oldBattle);
                oldChecksum=2166136261u;
                for(std::size_t i=12;i<oldBytes.size();++i) { oldChecksum^=oldBytes[i];oldChecksum*=16777619u; }
                for(int i=0;i<4;++i) oldBytes[8+i]=static_cast<std::uint8_t>(oldChecksum>>(i*8));
                assert(DecodeJourneySave(oldBytes,*oldBattle) && !oldJourney.HasDepotInterception() &&
                    oldJourney.goal==previousDefinition.goal && oldJourney.world.armies[4].arm==Arm::Spear);
                assert(oldJourney.Withdraw() && oldJourney.RetryAtCouncil(*oldBattle->beforeBattle) &&
                    oldJourney.Prepare(1,oldJourney.enemyPolicy) && oldJourney.HasDepotInterception());

                auto tactics=std::make_unique<StrategistJourney>(j);
                auto& units=tactics->world.armies;
                units[0].tile=units[0].target=DepotColumnStart-1;
                units[1].tile=units[1].target=Campaign::At(9,6);
                units[2].tile=units[2].target=Campaign::At(12,7);
                tactics->RefreshIntelligence();
                assert(tactics->Preview({{0,4,ChainTactic::Lure,DepotColumnStart-2}}).completed==0);
                const auto choices=ReadJourneyBattleChoices(*tactics,3);
                assert(!choices.empty() && choices.front().opensDepotColumn && !tactics->chain.disordered[3]);
                assert(tactics->Execute(choices.front().orders).completed==1 && tactics->stage==JourneyStage::Battle);
                assert(tactics->Move(2,roadblock) && tactics->won);

                auto blocked=std::make_unique<StrategistJourney>(j);
                blocked->world.armies[2].tile=blocked->world.armies[2].target=roadblock;
                assert(blocked->EndTurn() && blocked->EndTurn() && blocked->stage==JourneyStage::Battle &&
                    blocked->world.armies[4].tile==DepotColumnStart);
                auto lost=std::make_unique<StrategistJourney>(j);
                lost->world.armies[0].tile=lost->world.armies[0].target=DepotColumnStart-1;
                lost->world.armies[4].troops=10;
                lost->RefreshIntelligence();
                assert(lost->Strike(0,4) && lost->stage==JourneyStage::Debrief && !lost->won);

                auto retreat=std::make_unique<StrategistJourney>(j);
                auto& escaping=retreat->world.armies;
                for(int officer=0;officer<3;++officer) escaping[officer].tile=escaping[officer].target=Campaign::At(3,officer+3);
                retreat->RefreshIntelligence();
                for(int cycle=1;cycle<=8;++cycle)
                {
                    assert(retreat->EndTurn());
                    assert(escaping[4].tile==DepotColumnStart+cycle/2);
                }
                assert(retreat->stage==JourneyStage::Debrief && !retreat->won);
            }
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
            w.armies[3].troops=0;
            if(!west) w.armies[4].troops=0;
            const int marchFrom=Campaign::At(west ? 12:10,west ? 7:5);
            w.armies[0].tile=w.armies[0].target=marchFrom;
            payout->RefreshIntelligence();
            int expectedFood=w.cities[0].food+payout->reserveFood+(west ? WesternDepotFood:0);
            for(int army=0;army<3;++army) expectedFood+=w.armies[army].food;
            assert(payout->Move(0,west ? DepotColumnStart+1:payout->goal) && payout->won && w.cities[0].food==expectedFood);
            assert(j.Continue() && EncodeJourneySave(*next,bytes) && DecodeJourneySave(bytes,*next) &&
                j.luoyangPlan==route && j.history[LuoyangMission].won && !j.CanContinue());
        }
        battle.ResetCampaign();
        assert(battle.coalitionPlan==CoalitionPlan::None && !battle.history[CoalitionMission].completed && EncodeJourneySave(*old,before));
    }
    hold->stage=JourneyStage::Debrief;const auto ended=PlanJourneyAutoBattle(*hold);assert(ended.tactics.empty());
    for(const auto& order:ended.movement.orders) assert(order.kind==JourneyDirective::None);
}
