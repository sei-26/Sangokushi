#include "TacticalTrial.h"
#include "TacticalSceneryMesh.h"
#include "Materials/MaterialInterface.h"

void ATacticalField::BuildMountainBackdrop()
{
    TacticalMap::FIndexedScenery Scene;
    auto Mountain=[&](double X,double Y,int Layer)
    {
        const FVector Peaks[]={FVector(550,-900,820),FVector(850,-1550,1120),FVector(3550,-900,970),FVector(4100,-1800,1320),FVector(2150,-2350,1270)};
        double H=0;
        for(const auto& P:Peaks)
        {
            const double DX=(X-P.X)/(Layer==0 ? 440.:850.),DY=(Y-P.Y+Layer*1700)/(Layer==0 ? 630.:1000.);
            H=FMath::Max(H,P.Z*FMath::Exp(-FMath::Pow(FMath::Abs(DX),.95)-FMath::Pow(FMath::Abs(DY),1.05)));
        }
        const double Begin=FMath::SmoothStep(180.,650.,-Y);
        // A real open valley frames the settlement; do not bury it in a ridge.
        const double Side=FMath::SmoothStep(1200.,1450.,FMath::Abs(X-2200));
        const double Behind=FMath::SmoothStep(1150.,1700.,-Y);
        const double Valley=FMath::Max(Side,Behind);
        const double Fold=(36+30*FMath::Sin(X*.011+Y*.004))*FMath::Pow(FMath::Max(0.,FMath::Sin(X*.008-Y*.006)*.5+.5),3.);
        return BackdropGroundHeight(FVector(X,Y,0))-35+(H+Fold*FMath::Clamp(H/200.,0.,1.))*(Layer==0 ? 1.:1.12)*Begin*(Layer==0 ? Valley:1.);
    };
    for(int Layer=0;Layer<2;++Layer)
    {
        const double Step=Layer==0 ? 95.:160.;
        const int Width=Layer==0 ? 72:48,Depth=Layer==0 ? 36:30;
        const double Front=Layer==0 ? 50:-2600;
        auto Point=[&](double X,double Y) { return FVector(X,Y,Mountain(X,Y,Layer)); };
        auto Color=[&](FVector P,FVector N)
        {
            const float Light=FMath::Clamp(.62f-N.X*.22f+N.Y*.12f+N.Z*.50f,.35f,1.25f);
            const float Air=FMath::Clamp(float((-P.Y-500)/6000),0.f,1.f);
            const float Height=FMath::Clamp(float((P.Z-BackdropGroundHeight(P)+35)/650),0.f,1.f);
            const auto Stone=FMath::Lerp(FLinearColor(.10f,.13f,.095f),FLinearColor(.23f,.30f,.32f),Layer==0 ? Air*.4f:.6f+Air*.4f);
            const auto Tint=Layer==0 ? FMath::Lerp(FLinearColor(.025f,.047f,.024f),Stone,FMath::SmoothStep(.06f,.68f,Height)) : Stone;
            return Tint*Light;
        };
        const int First=Scene.Vertices.Num();
        for(int Y=0;Y<=Depth;++Y) for(int X=0;X<=Width;++X)
        {
            const FVector P=Point(-1600+X*Step,Front-Y*Step);
            const FVector N=FVector(Mountain(P.X-3,P.Y,Layer)-Mountain(P.X+3,P.Y,Layer),Mountain(P.X,P.Y-3,Layer)-Mountain(P.X,P.Y+3,Layer),6).GetSafeNormal();
            Scene.Vertex(P,N,Color(P,N));
        }
        for(int Y=0;Y<Depth;++Y) for(int X=0;X<Width;++X)
        {
            const int A=First+Y*(Width+1)+X,B=A+1,D=A+Width+1,C=D+1;
            Scene.Triangle(A,B,C);Scene.Triangle(A,C,D);
        }
    }
    Scene.Submit(Ground,4);
    Ground->SetMaterial(4,DistantMaterial ? DistantMaterial.Get() : TerrainMaterial.Get());
}
