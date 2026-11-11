#pragma once
#include "JourneyBattlePlayer.hpp"
#include "../NewGame/JourneySave.hpp"
#include <memory>
namespace contributionqa
{
    // Actual public commands, not a fabricated settled victory or hidden enemy policy.
    inline std::unique_ptr<JourneySaveData> Play(int variant,bool withDuties=false)
    {
        auto save=std::make_unique<JourneySaveData>();auto& j=save->journey;
        j.ResetCampaign();j.SelectDifficulty(JourneyDifficulty::Story);j.Meet();j.mission=variant==3 ? 0:1;
        if(withDuties && (!j.AssignDuty(0,j.Definition().positions[0],JourneyDuty::Hold) ||
                          !j.AssignDuty(1,j.Definition().positions[1],JourneyDuty::Ambush))) return {};
        save->beforeBattle=j;
        if(!j.Prepare(variant==3 ? 0:1,0)) return {};
        if(withDuties && !j.Hold(0)) return {};
        if(variant==3)
        {
            j.Execute({{0,3,ChainTactic::Lure,Campaign::At(8,11)},{1,3,ChainTactic::Ambush,-1},{2,3,ChainTactic::Breakthrough,-1}});
            if(j.stage==JourneyStage::Battle) j.EndTurn();
            if(j.stage==JourneyStage::Battle) j.Execute({{2,3,ChainTactic::Breakthrough,-1}});
        }
        else for(int cycle=0;cycle<16 && j.stage==JourneyStage::Battle;++cycle)
        {
            if(variant!=2)
            {
                const auto contacts=j.intelligence[0].contacts;
                for(const auto& contact:contacts) if(contact.current && contact.observed.troops>0)
                {
                    std::vector<ChainOrder> orders;
                    for(int id=0;id<3;++id) if(!j.chain.spent[id] && j.world.CanStrike(id,contact.observed.tile)) orders.push_back({id,contact.id,ChainTactic::Disrupt,-1});
                    if(orders.size()>=2) { j.Execute(orders);break; }
                }
            }
            for(int id=0;id<3 && j.stage==JourneyStage::Battle;++id) if(id!=variant) Fight(j,id,false);
            if(j.stage==JourneyStage::Battle) { Approach(j,variant,j.goal,false);Fight(j,variant,false); }
            if(j.stage==JourneyStage::Battle) j.EndTurn();
        }
        if(!j.won || (variant!=3 && j.world.armies[variant].tile!=j.goal)) return {};
        return save;
    }
}
