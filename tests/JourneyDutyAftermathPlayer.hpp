#pragma once
#include "JourneyContributionPlayer.hpp"

namespace dutyaftermathqa
{
    // Every outcome is settled by the public battle rules after normal preparation.
    inline std::unique_ptr<JourneySaveData> Play(int variant)
    {
        if(variant==0) return contributionqa::Play(2,true);

        auto save=std::make_unique<JourneySaveData>();
        auto& j=save->journey;
        j.ResetCampaign();
        j.Meet();
        if(!j.SelectPromise(JourneyPromise::EscortRefugees)) return {};
        if(variant==2)
        {
            if(!j.AssignDuty(0,j.Definition().positions[0],JourneyDuty::Hold)) return {};
        }
        else if(!j.AssignDuty(1,j.Definition().positions[1],JourneyDuty::Ambush)) return {};
        save->beforeBattle=j;
        if(!j.Prepare(0,1)) return {};

        if(variant==2)
        {
            // The unit is already at its post. A withdrawal before using Hold cannot earn credit.
            if(j.world.armies[0].tile!=j.assignments[0].tile || !j.Withdraw()) return {};
            return save;
        }
        if(variant==3) return j.Withdraw() ? std::move(save):nullptr;
        for(int phase=1;phase<=10 && j.stage==JourneyStage::Battle;++phase)
        {
            if(phase==1 && (!j.Move(0,Campaign::At(9,9)) || !j.Move(2,Campaign::At(9,10)))) return {};
            if(phase==2)
            {
                if(!j.Move(0,Campaign::At(10,9)) || !j.Move(1,Campaign::At(9,9))) return {};
                if(j.Execute({{0,3,ChainTactic::Feint,-1},{1,3,ChainTactic::Disrupt,-1},{2,3,ChainTactic::Breakthrough,-1}}).completed!=3) return {};
            }
            if(phase>=2) for(int id=0;id<3 && j.stage==JourneyStage::Battle;++id) Fight(j,id,true);
            if(j.stage==JourneyStage::Battle) j.EndTurn();
        }
        if(!j.won || !j.history[0].rescued) return {};
        return save;
    }
}
