#include "TacticalActivity.h"
namespace gunyuactivity
{
    TArray<FMark> Markers(const TArray<FVisualUnit>& U,const FBattleBeat* B,float P)
    {
        TArray<FMark> Out;if(!B || B->Kind==EBattleCue::Stop) return Out;
        for(int I=0;I<U.Num();++I)
        {
            const auto R=Read(I,U,B,P);if(!R.bActive || U[I].Id<0 || U[I].Troops<=0 || (!R.bActor && U[I].Motion<=.01f && B->Kind!=EBattleCue::Enemy)) continue;
            const FVector F=R.Direction.GetSafeNormal2D(),S(-F.Y,F.X,0),C=U[I].Position;
            FLinearColor Color=R.Color;Color.A=.85f;
            const FVector Tip=C+F*112;
            Out.Add({{Tip-F*19-S*13,Tip,Tip-F*19+S*13},Color});
            if(!R.bActor || R.Partner<0 || P>.84f) continue;
            const FVector Target=U[R.Partner].Position;
            const float Length=FVector::Dist2D(C,Target);
            if(Length>190 && Length<650)
                for(int D=0;D<3;++D)
                { const float T=.38f+D*.08f;Out.Add({{FMath::Lerp(C,Target,T),FMath::Lerp(C,Target,T+.045f)},Color}); }
            Color=P>=.48f && U[R.Partner].Loss>0 ? FLinearColor(1,.39f,.19f,.9f):FLinearColor(1,.82f,.34f,.8f);
            const float Radius=93+FMath::Sin(FMath::Clamp((P-.48f)/.35f,0.f,1.f)*PI)*7;
            for(int Side:{-1,1})
                Out.Add({{Target+FVector(Side*Radius,-26,0),Target+FVector(Side*Radius,26,0)},Color});
        }
        return Out;
    }
}
