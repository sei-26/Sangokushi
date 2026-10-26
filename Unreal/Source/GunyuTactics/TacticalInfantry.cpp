#include "TacticalInfantry.h"
namespace gunyuinfantry
{
    void Soldier(FBatches& B,const gunyumotion::FPose& P,FVector Base,FQuat Heading,float Scale,float Height)
    {
        auto Local=[&](FVector V) { V+=P.Root;V.Z*=Height;return V*Scale; };
        auto Part=[&](TArray<FTransform>& Out,FVector V,FVector Size,FQuat Rotation=FQuat::Identity)
        {
            Size.Z*=Height;Out.Add(FTransform(Heading*Rotation,Base+Heading.RotateVector(Local(V)),Size*Scale));
        };
        auto Upper=[&](FVector V) { return FVector(0,0,27)+P.Torso.RotateVector(V-FVector(0,0,27)); };
        auto Link=[&](FVector A,FVector Z,float Radius)
        {
            A=Local(A);Z=Local(Z);const FVector D=Z-A;
            const FQuat Q=FQuat::FindBetweenNormals(FVector::UpVector,D.GetSafeNormal());
            B.Limbs.Add(FTransform(Heading*Q,Base+Heading.RotateVector((A+Z)*.5f),FVector(Radius*Scale,Radius*Scale,D.Size()/100)));
        };
        Part(B.Cloth,Upper(FVector(0,0,37)),FVector(.25,.32,.33),P.Torso);
        Part(B.Armour,Upper(FVector(0,0,38)),FVector(.46,.32,.38),P.Torso);
        Part(B.Iron,FVector(0,0,24),FVector(.28,.34,.13));
        Part(B.Cloth,Upper(FVector(0,0,31)),FVector(.29,.355,.055),P.Torso);
        Part(B.Skin,Upper(FVector(0,0,60)),FVector(.16,.16,.20),P.Torso);
        Part(B.Helmets,Upper(FVector(0,0,68)),FVector(.22,.22,.22),P.Torso);
        Part(B.Cloth,Upper(FVector(-3,0,79)),FVector(.06,.07,.14),P.Torso);
        for(int I=0;I<2;++I)
        {
            Link(P.Hip[I],P.Knee[I],.105f);Link(P.Knee[I],P.Foot[I],.09f);
            Link(P.Shoulder[I],P.Elbow[I],.105f);Link(P.Elbow[I],P.Hand[I],.085f);
            Part(B.Iron,P.Foot[I]+FVector(2,0,0),FVector(.14,.11,.07));
            Part(B.Skin,P.Hand[I],FVector(.075,.075,.075));
        }
        Part(B.Shields,P.ShieldCenter,FVector(.85,.32,.43),P.Shield);
        Part(B.Cloth,P.ShieldCenter+P.Shield.RotateVector(FVector(3,-.5,0)),FVector(.035,.22,.31),P.Shield);
        // Crouching compresses world Z, not the spear's own long axis.
        FVector Shaft=P.Spear.RotateVector(FVector::UpVector);Shaft.Z*=Height;
        const float ShaftScale=Shaft.Size();const FQuat Weapon=FQuat::FindBetweenNormals(FVector::UpVector,Shaft.GetSafeNormal());
        B.Poles.Add(FTransform(Heading*Weapon,Base+Heading.RotateVector(Local(P.SpearCenter)),FVector(.025,.025,.92*ShaftScale)*Scale));
        B.Spears.Add(FTransform(Heading*Weapon,Base+Heading.RotateVector(Local(P.SpearCenter+P.Spear.RotateVector(FVector(0,0,54)))),FVector(.18,.18,.20*ShaftScale)*Scale));
    }
}
