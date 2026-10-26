#include "TacticalSpectacle.h"
#include "TacticalCinematicStyle.h"
namespace gunyuspectacle
{
    namespace
    {
        bool Present(const TArray<FVisualUnit>& U,int I) { return I>=0 && U.IsValidIndex(I) && U[I].Id==I; }
    }
    FShot Read(const FBattleBeat* B,const TArray<FVisualUnit>& U,float P)
    {
        FShot R;if(!B || !gunyucinema::IsTactic(B->Kind) || !Present(U,B->Actor)) return R;
        const bool Support=B->Kind==EBattleCue::Rally,Stopped=B->Kind==EBattleCue::Stop;
        if(!Support && !Stopped && (!Present(U,B->Target) || !Present(B->Before,B->Target))) return R;
        R.bValid=true;R.Center=U[B->Actor].Position;
        R.Weight=FMath::SmoothStep(.06f,.28f,P)*(1-FMath::SmoothStep(.82f,.99f,P));
        float Strength=0,Amplitude=3;
        if(Support)
        {
            FVector Total=FVector::ZeroVector;int Count=0;
            for(int I=0;I<U.Num();++I)
                if(U[I].Id>=0 && !U[I].bEnemy && I<3 && U[I].Troops>0)
                {
                    Total+=U[I].Position;++Count;
                    if(P>=.48f && Present(B->Before,I) && Present(B->After,I) && gunyucinema::SupportChanged(B->Before[I],B->After[I])) Strength=1;
                }
            if(Count>0) R.Center=Total/Count;
            R.CloseDistance=2250;R.Tilt=52;R.Bloom=.48f;Amplitude=0;
        }
        else if(!Stopped)
        {
            const float Bias=B->Kind==EBattleCue::Lure ? .30f:.55f;
            R.Center=FMath::Lerp(R.Center,U[B->Target].Position,Bias);
            if(P>=.48f && Present(B->Before,B->Target) && Present(B->After,B->Target))
            {
                const auto& A=B->Before[B->Target];const auto& Z=B->After[B->Target];
                Strength=Z.Troops<A.Troops || Z.Morale<A.Morale || Z.bDisordered!=A.bDisordered || Z.bPressured!=A.bPressured || Z.bGuarded!=A.bGuarded ? 1.f:0.f;
            }
            R.Bloom=.55f;
            if(B->Kind==EBattleCue::AzureDragon) { R.CloseDistance=1500;R.Tilt=36;R.Sweep=-42;R.Bloom=.9f;Amplitude=6; }
            if(B->Kind==EBattleCue::SerpentRoar || B->Kind==EBattleCue::Breakthrough) { R.CloseDistance=1800;R.Tilt=46;R.Sweep=26;R.Bloom=.7f;Amplitude=8; }
            if(B->Kind==EBattleCue::Ambush) { R.CloseDistance=1900;R.Tilt=43;R.Sweep=-18; }
            if(B->Kind==EBattleCue::Lure || B->Kind==EBattleCue::Feint) { R.CloseDistance=2000;R.Tilt=44;R.Bloom=.2f;Amplitude=1; }
        }
        // A single brief accent, tied to the current step's published result.
        const float T=FMath::Clamp((P-.48f)/.19f,0.f,1.f);
        R.Pulse=Strength*FMath::Sin(T*PI)*FMath::Pow(1-T,.6f);
        R.Shake=Amplitude*R.Pulse*FMath::Sin(T*PI*5);
        return R;
    }
}
