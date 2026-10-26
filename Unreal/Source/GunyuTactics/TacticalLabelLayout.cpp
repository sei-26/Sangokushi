#include "TacticalTrial.h"
#include "TacticalChrome.h"
#include "Campaign.hpp"

void ATacticalController::LayoutMapLabels()
{
    LabelLayoutFrame=GFrameCounter;UnitLabelPositions.Init(FVector2D(-2000,-2000),6);UnitLabelLeaders.Reset();
    if(!Field || Field->Presentation().Journey.bVisible) return;
    const auto& Units=Field->VisualUnits();const FVector2D Size(132,86);
    const FVector2D Min=gunyulayout::MapOrigin,Max=Min+gunyulayout::MapSize;
    TArray<FSlateRect> Occupied;
    // Leave the displayed formations and cart themselves readable, too.
    for(const auto& U:Units)
        if(U.Id>=0 && (U.Troops>0 || Field->IsPlaying()))
        {
            const auto P=MapLabelPosition(U.Position+FVector(0,0,35),FVector2D(100,66));
            Occupied.Add(FSlateRect(P.X,P.Y,P.X+100,P.Y+66));
        }
    Occupied.Add(FSlateRect(16,Field->bRealtimeCommand ? 486:586,246,704));
    if(Field->IsPlaying())
    {
        const auto C=Field->ReadTacticCinema();
        if(bTacticCinema && C.bVisible)
        {
            if(C.Relay>.001f) Occupied.Add(FSlateRect(860,168,1228,224));
            if(C.Intro>.001f) { Occupied.Add(FSlateRect(20,180,340,660));Occupied.Add(FSlateRect(300,220,760,400)); }
            if(C.Impact>.001f && !C.Primary.IsEmpty()) { const auto P=TacticResultPosition();Occupied.Add(FSlateRect(P.X,P.Y,P.X+380,P.Y+142)); }
        }
        else Occupied.Add(FSlateRect(290,106,1050,210));
    }
    if(bEnemyDetailsOpen && !Field->IsPlaying()) Occupied.Add(FSlateRect(16,174,716,306));
    for(int Tile:{Field->Presentation().GoalTile,Field->Presentation().SecondGoalTile})
        if(Tile>=0)
        { const auto P=MapLabelPosition(Field->SurfaceLocation(Tile)+FVector(0,0,240),FVector2D(140,34));Occupied.Add(FSlateRect(P.X,P.Y,P.X+140,P.Y+34)); }
    TArray<int> Priority{Field->Presentation().Selected,5};
    for(int Id=0;Id<6;++Id) Priority.AddUnique(Id);
    auto Intersects=[](const FSlateRect& A,const FSlateRect& B)
    { return A.Left<B.Right && B.Left<A.Right && A.Top<B.Bottom && B.Top<A.Bottom; };
    for(int Id:Priority)
    {
        if(!Units.IsValidIndex(Id) || Units[Id].Id<0 || (Units[Id].Troops<=0 && !Field->IsPlaying())) continue;
        const auto& U=Units[Id];const auto Anchor=MapLabelPosition(U.Position+FVector(0,0,70),FVector2D::ZeroVector);
        if(Anchor.X<Min.X-60 || Anchor.X>Max.X+60 || Anchor.Y<Min.Y-60 || Anchor.Y>Max.Y+60) continue;
        const auto Base=MapLabelPosition(U.Position+FVector(0,0,240),Size);
        TArray<double> Xs{FMath::Clamp(Base.X,Min.X,Max.X-Size.X),Min.X,Max.X-Size.X};
        TArray<double> Ys{FMath::Clamp(Base.Y,Min.Y,Max.Y-Size.Y),Min.Y,Max.Y-Size.Y};
        // Rectangle edges also find narrow free gaps; fixed offsets can miss
        // such a gap and unnecessarily hide a visible officer's plate.
        for(const auto& R:Occupied)
        {
            Xs.AddUnique(FMath::Clamp(double(R.Left)-Size.X-4,Min.X,Max.X-Size.X));
            Xs.AddUnique(FMath::Clamp(double(R.Right)+4,Min.X,Max.X-Size.X));
            Ys.AddUnique(FMath::Clamp(double(R.Top)-Size.Y-4,Min.Y,Max.Y-Size.Y));
            Ys.AddUnique(FMath::Clamp(double(R.Bottom)+4,Min.Y,Max.Y-Size.Y));
        }
        double Best=1e30;FVector2D Chosen(-2000,-2000);
        for(double X:Xs) for(double Y:Ys)
        {
            const FVector2D P(X,Y);const double Cost=(P-Base).SizeSquared();if(Cost>=Best) continue;
            const FSlateRect Bounds(X-3,Y-3,X+Size.X+3,Y+Size.Y+3);
            bool Free=true;for(const auto& Other:Occupied) if(Intersects(Bounds,Other)) { Free=false;break; }
            if(Free) { Best=Cost;Chosen=P; }
        }
        if(Best<1e29)
        {
            UnitLabelPositions[Id]=Chosen;Occupied.Add(FSlateRect(Chosen.X-3,Chosen.Y-3,Chosen.X+Size.X+3,Chosen.Y+Size.Y+3));
            if(!Chosen.Equals(Base,3)) UnitLabelLeaders.Add({Anchor,Chosen+FVector2D(Size.X*.5,U.Loss>0 ? 82:56)});
        }
    }
}
FString ATacticalController::UnitMapStatus(int Index) const
{
    const auto& Units=Field->VisualUnits();if(!Units.IsValidIndex(Index) || Units[Index].Id<0) return FString();
    const auto& U=Units[Index];
    if(Field->IsPlaying()) return Field->ReadUnitActivity(Index).Caption;
    if(U.Troops<=0) return TEXT("戦闘不能");
    if(U.bDisordered) return TEXT("混乱");
    if(U.bPressured) return TEXT("隙あり");
    if(U.Motion>.01f) return TEXT("行軍中");
    if(U.bConcealed) return TEXT("潜伏中");
    if(U.bGuarded) return TEXT("固守");
    if(U.bInspired) return TEXT("鼓舞");
    const auto& O=Field->Presentation().Officers;
    if(Index<3 && O.IsValidIndex(Index)) return O[Index].Command;
    return Index==5 ? TEXT("護送車") : FString();
}

void ATacticalController::SelectMapUnit(int Index)
{
    const auto& Units=Field->VisualUnits();
    if(!Units.IsValidIndex(Index) || Units[Index].Id<0 || Units[Index].Troops<=0 || (Field->IsPlaying() && !Field->bRealtimeCommand)) return;
    const auto& Unit=Units[Index];
    if(Index<3) Field->SelectOfficer(Index);
    else if(Unit.bEnemy)
    {
        const auto Cell=hexgrid::Pick(Unit.Position.X/200.,Unit.Position.Y/200.);
        Field->SelectAt(frontline::Campaign::At(Cell.first,Cell.second));bEnemyDetailsOpen=true;
    }
    else Focus=Unit.Position;
    IgnoreMapInputThroughFrame=Frames+1;LabelLayoutFrame=MAX_uint64;
}
