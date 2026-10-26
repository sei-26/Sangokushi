#include "TacticalTacticEffects.h"
// Broad translucent ribbons and bright narrow cores, drawn at real participants.
void gunyufx::Signature(const FBattleBeat& B,const TArray<FVisualUnit>& U,float P,FQuadEmitter Quad)
{
    if(P<.48f || P>=.92f || !U.IsValidIndex(B.Actor) || !U.IsValidIndex(B.Target) || U[B.Actor].Id<0 || U[B.Target].Id<0) return;
    if(!B.Before.IsValidIndex(B.Target) || B.Before[B.Target].Id<0) return;
    const float T=(P-.48f)/.44f,Fade=FMath::Pow(1-T,1.3f);
    const FVector A=U[B.Actor].Position+FVector(0,0,28),C=U[B.Target].Position+FVector(0,0,35);
    const FVector Forward=(C-A).GetSafeNormal2D(),Across(-Forward.Y,Forward.X,0);
    auto Ray=[&](FVector From,FVector To,float Width,FLinearColor Color)
    { const FVector Side=FVector::CrossProduct((To-From).GetSafeNormal(),FVector(0,-1,.8)).GetSafeNormal()*Width;Quad(From-Side,From+Side,To+Side*.12,To-Side*.12,Color); };
    if(B.Kind==EBattleCue::AzureDragon)
    {
        for(int Layer=0;Layer<3;++Layer) for(int I=0;I<32;++I)
        {
            const float U0=I/32.f,U1=(I+1)/32.f;
            const float Angle=.10f+U0*PI*.93f-T*.40f,Next=.10f+U1*PI*.93f-T*.40f;
            const float Radius=125+T*70+Layer*7,Thickness=FMath::Pow(FMath::Sin((U0+.015f)*PI),.65f)*(Layer==0 ? 36.f:Layer==1 ? 18.f:5.f)*Fade;
            const FVector V=Across*FMath::Cos(Angle)+FVector::UpVector*FMath::Sin(Angle)*.9f,W=Across*FMath::Cos(Next)+FVector::UpVector*FMath::Sin(Next)*.9f;
            const FVector Origin=C+Forward*(Layer*3+T*28)+FVector(0,0,20-T*12);
            const FLinearColor Color=Layer==0 ? FLinearColor(.06f,.72f,.34f,.46f*Fade):Layer==1 ? FLinearColor(.20f,1,.67f,.75f*Fade):FLinearColor(.87f,1,.94f,.95f*Fade);
            Quad(Origin+V*Radius,Origin+V*(Radius+Thickness),Origin+W*(Radius+Thickness),Origin+W*Radius,Color);
        }
        for(int I=0;I<16;++I)
        {
            const float F=I/15.f;
            const FVector Root=C+Across*((F-.5f)*180)+FVector(0,0,20+(I%4)*14);
            Ray(Root-Forward*(20+T*45),Root+Forward*(45+T*100)+FVector(0,0,28*FMath::Sin(F*PI)),2.6f*Fade,FLinearColor(.65f,1,.81f,.8f*Fade));
        }
    }
    else if(B.Kind==EBattleCue::SerpentRoar)
    {
        // Pressure fronts follow the displayed roar toward its observed target.
        for(int Wave=0;Wave<3;++Wave) for(int I=0;I<28;++I)
        {
            const float Angle=-PI*.42f+I*PI*.84f/28,Next=Angle+PI*.84f/28;
            const float Radius=65+T*190+Wave*20,Width=(Wave==0 ? 15.f:7.f)*Fade;
            const FVector V=Forward*FMath::Cos(Angle)+Across*FMath::Sin(Angle),W=Forward*FMath::Cos(Next)+Across*FMath::Sin(Next);
            const FVector Origin=A+FVector(0,0,Wave*16+T*20);
            const FLinearColor Color=Wave==0 ? FLinearColor(1,.77f,.37f,.85f*Fade):FLinearColor(1,.23f,.055f,.55f*Fade);
            Quad(Origin+V*Radius,Origin+V*(Radius+Width),Origin+W*(Radius+Width),Origin+W*Radius,Color);
        }
        for(int I=0;I<18;++I)
        {
            const FVector Root=C+Across*((I-8.5f)*12)+FVector(0,0,8+I%3*12);
            Ray(Root-Forward*25,Root+Forward*(48+T*95)+FVector(0,0,14+T*35),3.6f*Fade,FLinearColor(1,.54f,.12f,.75f*Fade));
        }
    }
}
