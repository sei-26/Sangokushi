#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyDefense.hpp"
#include "JourneyPlanExample.hpp"

using namespace frontline;

int ATacticalField::RetryDefenseSupporter(const StrategistJourney& Council)
{
    if(!Council.retryDefense) return -1;
    const auto Plan=PlanExample(Council);
    if(!Plan.remembersDefenseRetry) return -1;
    for(int Id=0;Id<3;++Id)
        if(Council.assignments[Id].duty!=Plan.assignments[Id].duty || Council.assignments[Id].tile!=Plan.assignments[Id].tile) return -1;
    return Plan.adviser;
}

void ATacticalField::RememberRetryDefenseAid(const FBattleBeat& Beat)
{
    const auto& J=State->Journey;
    const auto* Council=State->BeforeBattle.get();
    if(!Council || !Council->retryDefense || J.stage!=JourneyStage::Battle || Council->mission!=J.mission) return;
    const auto Support=gunyumission::ReadSupport(&Beat);
    if(Support.Line.IsEmpty() || RetryDefenseSupporter(*Council)!=Support.Actor) return;
    const auto& Assignment=Council->assignments[Support.ReplyActor];
    if(Assignment.duty!=JourneyDuty::Hold || Assignment.tile!=Beat.SupportGoal) return;
    if(J.lastChain.events.size()!=1) return;
    const auto& Event=J.lastChain.events.front();
    if(!Event.success || Event.army!=Support.Actor || Event.enemy!=Beat.Target ||
       !StrategistJourney::OnMap(Event.from) || J.world.MapDistance(Event.from,Beat.SupportGoal)>1 ||
       !Beat.Before[Beat.Target].Position.Equals(SurfaceLocation(Event.from),1.f)) return;
    State->RetryDefenseAid={J.turn,Support.Actor,Support.ReplyActor,Beat.SupportGoal,Beat.Target,Beat.After[Beat.Target].Troops==0};
}

gunyuresponse::FRead ATacticalField::ReadRetryDefensePayoff(const FBattleBeat& Beat) const
{
    const auto& Aid=State->RetryDefenseAid;
    const auto& J=State->Journey;
    if(Aid.Turn<0 || Beat.Kind!=EBattleCue::Enemy || !Beat.MissionBefore.bBattle || !Beat.MissionAfter.bBattle ||
       Beat.MissionBefore.CommandTurn!=Aid.Turn || J.turn!=Aid.Turn+1) return {};
    const auto Check=ReadJourneyDefenseCheck(J,Aid.Goal);
    if(!Check.secured || Check.guard!=Aid.Guard || Check.turn!=Aid.Turn) return {};
    for(int Id:{Aid.Guard,Aid.Supporter})
        if(!Beat.Before.IsValidIndex(Id) || !Beat.After.IsValidIndex(Id) ||
           Beat.Before[Id].Id!=Id || Beat.After[Id].Id!=Id ||
           Beat.Before[Id].bEnemy || Beat.After[Id].bEnemy || Beat.Before[Id].Troops<=0 || Beat.After[Id].Troops<=0) return {};
    const FVector Position=SurfaceLocation(Aid.Goal);
    if(!Beat.Before[Aid.Guard].Position.Equals(Position,1.f) || !Beat.After[Aid.Guard].Position.Equals(Position,1.f)) return {};
    if(!Aid.bDefeated)
    {
        // Keep the claim local to a target whose separation can still be seen.
        const auto* Seen=J.intelligence[0].Find(Aid.Target);
        if(!Seen || !Seen->current || Seen->observed.troops<=0 || J.world.MapDistance(Seen->observed.tile,Aid.Goal)<=1) return {};
    }
    return gunyumission::RetryDefenseVoices(Beat,Aid.Guard,Aid.Supporter,Aid.Goal==J.goal);
}
