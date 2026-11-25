#pragma once
#include "TacticalTrial.h"

namespace gunyucommand
{
    inline const FTacticTargetPresentation* MissionEnemy(const FTacticalPresentation& H)
    {
        if(!H.bBattle || H.bBusy || !H.bTacticTargetOverlay || H.MissionFocusTile<0) return nullptr;
        for(const auto& Officer:H.Officers)
            if(Officer.bLiving && Officer.Tile==H.MissionFocusTile) return nullptr;
        return H.TacticTargets.FindByPredicate([&](const FTacticTargetPresentation& T)
            { return T.Army>=0 && T.Tile==H.MissionFocusTile && !T.bSelected; });
    }

    inline bool SelectMissionEnemy(ATacticalField& Field,int ExpectedArmy,int ExpectedTile)
    {
        if(Field.IsPlaying() || Field.IsAutoBattle()) return false;
        const auto* Current=MissionEnemy(Field.Presentation());
        if(!Current || Current->Army!=ExpectedArmy || Current->Tile!=ExpectedTile) return false;
        // Revalidate the public contact at release, then use normal selection.
        // No movement or tactic is issued, and pending reservations stay intact.
        if(Field.bRealtimeCommand && !Field.RealtimePaused()) Field.Issue(ETacticalCommand::RealtimePause);
        Field.SelectAt(ExpectedTile);
        return Field.Presentation().TacticTargets.ContainsByPredicate([=](const FTacticTargetPresentation& T)
            { return T.Army==ExpectedArmy && T.Tile==ExpectedTile && T.bSelected; });
    }
}
