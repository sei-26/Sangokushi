#include "TacticalInfantry.h"
namespace gunyuinfantry
{
    void Soldier(FBatches& B,const gunyumotion::FPose& P,FVector Base,FQuat Heading,float Scale,float Height,const gunyucompany::FEquipment& E)
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
        // Sleeves and split cloth skirt remain visible around the curved armour.
        for(int Side:{-1,1})
        {
            Part(B.Cloth,Upper(FVector(0,Side*17,44)),FVector(.19,.12,.16),P.Torso);
            Part(B.Cloth,FVector(0,Side*9,20),FVector(.24,.16,.18));
        }
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
        if(E.bShield)
        {
            Part(B.Shields,P.ShieldCenter,FVector(.85,.32,.43),P.Shield);
            Part(B.Cloth,P.ShieldCenter+P.Shield.RotateVector(FVector(3,-.5,0)),FVector(.018,.10,.33),P.Shield);
            Part(B.Iron,P.ShieldCenter+P.Shield.RotateVector(FVector(5,0,0)),FVector(.07,.13,.13),P.Shield);
        }
        // Crouching compresses world Z, not the spear's own long axis.
        const FVector OriginalAxis=P.Spear.RotateVector(FVector::UpVector);
        FVector Shaft=OriginalAxis;Shaft.Z*=Height;
        const float ShaftScale=Shaft.Size();
        const FQuat Weapon=FQuat::FindBetweenNormals(OriginalAxis,Shaft.GetSafeNormal())*P.Spear;
        if(E.Weapon==gunyucompany::EWeapon::Sword)
        {
            // OBJ grip is at Z=-35; attach the actual grip to the right hand.
            B.Swords.Add(FTransform(Heading*Weapon,Base+Heading.RotateVector(Local(P.Hand[1]+P.Spear.RotateVector(FVector(0,0,35*.62f)))),FVector(.32,.32,.62*ShaftScale)*Scale));
        }
        else
        {
            B.Poles.Add(FTransform(Heading*Weapon,Base+Heading.RotateVector(Local(P.SpearCenter)),FVector(.025,.025,E.ShaftLength/100*ShaftScale)*Scale));
            const float BladeLength=E.Weapon==gunyucompany::EWeapon::Spear ? .20f:.34f;
            auto& Blades=E.Weapon==gunyucompany::EWeapon::Glaive ? B.Glaives:E.Weapon==gunyucompany::EWeapon::Serpent ? B.Serpents:B.Spears;
            Blades.Add(FTransform(Heading*Weapon,Base+Heading.RotateVector(Local(P.SpearCenter+P.Spear.RotateVector(FVector(0,0,E.ShaftLength*.5f+BladeLength*50)))),FVector(.30,.30,BladeLength*ShaftScale)*Scale));
            if(E.bCommander)
                for(int I=0;I<3;++I)
                    Part(B.Cloth,P.SpearCenter+P.Spear.RotateVector(FVector(4+I*2,0,E.ShaftLength*.5f-5-I*6)),FVector(.045,.035,.08),P.Spear);
        }
    }
}
