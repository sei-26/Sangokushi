#pragma once
#include "TacticalPresentation.h"
#include "HexGrid.hpp"
#include "CampaignTypes.hpp"
namespace gunyutargetui
{
    inline bool MapTargetSelected(const FTacticalPresentation& H,const FVisualUnit& U)
    {
        if(H.bBusy || U.Id<0 || !U.bEnemy) return false;
        for(const auto& Read:H.TacticTargets) if(H.bTacticTargetOverlay && Read.Army==U.Id && Read.bSelected) return true;
        const auto Cell=hexgrid::Pick(U.Position.X/200.,U.Position.Y/200.);
        const int Tile=Cell.first+Cell.second*frontline::Width;
        for(const auto& Read:H.EnemyReads) if(Read.bSelected && Read.Tile==Tile) return true;
        return false;
    }
    inline const FTacticTargetPresentation* Find(const FTacticalPresentation& H,int Army)
    {
        if(!H.bTacticTargetOverlay || H.bBusy) return nullptr;
        for(const auto& Read:H.TacticTargets) if(Read.Army==Army) return &Read;
        return nullptr;
    }
    inline FLinearColor Color(const FTacticTargetPresentation& Read)
    {
        return !Read.bReady ? FLinearColor(.70f,.66f,.59f):
            Read.Step==1 ? FLinearColor(.25f,.94f,.86f):FLinearColor(1.f,.78f,.28f);
    }
}
