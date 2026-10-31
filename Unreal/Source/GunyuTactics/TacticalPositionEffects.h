#pragma once
#include "TacticalPresentation.h"
namespace gunyupositionfx
{
    template<class Surface,class Height,class Quad>
    void Draw(const FTacticPositionComparison& P,Surface Location,Height Floor,Quad Add)
    {
        if(!P.bVisible) return;
        for(const auto& C:P.Cells)
        {
            if(!C.bMoveAllowed) continue;
            const auto Center=Location(C.Tile);
            FLinearColor Color=C.bReady ? FLinearColor(.72f,.44f,1.f,.85f):FLinearColor(.32f,.57f,.65f,.5f);
            for(int Edge=0;Edge<6;++Edge)
            {
                const float A=(Edge*60.f+30.f)*PI/180.f,B=((Edge+1)*60.f+30.f)*PI/180.f;
                auto V=Center+FVector(FMath::Cos(A),FMath::Sin(A),0)*104;
                auto W=Center+FVector(FMath::Cos(B),FMath::Sin(B),0)*104;
                V.Z=Floor(V)+16;W.Z=Floor(W)+16;
                const auto Side=FVector::CrossProduct((W-V).GetSafeNormal(),FVector::UpVector)*(C.Tile==P.SelectedTile ? 4.f:2.f);
                Add(V-Side,V+Side,W+Side,W-Side,Color);
            }
        }
        const auto& R=P.Relief;
        if(R.bVisible && R.bCanIssue && R.IncomingTile>=0 && R.Source>=0)
        {
            const auto A=Location(R.IncomingTile),B=Location(R.Source);
            const auto Side=FVector::CrossProduct((B-A).GetSafeNormal(),FVector::UpVector)*3.f;
            for(int I=0;I<7;++I)
            {
                auto V=FMath::Lerp(A,B,I/7.f),W=FMath::Lerp(A,B,(I+.55f)/7.f);
                V.Z=Floor(V)+21;W.Z=Floor(W)+21;
                Add(V-Side,V+Side,W+Side,W-Side,FLinearColor(.25f,.94f,.62f,.9f));
            }
        }
    }
}
