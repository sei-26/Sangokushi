#include "TacticalMissionSignals.h"
#include "TacticalPresentation.h"
#include "StrategistJourney.hpp"

namespace gunyumission
{
    namespace
    {
        bool LivingAlly(const TArray<FVisualUnit>& Units,int Id)
        { return Units.IsValidIndex(Id) && Units[Id].Id==Id && !Units[Id].bEnemy && Units[Id].Troops>0; }

        int TileAt(FVector Position)
        {
            const auto [X,Y]=hexgrid::Pick(Position.X/200.,Position.Y/200.);
            return X>=0 && X<frontline::StrategistJourney::Columns && Y>=0 && Y<frontline::StrategistJourney::Rows
                ? frontline::Campaign::At(X,Y):-1;
        }
    }

    FSignals ReadSignals(const FBattleBeat* Beat,const TArray<FVisualUnit>& Displayed,float Progress)
    {
        FSignals Read;
        if(!Beat || Beat->Kind!=EBattleCue::Enemy) return Read;
        const auto& Before=Beat->MissionBefore;
        const auto& After=Beat->MissionAfter;
        if(!Before.bBattle || !After.bBattle || Before.CommandTurn<=0 || After.CommandTurn!=Before.CommandTurn+1) return Read;

        constexpr int Transport=frontline::StrategistJourney::ConvoyArmy;
        if(Before.Convoy>=0 && After.Convoy==Before.Convoy+1 && After.Total==Before.Total && After.Convoy<=After.Total &&
           LivingAlly(Beat->Before,Transport) && LivingAlly(Beat->After,Transport) && LivingAlly(Displayed,Transport))
        {
            const auto& From=Beat->Before[Transport].Position;
            const auto& To=Displayed[Transport].Position;
            const float Travel=FVector::Dist2D(From,To);
            if(Travel>5.f && FVector::DistSquared2D(From,Beat->After[Transport].Position)>25.f)
            {
                auto& Cart=Read.Cart;
                const int Clearer=Beat->RoadClearedBy;
                Cart.Kind=Clearer>=0 && Clearer<3 && LivingAlly(Displayed,Clearer) ? ESignal::RoadOpened:ESignal::CartAdvance;
                Cart.Actor=Cart.Kind==ESignal::RoadOpened ? Clearer:-1;
                Cart.Origin=From;Cart.Position=To;Cart.Total=After.Total;
                if(FVector::DistSquared2D(To,Beat->After[Transport].Position)<=4.f) Cart.Count=After.Convoy;
                Cart.Weight=FMath::SmoothStep(5.f,30.f,Travel);
                Cart.Phase=Progress;
            }
        }

        if(Progress<.84f || Before.Held<0 || After.Held<0 || After.Required<=0 ||
           Before.PrimaryGoal!=After.PrimaryGoal || Before.SecondaryGoal!=After.SecondaryGoal ||
           After.PrimaryGoal==After.SecondaryGoal || After.DefenseChecks.Num()!=2) return Read;
        const int Goals[]={After.PrimaryGoal,After.SecondaryGoal};
        for(int Wing=0;Wing<2;++Wing)
        {
            const auto& Check=After.DefenseChecks[Wing];
            if(Check.Goal!=Goals[Wing] || Check.Turn!=Before.CommandTurn ||
               !frontline::StrategistJourney::OnMap(Check.Goal)) continue;
            const auto* Previous=Before.DefenseChecks.IsValidIndex(Wing) ? &Before.DefenseChecks[Wing]:nullptr;
            const bool HasPrevious=Previous && Previous->Goal==Check.Goal && Previous->Turn==Before.CommandTurn-1;
            // The save keeps the combined streak. A positive streak proves that both wings held.
            const bool WasHeld=HasPrevious ? Previous->bSecured:Before.Held>0;
            const bool NewGuard=HasPrevious && Previous->Guard!=Check.Guard;
            if(Check.bSecured && (!WasHeld || NewGuard) && Check.Guard>=0 && Check.Guard<3 &&
               LivingAlly(Displayed,Check.Guard) && TileAt(Displayed[Check.Guard].Position)==Check.Goal)
            {
                auto& Signal=Read.Wings[Wing];
                Signal.Kind=ESignal::WingHeld;Signal.Actor=Check.Guard;Signal.Tile=Check.Goal;
                Signal.Weight=FMath::SmoothStep(.84f,.95f,Progress);Signal.Phase=(Progress-.84f)*1.5f;
            }
            else if(!Check.bSecured && WasHeld)
            {
                auto& Signal=Read.Wings[Wing];
                Signal.Kind=ESignal::WingLost;Signal.Tile=Check.Goal;
                Signal.Weight=FMath::SmoothStep(.84f,.95f,Progress);Signal.Phase=(Progress-.84f)*1.5f;
            }
        }
        return Read;
    }

    FLinearColor SignalColor(ESignal Kind)
    {
        if(Kind==ESignal::WingLost) return FLinearColor(1.f,.35f,.15f);
        if(Kind==ESignal::WingHeld) return FLinearColor(1.f,.78f,.26f);
        return FLinearColor(.33f,1.f,.78f);
    }
}
