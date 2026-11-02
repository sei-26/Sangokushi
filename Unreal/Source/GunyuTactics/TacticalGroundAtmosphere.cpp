#include "TacticalGroundAtmosphere.h"
namespace gunyuatmosphere
{
    TArray<FCloud> GroundDust(const TArray<FVisualUnit>& Units,const FBattleBeat* Beat,float P)
    {
        TArray<FCloud> Out;
        if(!Beat || Beat->Kind==EBattleCue::Stop) return Out;
        const bool Combat=Beat->Kind==EBattleCue::Enemy || Beat->Kind==EBattleCue::Strike || Beat->Kind==EBattleCue::Ambush ||
            Beat->Kind==EBattleCue::Breakthrough || Beat->Kind==EBattleCue::Feint || Beat->Kind==EBattleCue::Disrupt ||
            Beat->Kind==EBattleCue::AzureDragon || Beat->Kind==EBattleCue::SerpentRoar;
        for(const auto& U:Units)
        {
            const int Id=U.Id;
            if(Id<0 || !Beat->Before.IsValidIndex(Id) || Beat->Before[Id].Id!=Id || Beat->Before[Id].Troops<=0 ||
               !Beat->After.IsValidIndex(Id) || Beat->After[Id].Id!=Id) continue;
            const FVector Travel=Beat->After[Id].Position-Beat->Before[Id].Position;
            if(U.Troops>0 && U.Motion>.01f && Travel.SizeSquared2D()>1)
            {
                const FVector F=Travel.GetSafeNormal2D(),S(-F.Y,F.X,0);
                for(int I=0;I<10;++I)
                {
                    const float Age=FMath::Frac(P*2.6f+I/10.f+Id*.137f);
                    const float Fade=FMath::Sin(Age*PI)*FMath::Clamp(U.Motion,0.f,1.f);
                    const FVector Position=U.Position-F*(25+Age*72)+S*((I%5-2)*22);
                    Out.Add({Position,FVector(.25f+Age*.40f,.21f+Age*.32f,.12f+Age*.19f)*Fade,8+Age*23});
                }
            }
            const bool ActualLoss=Combat && (Beat->Kind==EBattleCue::Enemy || Id==Beat->Target) && U.Loss>0 && Beat->After[Id].Troops<Beat->Before[Id].Troops;
            if(!ActualLoss || P<.48f || P>=.94f) continue;
            const float Age=FMath::Clamp((P-.48f)/.46f,0.f,1.f);
            const float Fade=FMath::SmoothStep(0.f,.12f,Age)*(1-FMath::SmoothStep(.50f,1.f,Age));
            for(int I=0;I<14;++I)
            {
                const float Angle=I*2.4f+Id*.73f;
                const FVector Position=U.Position+FVector(FMath::Cos(Angle),FMath::Sin(Angle),0)*(18+Age*(52+I%3*11));
                Out.Add({Position,FVector(.24f+Age*.57f,.24f+Age*.57f,.12f+Age*.27f)*Fade,7+FMath::Sin(Age*PI)*(12+I%4*6)});
            }
        }
        return Out;
    }
}
