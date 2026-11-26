#include "TacticalTrial.h"
bool ATacticalController::PrepareFirstExperiencePreview(int Phase,int Road)
{
    if(Phase<0 || Phase>5 || Road<0 || Road>1) return false;
    CloseEpisode();Field->Issue(ETacticalCommand::Skip);
    Field->bLegacyJourney=false;Field->bGrandHistoricalBattles=true;Field->bStartWithCouncil=true;
    Field->bRealtimeCommand=true;Field->FirstMission=0;Field->EnemyPolicy=0;
    Field->Issue(ETacticalCommand::Restart);bDetailedBattle=false;
    if(!Field->CanChooseFirstRoad()) return false;
    if(Phase>0)
    {
        ChooseEncounterRoad(Road);
        if(!Field->Presentation().Journey.bPreparation) return false;
        CouncilElapsed=1000;
    }
    if(Phase>=2)
    {
        DeployNovelPlan();ResetBattleCamera();
        if(!Field->Presentation().bBattle) return false;
    }
    if(Phase>=3 && Road==0)
    {
        const auto First=Field->Presentation().OpeningOrder;
        if(!Field->UseFirstStrike(First.SoloVersion,First.SoloTarget)) return false;
        Field->AdvancePresentation(60);
    }
    if(Phase>=4)
    {
        if(Field->Presentation().bAlertPause) Field->Issue(ETacticalCommand::RealtimeAlertPause);
        Field->Issue(ETacticalCommand::AutoBattle);
        for(int Cycle=0;Cycle<24 && Field->Presentation().bBattle;++Cycle)
        { Field->AdvanceRealtime(12.1f);Field->AdvancePresentation(60);Field->AdvanceRealtime(.1f);Field->AdvancePresentation(60); }
        if(Field->IsAutoBattle()) Field->StopAutoBattle();
        if(!Field->Presentation().Journey.bDebrief || !Field->Presentation().bWon) return false;
    }
    if(Phase==5) Field->RevealFirstReply();
    NovelElapsed=1000;return true;
}
