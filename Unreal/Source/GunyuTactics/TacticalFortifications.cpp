#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalMapMesh.h"
#include "Components/InstancedStaticMeshComponent.h"

// Site markers use only public cities and mission goals. They never inspect
// enemy armies or create an additional movement/collision rule.
void ATacticalField::BuildFortifications()
{
    TacticalMap::FSurface Cloth;
    auto Wood=[&](FVector P,FVector Size,FRotator R=FRotator::ZeroRotator)
    { Buildings->AddInstance(FTransform(R,P,Size/100)); };
    auto Flag=[&](FVector Pole,FLinearColor Color,float Scale)
    {
        Pole.Z=SurfaceHeight(Pole);
        Wood(Pole+FVector(0,0,75)*Scale,FVector(3,3,150)*Scale);
        auto Point=[&](float U,float V)
        { return Pole+FVector(U*54,4*FMath::Sin(U*PI),143-V*36)*Scale; };
        for (int Y=0;Y<6;++Y) for (int X=0;X<10;++X)
        {
            const FVector A=Point(X/10.f,Y/6.f),B=Point((X+1)/10.f,Y/6.f);
            const FVector C=Point((X+1)/10.f,(Y+1)/6.f),D=Point(X/10.f,(Y+1)/6.f);
            const bool Trim=X==0 || X==9 || Y==0 || Y==5;
            const bool Emblem=(X==4 || X==5) && Y>=2 && Y<=3;
            const FLinearColor Tint=Trim || Emblem ? FLinearColor(.78f,.52f,.13f) : Color;
            Cloth.Triangle(A,B,C,Tint,Tint,Tint);Cloth.Triangle(A,C,D,Tint,Tint,Tint);
        }
    };
    const auto& J=State->Journey;
    for (const auto& City : J.world.cities)
    {
        if (!frontline::StrategistJourney::OnMap(City.tile)) continue;
        const FVector Center=SurfaceLocation(City.tile);
        // All pieces fit within the existing impassable city cell.
        FVector Tent=Center+FVector(-18,17,0);Tent.Z=SurfaceHeight(Tent);
        Tents->AddInstance(FTransform(FRotator(0,12,0),Tent,FVector(.92)));
        const FVector Tower=Center+FVector(-43,-44,0);
        Watchtowers->AddInstance(FTransform(FRotator::ZeroRotator,FVector(Tower.X,Tower.Y,SurfaceHeight(Tower)),FVector(.56)));
        Roofs->AddInstance(FTransform(FRotator::ZeroRotator,FVector(Tower.X,Tower.Y,SurfaceHeight(Tower)+66),FVector(.36,.37,.22)));
        for (int Post=0;Post<30;++Post)
        {
            const double Angle=Post*2*PI/30;
            if (FMath::Abs(FMath::UnwindRadians(Angle))<.4) continue;
            FVector P=Center+FVector(FMath::Cos(Angle)*88,FMath::Sin(Angle)*88,0);
            P.Z=SurfaceHeight(P);
            Palisades->AddInstance(FTransform(FRotator(0,Post*13,0),P,FVector(.8,.8,.72+(Post%3)*.04)));
        }
        for (int Crate=0;Crate<3;++Crate)
        {
            FVector P=Center+FVector(35+(Crate%2)*23,-34+(Crate/2)*24,0);P.Z=SurfaceHeight(P);
            SupplyCrates->AddInstance(FTransform(FRotator(0,Crate*11,0),P,FVector(.47)));
        }
        Flag(Center+FVector(23,48,0),City.owner==0 ? FLinearColor(.025f,.27f,.30f) : FLinearColor(.40f,.04f,.025f),.95f);
    }
    auto Gate=[&](int Tile)
    {
        if (Tile<0) return;
        const FVector P=SurfaceLocation(Tile);
        // A 176-wide, 246-high opening clears the formation and its standard.
        // Stone feet and carved-looking braces give the landmark a silhouette
        // without filling the destination with a solid proxy building.
        for (int Side : {-1,1})
        {
            Stonework->AddInstance(FTransform(FRotator::ZeroRotator,P+FVector(0,Side*96,13),FVector(.38,.34,.26)));
            Wood(P+FVector(0,Side*96,140),FVector(16,16,228));
            Wood(P+FVector(0,Side*110,230),FVector(12,42,8),FRotator(0,0,Side*30));
            Wood(P+FVector(0,Side*96,258),FVector(27,25,13));
        }
        Wood(P+FVector(0,0,254),FVector(22,226,16));
        Wood(P+FVector(0,0,268),FVector(17,244,9));
        Roofs->AddInstance(FTransform(FRotator::ZeroRotator,P+FVector(0,0,275),FVector(.92,2.58,.48)));
        Flag(P+FVector(10,127,0),FLinearColor(.48f,.24f,.035f),.95f);
    };
    Gate(J.goal);Gate(J.Definition().secondGoal);
    Cloth.Submit(FieldBanners,0);
}
