#pragma once
#include "Campaign.hpp"
namespace frontline
{
    struct TacticBattlefieldImpact
    {
        int terrain=100,stance=100,directions=0;
        int Flank() const { return 100+std::max(0,directions-1)*15; }
        int Damage(int raw) const { return static_cast<int>(static_cast<long long>(raw)*terrain*stance*Flank()/1000000); }
        std::u32string Describe() const;
        std::u32string Hint() const;
    };
    // Use a player-observed world for UI/preview. Concealed support cannot supply
    // pressure until it attacks; this action's own officer is revealed by the hit.
    TacticBattlefieldImpact ReadTacticBattlefield(const Campaign&,const std::vector<bool>& concealed,int actor,int target);
}
