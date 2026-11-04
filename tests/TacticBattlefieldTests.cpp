#include "TacticBattlefield.hpp"
#include "JourneySave.hpp"
#include <cassert>
#include <memory>
using namespace frontline;
static std::unique_ptr<JourneySaveData> Setup(bool campaign=true)
{
    auto s=std::make_unique<JourneySaveData>();s->journey.Reset();s->journey.campaignFlow=campaign;s->journey.Meet();
    s->beforeBattle=s->journey;assert(s->journey.Prepare(1,0));auto& w=s->journey.world;
    w.armies[0].tile=w.armies[0].target=Campaign::At(9,10);w.armies[3].tile=w.armies[3].target=Campaign::At(10,10);
    w.armies[1].troops=w.armies[2].troops=w.armies[4].troops=0;w.armies[3].troops=6000;w.armies[3].morale=100;
    w.armies[3].stance=battle::Stance::Balanced;w.tiles[w.armies[3].tile].terrain=Terrain::Plain;
    s->journey.RefreshIntelligence();return s;
}
static int Hit(Terrain terrain,battle::Stance stance)
{
    auto s=Setup();auto& j=s->journey;j.world.tiles[j.world.armies[3].tile].terrain=terrain;j.world.armies[3].stance=stance;j.RefreshIntelligence();
    std::vector<std::uint8_t> before,after;assert(EncodeJourneySave(*s,before));
    const auto p=j.Preview({{0,3,ChainTactic::Feint,-1}});assert(EncodeJourneySave(*s,after) && before==after);
    const auto a=j.Execute({{0,3,ChainTactic::Feint,-1}});assert(a.completed==1 && a.events[0].damage==p.events[0].damage);return a.events[0].damage;
}
int main()
{
    const int plain=Hit(Terrain::Plain,battle::Stance::Balanced);
    assert(Hit(Terrain::Forest,battle::Stance::Balanced)<plain && Hit(Terrain::Mountain,battle::Stance::Balanced)<Hit(Terrain::Forest,battle::Stance::Balanced));
    assert(Hit(Terrain::Plain,battle::Stance::Guard)<plain && Hit(Terrain::Plain,battle::Stance::Assault)>plain);
    auto s=Setup();auto& j=s->journey;auto& w=j.world;
    w.armies[1].troops=1400;w.armies[1].morale=90;w.armies[1].tile=w.armies[1].target=Campaign::At(11,10);
    auto field=ReadTacticBattlefield(w,j.chain.concealed,0,3);assert(field.directions==2 && field.Flank()==115);
    j.chain.concealed[1]=true;assert(ReadTacticBattlefield(w,j.chain.concealed,0,3).directions<2);
    assert(ReadTacticBattlefield(w,j.chain.concealed,1,3).directions==2);j.chain.concealed[1]=false;
    w.armies[1].troops=1199;assert(ReadTacticBattlefield(w,j.chain.concealed,0,3).directions<2);w.armies[1].troops=1400;
    w.armies[1].tile=w.armies[1].target=Campaign::At(9,9);assert(ReadTacticBattlefield(w,j.chain.concealed,0,3).directions<2);
    w.armies[1].tile=w.armies[1].target=Campaign::At(11,10);j.RefreshIntelligence();
    const auto p=j.Preview({{0,3,ChainTactic::Feint,-1}}),a=j.Execute({{0,3,ChainTactic::Feint,-1}});
    assert(p.events[0].damage==a.events[0].damage && a.events[0].damage>plain && a.events[0].reason.find(U"挟撃")!=std::u32string::npos);
    // Breaking the guard benefits its own hit and all later hands, without removing terrain cover.
    auto guard=Setup(),balanced=Setup();guard->journey.world.armies[3].stance=battle::Stance::Guard;
    guard->journey.RefreshIntelligence();const auto g=guard->journey.Execute({{0,3,ChainTactic::Disrupt,-1}}),b=balanced->journey.Execute({{0,3,ChainTactic::Disrupt,-1}});
    assert(g.events[0].damage==b.events[0].damage && guard->journey.world.armies[3].stance==battle::Stance::Balanced);
    // Cautious guards react to an observed pincer. A successful re-form costs
    // their brace; disorder prevents the move, and objective defenders stay put.
    auto cautious=Setup();auto& c=cautious->journey;c.enemyPolicy=1;
    c.world.armies[3].stance=battle::Stance::Guard;
    c.world.armies[1].troops=1400;c.world.armies[1].morale=90;c.chain.concealed[1]=false;
    c.world.armies[1].tile=c.world.armies[1].target=Campaign::At(11,10);c.RefreshIntelligence();
    const int revision=c.world.revision,from=c.world.armies[3].tile;
    auto plans=c.ReadEnemyPlans();const JourneyEnemyPlan* reform=nullptr;
    for(const auto& plan:plans) if(plan.army==3) reform=&plan;
    assert(reform && reform->intent==JourneyEnemyIntent::Guard && reform->next!=from && c.world.revision==revision);
    const int expected=reform->next;
    auto Exposure=[](const Campaign& position)
    {
        int damage=0;for(int i=0;i<3;++i) if(position.armies[i].troops>0 && position.CanStrike(i,position.armies[3].tile))
            damage+=ReadTacticBattlefield(position,{},i,3).Damage(100+position.armies[i].troops/12);
        return damage;
    };
    auto future=std::make_unique<Campaign>(c.world);future->armies[3].tile=expected;future->armies[3].stance=battle::Stance::Balanced;
    assert(Exposure(*future)<Exposure(c.world));
    c.chain.disordered[3]=true;plans=c.ReadEnemyPlans();
    for(const auto& plan:plans) if(plan.army==3) assert(plan.intent==JourneyEnemyIntent::Recover && plan.next==from);
    c.chain.disordered[3]=false;assert(c.EndTurn());
    assert(c.world.armies[3].tile==expected && c.world.armies[3].stance==battle::Stance::Balanced);
    auto port=std::make_unique<StrategistJourney>();port->ResetCampaign();port->mission=1;port->Meet();assert(port->Prepare(1,1));
    port->world.armies[3].tile=port->world.armies[3].target=port->goal;port->world.armies[3].stance=battle::Stance::Guard;
    port->world.armies[0].tile=port->world.armies[0].target=Campaign::At(11,11);
    port->world.armies[1].tile=port->world.armies[1].target=Campaign::At(13,11);port->chain.concealed[1]=false;port->RefreshIntelligence();
    for(const auto& plan:port->ReadEnemyPlans()) if(plan.army==3) assert(plan.next==plan.from);
    // Old noncampaign trials keep their original damage rules and save schema.
    auto old=Setup(false);old->journey.world.tiles[old->journey.world.armies[3].tile].terrain=Terrain::Mountain;
    old->journey.world.armies[3].stance=battle::Stance::Guard;old->journey.RefreshIntelligence();
    assert(old->journey.Execute({{0,3,ChainTactic::Feint,-1}}).events[0].damage==plain);
    std::vector<std::uint8_t> bytes;assert(EncodeJourneySave(*old,bytes) && bytes[4]==11);
}
