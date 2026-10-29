#include "../NewGame/JourneyArchive.hpp"
#include "../NewGame/JourneyBattlefield.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::unique_ptr<JourneySaveData> Council(int mission=0)
{
    auto s=std::make_unique<JourneySaveData>();s->journey.ResetCampaign();s->journey.Meet();s->journey.mission=mission;return s;
}
static std::unique_ptr<JourneySaveData> Round(const JourneySaveData& s)
{
    std::vector<std::uint8_t> bytes;assert(EncodeJourneySave(s,bytes));
    auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(bytes,*loaded));return loaded;
}
static int TotalFood(const StrategistJourney& j)
{
    int amount=j.world.cities[0].food+j.reserveFood;
    if(j.stage==JourneyStage::Battle)
        for(const auto& a:j.world.armies) if(a.faction==0) amount+=a.food+a.cargoFood;
    return amount;
}
int main()
{
    auto opening=Council();opening->beforeBattle=opening->journey;assert(opening->journey.Prepare(0,0));
    const std::vector<ChainOrder> chain{{0,3,ChainTactic::Lure,Campaign::At(8,11)},{1,3,ChainTactic::Ambush,-1},{2,3,ChainTactic::Breakthrough,-1}};
    opening->journey.Preview(chain);assert(opening->journey.battleReview.openingAmbushes==0);
    opening->journey.Execute(chain);assert(opening->journey.battleReview.openingAmbushes==1);
    assert(Round(*opening)->journey.battleReview.openingAmbushes==1);
    // The player's actual forest order changes the opening, without paying a battle action.
    auto v=Council();auto& a=v->journey;
    assert(a.AssignDuty(1,Campaign::At(9,8),JourneyDuty::Ambush));
    assert(a.PreparationPositions(0)[1]==Campaign::At(9,8));
    assert(a.PreparationPositions(1)[1]==a.Definition().positions[1]);
    const int originalFood=TotalFood(a);v->beforeBattle=a;assert(a.Prepare(0,0));
    assert(a.world.armies[1].tile==Campaign::At(9,8) && a.chain.concealed[1] && !a.chain.moved[1] && !a.chain.spent[1]);
    assert(a.assignments[1].completed && !a.CanResupply() && !a.Resupply() && TotalFood(a)==originalFood);
    JourneyRealtime orders;orders.ApplyAssignments(a);assert(orders.orders[1].kind==JourneyDirective::None);
    assert(a.Move(1,Campaign::At(8,8)) && !a.chain.concealed[1]); // Moving forfeits the paid concealment.
    // Distant duties remain real marches, not unrestricted teleportation.
    v=Council();assert(v->journey.AssignDuty(1,Campaign::At(3,3),JourneyDuty::Ambush));
    assert(v->journey.PreparationPositions(0)[1]==v->journey.Definition().positions[1]);
    v=Council();assert(v->journey.AssignDuty(1,v->journey.Definition().positions[0],JourneyDuty::Hold));
    assert(v->journey.PreparationPositions(0)[1]==v->journey.Definition().positions[1]);
    // Same public order preview under all hidden enemy policies, and no overlapping starts.
    for(int mission=0;mission<3;++mission) for(int policy=0;policy<3;++policy)
    {
        auto s=Council(mission);auto& j=s->journey;
        const int target=mission==0 ? Campaign::At(9,8):mission==1 ? Campaign::At(9,9):Campaign::At(7,6);
        assert(j.AssignDuty(1,target,JourneyDuty::Ambush));const auto plan=j.PreparationPositions(0);
        j.enemyPolicy=policy;assert(j.PreparationPositions(0)==plan);assert(j.Prepare(0,policy));
        assert(j.world.armies[1].tile==plan[1]);
        for(std::size_t i=0;i<j.world.armies.size();++i) for(std::size_t k=0;k<i;++k) assert(j.world.armies[i].tile!=j.world.armies[k].tile);

    }
    auto scouted=Council();assert(scouted->journey.AssignDuty(1,Campaign::At(7,9),JourneyDuty::Ambush));
    assert(scouted->journey.Prepare(0,2) && !scouted->journey.chain.concealed[1]); // A genuine enemy scout counters the setup.
    // The reserve is paid at deployment, not secretly added on top of the food total.
    auto s=Council();auto& j=s->journey;const auto camp=j.world.cities[0];s->beforeBattle=j;
    assert(j.Prepare(1,0) && j.reserveFood==1800 && j.world.armies[0].food==1000 && j.world.armies[0].morale==90);
    assert(TotalFood(j)==camp.food && j.world.cities[0].gold==camp.gold-80);
    auto loaded=Round(*s);assert(loaded->journey.CanResupply());
    // Morale unlocks an ordinary solo tactic; spending the reserve does not reset action/signature state.
    j.battleReview.preparationTracked=false; // A v7 in-flight reserve must become trackable on actual use.
    j.world.armies[0].morale=5;
    const std::vector<ChainOrder> solo{{0,3,ChainTactic::Disrupt,-1}};
    assert(j.Preview(solo).completed==0);
    j.chain.spent[1]=j.chain.moved[1]=j.chain.signatureUsed[1]=true;
    const auto spent=j.chain.spent,moved=j.chain.moved,used=j.chain.signatureUsed;
    const int day=j.world.day,turn=j.turn;const auto review=j.battleReview;
    assert(j.Resupply() && j.world.armies[0].morale==25 && j.world.armies[2].food==1600);
    assert(j.world.day==day && j.turn==turn && j.battleReview.tacticAttempts==review.tacticAttempts);
    assert(j.chain.spent==spent && j.chain.moved==moved && j.chain.signatureUsed==used);
    assert(j.battleReview.resupplyTurn==turn && j.battleReview.resupplyMorale==40);
    assert(j.Preview(solo).completed==1 && !j.CanResupply() && !j.Resupply() && TotalFood(j)==camp.food);
    assert(j.Execute(solo).completed==1 && j.battleReview.postSupplySteps==1);
    loaded=Round(*s);assert(loaded->journey.battleReview.postSupplySteps==1 && !loaded->journey.CanResupply() && loaded->journey.reserveFood==0);
    assert(j.Withdraw() && j.history[j.mission].review.postSupplySteps==1 && TotalFood(j)==camp.food && !j.Resupply()); // Neither issued nor unused food disappears.
    auto withdrawn=Round(*s);assert(withdrawn->journey.reserveFood==0);
    s=Council();s->beforeBattle=s->journey;assert(s->journey.Prepare(1,0));
    assert(s->journey.Withdraw() && s->journey.world.cities[0].food==18000);Round(*s);
    s->journey=*s->beforeBattle;assert(s->journey.Prepare(1,0) && s->journey.reserveFood==1800); // Retry starts from the unpaid checkpoint.
    // Fallen troops are not revived; the living main army receives all 1,800. The cart is excluded.
    s=Council();s->journey.SelectPromise(JourneyPromise::EscortRefugees);s->beforeBattle=s->journey;auto& rescue=s->journey;
    assert(rescue.Prepare(1,0));rescue.world.armies[1].troops=0;rescue.world.armies[0].morale=99;
    const auto cart=rescue.world.armies[5];assert(rescue.Resupply());
    assert(rescue.world.armies[0].food==1900 && rescue.world.armies[0].morale==100 && rescue.world.armies[1].troops==0);
    assert(rescue.world.armies[5].food==cart.food && rescue.world.armies[5].morale==cart.morale);
    // V6 battles already allocated every ration. Loading never manufactures a new reserve.
    s=Council();s->beforeBattle=s->journey;assert(s->journey.Prepare(1,0));s->journey.reserveFood=0;
    for(int i=0;i<3;++i) s->journey.world.armies[i].food+=600;
    std::vector<std::uint8_t> old;JourneyArchive writer{false,old};writer.version=6;
    unsigned magic=0x53594e47,version=6,checksum=0;writer(magic,version,checksum);writer(*s);
    checksum=2166136261u;for(std::size_t i=12;i<old.size();++i) { checksum^=old[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) old[8+i]=static_cast<std::uint8_t>(checksum>>(i*8));
    assert(DecodeJourneySave(old,*loaded) && loaded->journey.campaignFlow && !loaded->journey.CanResupply() && loaded->journey.reserveFood==0);
    loaded->journey.reserveFood=600;assert(!ValidJourneySave(*loaded));
    auto legacy=std::make_unique<StrategistJourney>();legacy->Reset();legacy->Meet();assert(legacy->Prepare(1,0));
    assert(legacy->reserveFood==0 && legacy->world.armies[0].food==1600 && legacy->world.armies[0].morale==100 && !legacy->Resupply());
    // Optional conversations react to settled facts, not chosen plan alone; reading is immutable.
    auto dialogue=Council();dialogue->beforeBattle=dialogue->journey;assert(dialogue->journey.Prepare(1,0));
    assert(dialogue->journey.Resupply() && dialogue->journey.Withdraw());
    auto text=[](const StrategistJourney& value,int who) { std::u32string all;for(const auto& line:JourneyConversation(value,who)) all+=line.text;return all; };
    std::vector<std::uint8_t> before,after;assert(EncodeJourneySave(*dialogue,before));
    assert(text(dialogue->journey,0).find(U"帳消し")!=std::u32string::npos);
    assert(text(dialogue->journey,2).find(U"戦法にはつながらなかった")!=std::u32string::npos);
    assert(EncodeJourneySave(*dialogue,after) && before==after);
    auto& record=dialogue->journey.history[0];record.review.postSupplySteps=record.review.tacticSteps=2;
    assert(text(dialogue->journey,2).find(U"戦法を使えた")!=std::u32string::npos);
    record.review.resupplyMorale=0;
    assert(text(dialogue->journey,0).find(U"その先の行軍")!=std::u32string::npos);
    assert(text(dialogue->journey,0).find(U"早く出せば") == std::u32string::npos);
    record.review.resupplyTurn=record.review.postSupplySteps=0;
    assert(text(dialogue->journey,0).find(U"封を切らず")!=std::u32string::npos);
    dialogue->journey.preparation=0;record.review.openingAmbushes=1;
    assert(text(dialogue->journey,1).find(U"一手だけ")!=std::u32string::npos);
    record.review.preparationTracked=false;record.review.openingAmbushes=0;
    assert(text(dialogue->journey,1).find(U"初手の伏兵") == std::u32string::npos);
    std::cout<<"Preparation PASS: real forward forest deployment, counter-scouting, opening action, public preview, reserve unlocks solo tactics, one use, food conservation, dead/escort exclusions, exact save/retry and v6 compatibility\n";
}
