#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;
void ATacticalField::AttachMissionCredit(FBattleBeat& B)
{
    const auto& A=B.MissionBefore;const auto& Z=B.MissionAfter;
    auto Clear=[&] { State->RoadClearArmy=State->RoadClearStep=State->RoadClearTile=-1; };
    auto Living=[](const TArray<FVisualUnit>& U,int Id) { return U.IsValidIndex(Id) && U[Id].Id==Id && !U[Id].bEnemy && U[Id].Troops>0; };
    if(!A.bBattle || !Z.bBattle) { Clear();return; }
    if(A.Convoy>=0 && StrategistJourney::OnMap(A.ConvoyNext))
    {
        if(State->RoadClearStep!=A.Convoy || State->RoadClearTile!=A.ConvoyNext) Clear();
        const auto Road=SurfaceLocation(A.ConvoyNext);bool Blocked=false;
        for(int Id=0;Id<3;++Id) Blocked|=Living(B.After,Id) && B.After[Id].Position.Equals(Road,1.f);
        if(Blocked) Clear();
        else for(int Id=0;Id<3;++Id)
        {
            const bool Directed=B.Kind==EBattleCue::Enemy ? (A.DirectedMovers&(1<<Id))!=0:
                B.Actor==Id && (B.Kind==EBattleCue::March || B.Kind==EBattleCue::Lure || B.Kind==EBattleCue::Breakthrough);
            if(Directed && Living(B.Before,Id) && Living(B.After,Id) && B.Before[Id].Position.Equals(Road,1.f) && !B.After[Id].Position.Equals(Road,1.f))
            { State->RoadClearArmy=Id;State->RoadClearStep=A.Convoy;State->RoadClearTile=A.ConvoyNext;break; }
        }
        if(B.Kind==EBattleCue::Enemy && Z.Convoy>A.Convoy)
        {
            const int Id=State->RoadClearArmy;
            if(Id>=0 && Living(B.Before,Id) && Living(B.After,Id) && Living(B.Before,5) && Living(B.After,5) && !B.Before[5].Position.Equals(B.After[5].Position)) B.RoadClearedBy=Id;
            Clear();
        }
    }
    else Clear();
    if(B.Kind!=EBattleCue::Enemy || A.Held<0 || Z.Held<=A.Held) return;
    const auto& J=State->Journey;
    for(int I=0;I<int(J.battleReview.relief.size());++I)
    {
        const auto& R=J.battleReview.relief[I];
        if(A.ConfirmedReliefs.Contains(I) || R.checkedTurn!=J.turn-1 || !R.allSecured || R.departedTurn<=0 || R.arrivedTurn<=0 ||
           (R.outcome!=JourneyReliefOutcome::Held && R.outcome!=JourneyReliefOutcome::Recovered) || !Living(B.Before,R.incoming) || !Living(B.After,R.incoming)) continue;
        B.ReliefIncoming=R.incoming;B.ReliefOutgoing=R.outgoing;B.bReliefRecovered=R.outcome==JourneyReliefOutcome::Recovered;break;
    }
}
