#include "TacticalTacticEffects.h"
#include "TacticalCinematicStyle.h"
void gunyufx::Tactic(const FBattleBeat& B,const TArray<FVisualUnit>& U,float P,FQuadEmitter Quad)
{
    if(!gunyucinema::IsTactic(B.Kind) || B.Kind==EBattleCue::Stop || !U.IsValidIndex(B.Actor) || U[B.Actor].Id<0) return;
    const FVector Actor=U[B.Actor].Position+FVector(0,0,38);
    auto Color=gunyucinema::Color(B.Kind);
    auto Line=[&](FVector A,FVector Z,float Width,FLinearColor Tint)
    {
        const FVector D=(Z-A).GetSafeNormal(),Side=FVector::CrossProduct(D,FVector(0,-1,.8)).GetSafeNormal()*Width;
        Quad(A-Side,A+Side,Z+Side*.15,Z-Side*.15,Tint);
    };
    if(P>.08f && P<.37f)
    {
        Color.A=FMath::Sin((P-.08f)/.29f*PI)*.65f;
        for(int I=0;I<12;++I)
        {
            const float A=I*2*PI/12+P*1.5f;
            const FVector Near=Actor+FVector(FMath::Cos(A),FMath::Sin(A),.25)*55;
            Line(Actor+FVector(FMath::Cos(A),FMath::Sin(A),.5)*(120-P*100),Near,2,Color);
        }
    }
    if(P<.48f || P>.92f) return;
    const float T=(P-.48f)/.44f;Color.A=1-T;
    if(B.Kind==EBattleCue::Rally)
    {
        for(int Id=0;Id<U.Num();++Id)
        {
            if(U[Id].Id<0 || !B.Before.IsValidIndex(Id) || !B.After.IsValidIndex(Id) || !gunyucinema::SupportChanged(B.Before[Id],B.After[Id])) continue;
            for(int I=0;I<8;++I)
            {
                const float A=I*PI/4;
                const FVector Root=U[Id].Position+FVector(FMath::Cos(A)*62,FMath::Sin(A)*62,8);
                Line(Root+FVector(0,0,T*105),Root+FVector(0,0,T*105+45),3.2f,Color);
            }
        }
        return;
    }
    if(!U.IsValidIndex(B.Target) || U[B.Target].Id<0) return;
    const FVector Target=U[B.Target].Position+FVector(0,0,35);
    const FVector Forward=(Target-Actor).GetSafeNormal2D(),Across(-Forward.Y,Forward.X,0);
    if(B.Kind==EBattleCue::AzureDragon || B.Kind==EBattleCue::SerpentRoar)
        gunyufx::Signature(B,U,P,Quad);
    else if(B.Kind==EBattleCue::Lure || B.Kind==EBattleCue::Feint)
    {
        // Retreat/feint leaves blue wakes on the acting unit, not sword blows on the enemy.
        for(int I=0;I<5;++I)
        {
            const FVector Offset=Across*((I-2)*18)+FVector(0,0,I%2*8);
            Line(Actor+Offset+Forward*(25+T*70),Actor+Offset-Forward*20,2.4f*(1-T),Color);
        }
    }
    else if(B.Kind==EBattleCue::Ambush)
    {
        for(int Direction=0;Direction<4;++Direction)
        {
            const float Angle=Direction*PI/2+.4f;
            const FVector Axis(FMath::Cos(Angle),FMath::Sin(Angle),0);
            for(int I=0;I<12;++I)
            {
                const float S=I/12.f,Z=(I+1)/12.f;
                auto Point=[&](float F) { return Target+Axis*(150*(1-F))+FVector(0,0,FMath::Sin(F*PI)*60+T*20); };
                Line(Point(S),Point(Z),2.8f,Color);
            }
        }
    }
    else
    {
        const int Count=B.Kind==EBattleCue::Breakthrough ? 11:5;
        for(int I=0;I<Count;++I)
        {
            const FVector Offset=Across*((I-(Count-1)*.5f)*13)+FVector(0,0,(I%3)*12);
            Line(Target+Offset-Forward*(75-T*35),Target+Offset+Forward*(50+T*150),B.Kind==EBattleCue::Breakthrough ? 4.f:2.5f,Color);
        }
    }
}
