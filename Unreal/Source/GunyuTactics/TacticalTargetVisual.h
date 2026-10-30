#pragma once
#include "TacticalPresentation.h"
namespace gunyutargetui
{
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
