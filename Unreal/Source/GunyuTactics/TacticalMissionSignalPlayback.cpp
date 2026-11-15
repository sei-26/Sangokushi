#include "TacticalTrial.h"
#include "TacticalState.h"

using namespace gunyumission;

void ATacticalField::PublishMissionSignals()
{
    if(State->bNoPlayback || State->Beats.IsEmpty()) return;
    const auto& Beat=State->Beats.Last();
    const auto Fresh=ReadSignals(&Beat,Beat.After,1.f);
    auto Store=[this](FSignal& Slot,const FSignal& Result,int Timer)
    {
        if(Result.Kind==ESignal::None) return;
        Slot=Result;
        State->MissionSignalRemaining[Timer]=SignalLifetime;
    };
    Store(State->SettledMissionSignals.Cart,Fresh.Cart,0);
    for(int Wing=0;Wing<2;++Wing)
        Store(State->SettledMissionSignals.Wings[Wing],Fresh.Wings[Wing],Wing+1);
}

FSignals ATacticalField::ReadMissionSignals() const
{
    if(!State->HUD.bBattle || State->bNoPlayback) return {};
    const auto* Beat=CurrentBeat();
    const auto& Units=VisualUnits();
    auto Signals=ReadSignals(Beat,Units,BeatProgress());
    auto Keep=[this,&Units](FSignal& Live,const FSignal& Settled,int Timer)
    {
        const float Left=State->MissionSignalRemaining[Timer];
        if(Live.Kind!=ESignal::None || Settled.Kind==ESignal::None || Left<=0.f) return;
        const bool Cart=Timer==0;
        const int Id=Cart ? frontline::StrategistJourney::ConvoyArmy:Settled.Actor;
        if(Cart || Settled.Kind==ESignal::WingHeld)
        {
            const FVector Position=Cart ? Settled.Position:SurfaceLocation(Settled.Tile);
            if(!Units.IsValidIndex(Id) || Units[Id].Id!=Id || Units[Id].Troops<=0 ||
                FVector::DistSquared2D(Units[Id].Position,Position)>25.f) return;
        }
        Live=Settled;
        Live.Weight=FMath::SmoothStep(0.f,.55f,Left);
        Live.Phase=FMath::Lerp(Settled.Phase,1.f,FMath::Clamp((SignalLifetime-Left)/.85f,0.f,1.f));
    };
    Keep(Signals.Cart,State->SettledMissionSignals.Cart,0);
    for(int Wing=0;Wing<2;++Wing)
        Keep(Signals.Wings[Wing],State->SettledMissionSignals.Wings[Wing],Wing+1);
    return Signals;
}
