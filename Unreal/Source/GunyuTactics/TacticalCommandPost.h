#pragma once
#include "TacticalMapMesh.h"
namespace TacticalMap
{
    // A public objective, with no wall across the formation center. Nothing in
    // this vignette reads armies, ownership changes or private enemy orders.
    template<class Height,class Wood>
    void CommandPost(FVector Center,Height Floor,Wood Beam,FSurface& Cloth)
    {
        auto At=[&](FVector V) { auto P=Center+FVector(V.X,V.Y,0);P.Z=Floor(P)+V.Z;return P; };
        const FLinearColor Linen(.48f,.34f,.16f),Gold(.78f,.51f,.14f),Ink(.12f,.17f,.16f);
        for(float X:{80.f,98.f}) for(int Side:{-1,1})
            Beam(At(FVector(X,Side*56,65)),FVector(3,3,130),FRotator::ZeroRotator);
        auto Roof=[&](float U,float V)
        { return At(FVector(58+U*47,-57+V*114,116+18*FMath::Sin(V*PI)+U*7)); };
        for(int X=0;X<6;++X) for(int Y=0;Y<16;++Y)
        {
            const auto A=Roof(X/6.f,Y/16.f),B=Roof((X+1)/6.f,Y/16.f),C=Roof((X+1)/6.f,(Y+1)/16.f),D=Roof(X/6.f,(Y+1)/16.f);
            const auto Color=Y==0 || Y==15 || X==0 ? Gold:Y%4==0 ? Linen*.68f:Linen;
            Cloth.Triangle(A,B,C,Color,Color,Color);Cloth.Triangle(A,C,D,Color,Color,Color);
        }
        // A small map table sits behind the front ranks, under the awning.
        for(int X:{-1,1}) for(int Y:{-1,1})
            Beam(At(FVector(87+X*8,Y*21,14)),FVector(3,3,28),FRotator::ZeroRotator);
        Beam(At(FVector(87,0,29)),FVector(23,52,4),FRotator::ZeroRotator);
        const FVector A=At(FVector(77,-21,31.5)),B=At(FVector(97,-21,31.5)),C=At(FVector(97,21,31.5)),D=At(FVector(77,21,31.5));
        Cloth.Triangle(A,B,C,Linen*1.4f,Linen*1.4f,Linen*1.4f);Cloth.Triangle(A,C,D,Linen*1.4f,Linen*1.4f,Linen*1.4f);
        for(int I=0;I<3;++I) Cloth.Line(At(FVector(80,-14+I*12,32)),At(FVector(94,-8+I*10,32)),.8f,Ink);
        // A real round war drum: leather ends, wooden shell and bronze hoops.
        const FVector Drum=At(FVector(69,-69,39));
        for(int Side:{-1,1})
            Beam(At(FVector(69, -69+Side*10,16)),FVector(5,5,32),FRotator(0,0,Side*12));
        constexpr int Sides=32;
        for(int I=0;I<Sides;++I)
        {
            const float T=I*2*PI/Sides,U=(I+1)*2*PI/Sides;
            auto Point=[&](float X,float Angle,float Radius) { return Drum+FVector(X,FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius); };
            const auto P=Point(-9,T,16),Q=Point(9,T,16),R=Point(9,U,16),S=Point(-9,U,16);
            Cloth.Triangle(P,Q,R,Ink,Ink,Ink);Cloth.Triangle(P,R,S,Ink,Ink,Ink);
            for(int Side:{-1,1})
            {
                const auto V=Point(Side*9.3f,T,14),W=Point(Side*9.3f,U,14),End=Drum+FVector(Side*9.3f,0,0);
                Cloth.Triangle(End,V,W,Linen*1.3f,Linen*1.3f,Linen*1.3f);
                Cloth.Triangle(V,Point(Side*9.4f,T,16.5f),Point(Side*9.4f,U,16.5f),Gold,Gold,Gold);
                Cloth.Triangle(V,Point(Side*9.4f,U,16.5f),W,Gold,Gold,Gold);
            }
        }
    }
}
