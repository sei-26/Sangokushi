#include "TacticalTrial.h"
#include "TacticalState.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "JourneyDefense.hpp"
FMissionProgress ATacticalField::CaptureMissionProgress() const
{
    const auto& J=State->Journey;FMissionProgress P;
    P.CommandTurn=J.turn;
    P.bBattle=J.stage==frontline::JourneyStage::Battle;P.bRescue=J.IsRescue();
    if(J.HasConvoy()) { P.Convoy=J.convoyStep;P.Total=int(J.ConvoyPath().size())-1;P.ConvoyNext=J.ConvoyPath()[FMath::Min(P.Convoy+1,P.Total)]; }
    if(J.Definition().kind==frontline::JourneyObjective::HoldPositions)
    {
        P.Held=J.heldTurns;P.Required=J.RequiredHoldTurns();
        P.PrimaryGoal=J.goal;P.SecondaryGoal=J.Definition().secondGoal;
        for(int Goal:{J.goal,J.Definition().secondGoal})
        {
            const auto C=frontline::ReadJourneyDefenseCheck(J,Goal);FBeaconDefenseProgress D;
            D.Goal=Goal;D.Guard=C.guard;D.Turn=C.turn;D.Reason=int(C.reason);
            D.bSecured=C.secured;D.bGuarded=C.guarded;D.bAttackSpent=C.attackSpent;P.DefenseChecks.Add(D);
        }
    }
    if(bRealtimeCommand) for(int A=0;A<3;++A)
        if(State->Realtime.orders[A].kind==frontline::JourneyDirective::March || State->Realtime.orders[A].kind==frontline::JourneyDirective::Attack) P.DirectedMovers|=1<<A;
    for(int I=0;I<int(J.battleReview.relief.size());++I)
        if(J.battleReview.relief[I].outcome==frontline::JourneyReliefOutcome::Held || J.battleReview.relief[I].outcome==frontline::JourneyReliefOutcome::Recovered) P.ConfirmedReliefs.Add(I);
    return P;
}
void ATacticalField::PublishMissionResponse()
{
    if(State->bNoPlayback || State->Beats.IsEmpty()) return;
    auto Response=State->Beats.Num()==2 ? gunyumission::ReadSupportCombo(State->Beats):gunyumission::ReadSupport(&State->Beats.Last());
    if(Response.Line.IsEmpty())
    {
        const auto& Beat=State->Beats.Last();
        Response=gunyumission::Read(&Beat,ReadRetryDefensePayoff(Beat));
    }
    if(Response.Line.IsEmpty()) return;
    State->MissionResponse=MoveTemp(Response);State->MissionResponseRemaining=4.5f;
    const auto* Path=gunyuresponse::SoundPath(State->MissionResponse.Sound);
    if(Path && EffectsVolume>0) if(auto* Sound=LoadObject<USoundBase>(nullptr,Path))
        CueAudio=UGameplayStatics::SpawnSound2D(this,Sound,FMath::Clamp(EffectsVolume,0.f,1.f));
}
gunyuresponse::FRead ATacticalField::ReadMissionResponse() const
{
    if(IsPlaying() || State->Journey.stage!=frontline::JourneyStage::Battle || State->MissionResponseRemaining<=0) return {};
    auto R=State->MissionResponse;R.Weight=FMath::SmoothStep(0.f,.35f,State->MissionResponseRemaining);return R;
}
