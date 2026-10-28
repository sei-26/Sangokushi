#include "../NewGame/JourneySave.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
using namespace frontline;
static std::unique_ptr<StrategistJourney> Formation()
{
    auto j=std::make_unique<StrategistJourney>();j->Reset();j->Meet();assert(j->Prepare(1,0));
    const int cells[]={Campaign::At(9,9),Campaign::At(10,8),Campaign::At(8,9),Campaign::At(9,8)};
    for(int i=0;i<4;++i) j->world.armies[i].tile=j->world.armies[i].target=cells[i];
    j->RefreshIntelligence();return j;
}
int main()
{
    // Every loadout tactic can resolve as a single order without preceding ally actions.
    for(int actor=0;actor<3;++actor)
    {
        auto base=Formation();
        for(auto tactic:TacticChain::Loadout(base->world,actor))
        {
            auto j=std::make_unique<StrategistJourney>(*base);
            for(int ally=0;ally<3;++ally) if(ally!=actor) j->world.armies[ally].troops=0;
            j->RefreshIntelligence();
            const int dest=tactic==ChainTactic::Lure ? Campaign::At(9,10):-1;
            const std::vector<ChainOrder> orders{{actor,tactic==ChainTactic::Oath?-1:3,tactic,dest}};
            const auto preview=j->Preview(orders),actual=j->Execute(orders);
            assert(actual.completed==1 && actual.events[0].combo==1 && actual.events[0].comboPercent==100);
            assert(preview.completed==1 && preview.events[0].damage==actual.events[0].damage);
            assert(j->chain.spent[actor]);
        }
    }
    // Same attacker, troops and tactic: only its combo slot changes.
    int damage[3]{};
    for(int count=1;count<=3;++count)
    {
        auto j=Formation();std::vector<ChainOrder> orders;
        for(int actor=0;actor<count-1;++actor) orders.push_back({actor,3,ChainTactic::Feint,-1});
        orders.push_back({2,3,ChainTactic::Feint,-1});
        const int troops=j->world.armies[3].troops,revision=j->world.revision;
        const auto preview=j->Preview(orders);assert(j->world.revision==revision && j->world.armies[3].troops==troops);
        const auto actual=j->Execute(orders);assert(actual.completed==count);
        damage[count-1]=actual.events.back().damage;
        int sum=0;for(int i=0;i<count;++i)
        {
            const auto& e=actual.events[i];sum+=e.damage;
            assert(e.combo==i+1 && e.comboPercent==100+25*i && e.damage==preview.events[i].damage);
        }
        assert(troops-j->world.armies[3].troops==sum);
    }
    assert(damage[1]==damage[0]*125/100 && damage[2]==damage[0]*150/100);
    // Separate dispatches reset the combo even within the same command interval.
    {
        auto j=Formation();j->Execute({{0,3,ChainTactic::Feint,-1}});
        const auto second=j->Execute({{2,3,ChainTactic::Feint,-1}});
        assert(second.events[0].combo==1 && second.events[0].damage==damage[0]);
    }
    // A stopped hand earns no combo, damage or cost; retry starts at one, never at three.
    {
        auto j=Formation();j->world.armies[1].morale=5;
        const auto actual=j->Execute({{0,3,ChainTactic::Feint,-1},{1,3,ChainTactic::Disrupt,-1},{2,3,ChainTactic::Feint,-1}});
        assert(actual.completed==1 && actual.events.size()==2 && actual.events[1].combo==0 && actual.events[1].damage==0);
        assert(!j->chain.spent[1] && !j->chain.spent[2]);
        assert(j->Execute({{2,3,ChainTactic::Feint,-1}}).events[0].combo==1);
    }
    // Preparation remains useful on top of, and distinct from, the combo multiplier.
    {
        auto plain=Formation(),prepared=Formation();prepared->chain.pressured[3]=true;
        const auto a=plain->Execute({{1,3,ChainTactic::AzureDragon,-1}});
        const auto b=prepared->Execute({{1,3,ChainTactic::AzureDragon,-1}});
        assert(a.completed==1 && b.events[0].setupPercent==125 && b.events[0].damage==a.events[0].damage*125/100);
    }
    // A solo oath helps its own officer; inspiration survives a turn and is consumed once.
    {
        auto j=Formation();j->world.armies[1].troops=j->world.armies[2].troops=0;
        assert(j->Execute({{0,-1,ChainTactic::Oath,-1}}).completed==1 && j->chain.inspired[0]);
        j->chain.BeginTurn(j->world);assert(j->chain.inspired[0]);
        const auto e=j->chain.Execute(j->world,j->intelligence[0],{{0,3,ChainTactic::Feint,-1}}).events[0];
        assert(e.damage==(1400/16+j->world.generals[j->world.armies[0].general].leadership/2)*120/100 && !j->chain.inspired[0]);
        j->chain.Reset(j->world);assert(!j->chain.inspired[0]);
    }
    // Overkill is clamped to actual remaining troops, even with a 150% finisher.
    {
        auto j=Formation();j->world.armies[3].troops=500;j->RefreshIntelligence();
        const auto r=j->Execute({{0,3,ChainTactic::Feint,-1},{2,3,ChainTactic::Feint,-1},{1,3,ChainTactic::AzureDragon,-1}});
        assert(r.completed==3 && r.events[2].comboPercent==150 && j->world.armies[3].troops==0);
        assert(r.events[0].damage+r.events[1].damage+r.events[2].damage==500);
    }
    // Genuine v1 bytes, created before this change: restore historical damage without inventing bonuses.
    {
        const auto path=std::filesystem::path(__FILE__).parent_path()/"fixtures/journey-v1-chain.bin";
        std::ifstream file(path,std::ios::binary);assert(file);
        std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});
        auto s=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(bytes,*s));
        assert(s->journey.lastChain.completed==3 && s->journey.lastChain.events[1].damage==413);
        for(const auto& e:s->journey.lastChain.events) assert(e.combo==0 && e.comboPercent==100);
        std::vector<std::uint8_t> modern;assert(EncodeJourneySave(*s,modern) && modern[4]==4);
        auto round=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(modern,*round));
        assert(round->journey.lastChain.events[1].damage==413 && round->journey.lastChain.events[1].combo==0);
        s->journey.chain.BeginTurn(s->journey.world);
        const auto result=s->journey.Execute({{2,3,ChainTactic::Breakthrough,-1}});assert(result.completed==1);
        assert(EncodeJourneySave(*s,modern) && DecodeJourneySave(modern,*round));
        assert(round->journey.lastChain.events[0].combo==1 && round->journey.lastChain.events[0].damage==result.events[0].damage);
        s->journey.lastChain.events[0].comboPercent=999;assert(!EncodeJourneySave(*s,modern));
    }
    std::cout<<"Tactic combo PASS: all solo loadouts, 100/125/150 actual damage, prediction, reset/stop, setup, support, overkill, genuine v1/v2 persistence\n";
}
