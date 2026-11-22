#include "TacticalGrandMoment.h"
#include "TacticalTrial.h"
#include "TacticalState.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void ATacticalField::PublishGrandMoment()
{
    if(State->bNoPlayback || State->Beats.IsEmpty()) return;
    const auto& Beat=State->Beats.Last();
    auto Read=gunyugrand::Compare(Beat.MissionBefore.Grand,Beat.MissionAfter.Grand);
    if(Read.Kind==EGrandMoment::None) return;
    const auto& U=Beat.After;
    if(!U.IsValidIndex(Read.FocusArmy) || U[Read.FocusArmy].Id<0 || U[Read.FocusArmy].Troops<=0) return;
    State->GrandMoment=MoveTemp(Read);State->GrandMomentRemaining=5.f;
    if(CueAudio) CueAudio->Stop();
    if(EffectsVolume<=0) return;
    const TCHAR* Path=State->GrandMoment.Kind==EGrandMoment::Counter ? TEXT("/Game/Audio/S_TacticStopV1.S_TacticStopV1"):
        State->GrandMoment.bVictory ? TEXT("/Game/Audio/S_TacticCombo3V1.S_TacticCombo3V1"):TEXT("/Game/Audio/S_Command.S_Command");
    if(auto* Sound=LoadObject<USoundBase>(nullptr,Path)) CueAudio=UGameplayStatics::SpawnSound2D(this,Sound,FMath::Clamp(EffectsVolume,0.f,1.f));
}

FGrandMoment ATacticalField::ReadGrandMoment() const
{
    if(IsPlaying() || State->GrandMomentRemaining<=0 || State->GrandMoment.Mission!=State->Journey.mission) return {};
    if(State->Journey.stage!=frontline::JourneyStage::Battle && !(State->GrandMoment.bVictory && State->Journey.stage==frontline::JourneyStage::Debrief)) return {};
    auto R=State->GrandMoment;R.Age=5.f-State->GrandMomentRemaining;
    R.Weight=FMath::SmoothStep(0.f,.25f,R.Age)*FMath::SmoothStep(0.f,.7f,State->GrandMomentRemaining);
    return R;
}
void ATacticalField::DismissGrandMoment()
{ State->GrandMoment={};State->GrandMomentRemaining=0; }
