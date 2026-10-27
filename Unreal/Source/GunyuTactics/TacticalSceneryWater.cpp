#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalMapMesh.h"
#include "TacticalSceneryLayout.h"

void ATacticalField::BuildSceneryWater()
{
    using namespace frontline;
    const auto& J=State->Journey;
    const int Mouth=Campaign::At(11,1);
    if(J.mission!=1 || J.world.tiles[Mouth].terrain!=Terrain::River)
    { Ground->ClearMeshSection(7);return; }
    TacticalMap::FSurface Water;
    const FVector First=TileLocation(Mouth);
    auto Center=[&](double T)
    { const double Y=FMath::Lerp(First.Y,-1600.,T);return FVector(TacticalMap::PortRiverX(Y),Y,-9); };
    auto Edge=[&](double T,int Side)
    {
        const FVector P=Center(T),D=(Center(T+.001)-Center(T-.001)).GetSafeNormal2D();
        const double ChannelRadius=46+3*FMath::Sin(P.Y*.008+P.X*.006)+2*FMath::Sin(P.X*.021-P.Y*.003);
        return P+FVector(-D.Y,D.X,0)*Side*ChannelRadius;
    };
    auto Shade=[&](double T)
    { return FMath::Lerp(J.intelligence[0].visible[Mouth] ? 1.f:.4f,.4f,float(FMath::SmoothStep(0.,.16,T))); };
    constexpr int Steps=80;
    for(int I=0;I<Steps;++I)
    {
        const double T=I/double(Steps),U=(I+1)/double(Steps);
        const FVector A=Edge(T,-1),B=Edge(T,1),C=Edge(U,1),D=Edge(U,-1),P=Center(T),Q=Center(U);
        const auto Deep=FLinearColor(.028f,.095f,.10f)*Shade(T),EndDeep=FLinearColor(.028f,.095f,.10f)*Shade(U);
        const auto Shallow=FLinearColor(.10f,.205f,.17f)*Shade(T),EndShallow=FLinearColor(.10f,.205f,.17f)*Shade(U);
        Water.Triangle(A,P,Q,Shallow,Deep,EndDeep);Water.Triangle(A,Q,D,Shallow,EndDeep,EndShallow);
        Water.Triangle(P,B,C,Deep,Shallow,EndShallow);Water.Triangle(P,C,Q,Deep,EndShallow,EndDeep);
    }
    Water.Submit(Ground,7);
    Ground->SetMaterial(7,WaterMaterial ? WaterMaterial.Get():Ground->GetMaterial(2));
}
