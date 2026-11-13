#include "TacticalTrial.h"
#include "TacticalChrome.h"
#include "Campaign.hpp"
#include "TacticalTargetVisual.h"
#include "TacticalActionFocus.h"
#include "TacticalBattleResponse.h"
#include "TacticalMapLabelLayout.h"

void ATacticalController::LayoutMapLabels()
{
    using namespace gunyumaplabel;
    LabelLayoutFrame=GFrameCounter;UnitLabelPositions.Init(Hidden,6);GoalLabelPositions.Init(Hidden,4);MapLabelLeaders.Reset();
    if(!Field || Field->Presentation().Journey.bVisible) return;
    const auto& Units=Field->VisualUnits();const FVector2D Size(132,86);
    const bool WideShot=bTacticCinema && Field->ReadTacticCinema().bVisible;
    const auto Action=Field->ReadActionFocus();
    const bool Focused=WideShot && Action.bActive && Action.bHasTarget && Action.Weight>.20f;
    const FVector2D Min=gunyulayout::MapOrigin,Max=WideShot ? FVector2D(1584,718):Min+gunyulayout::MapSize;
    TArray<FSlateRect> Occupied;
    if(bTacticCinema && Field->ReadMissionResponse().Weight>.001f) Occupied.Add(FSlateRect(32,120,732,260));
    auto Protect=[&](FVector Center,FVector Extent)
    {
        FVector2D Lo(1e9,1e9),Hi(-1e9,-1e9);bool Valid=true;
        for(int X:{-1,1}) for(int Y:{-1,1}) for(int Z:{-1,1})
        {
            const auto P=MapLabelPosition(Center+Extent*FVector(X,Y,Z),FVector2D::ZeroVector);
            if(P.X<-1000) { Valid=false;continue; }
            Lo.X=FMath::Min(Lo.X,P.X);Lo.Y=FMath::Min(Lo.Y,P.Y);Hi.X=FMath::Max(Hi.X,P.X);Hi.Y=FMath::Max(Hi.Y,P.Y);
        }
        if(Valid) Occupied.Add(FSlateRect(Lo.X-6,Lo.Y-6,Hi.X+6,Hi.Y+6));
    };
    if(Field->Presentation().Position.bVisible) Occupied.Add(FSlateRect(16,174,Field->Presentation().Position.Relief.bVisible ? 408:368,Field->Presentation().Position.Relief.bVisible ? 694:534));
    // Leave the displayed formations and cart themselves readable, too.
    for(const auto& U:Units)
        if(U.Id>=0 && (U.Troops>0 || Field->IsPlaying()))
        {
            Protect(U.Position+FVector(0,0,45),FVector(110,110,45));
        }
    if(Focused) for(const auto& Subject:Action.Clearings) Protect(Subject.Center,FVector(Subject.Radius));
    if(bDetailedBattle && !WideShot && !Field->Presentation().Position.bVisible) Occupied.Add(FSlateRect(16,Field->bRealtimeCommand ? 486:586,246,704));
    if(bDetailedBattle && Field->bRealtimeCommand && !Field->IsPlaying() && !bTacticPickerOpen) Occupied.Add(FSlateRect(936,174,1252,278));
    if(Field->IsPlaying())
    {
        const auto C=Field->ReadTacticCinema();
        if(bTacticCinema && C.bVisible)
        {
            if(gunyuresponse::Read(Field->CurrentBeat(),Units,Field->BeatProgress()).Weight>.001f) Occupied.Add(FSlateRect(984,168,1572,304));
            if(C.Intro>.001f) { Occupied.Add(FSlateRect(20,180,340,660));Occupied.Add(FSlateRect(300,220,760,400)); }
            if(C.Impact>.001f && !C.Primary.IsEmpty()) { const auto P=TacticResultPosition();Occupied.Add(FSlateRect(P.X,P.Y,P.X+380,P.Y+142)); }
        }
        else Occupied.Add(FSlateRect(290,106,1050,210));
    }
    if(bEnemyDetailsOpen && !Field->IsPlaying()) Occupied.Add(FSlateRect(16,174,716,306));
    const auto& H=Field->Presentation();
    const int DutyTile=SelectedDutyTile();
    const int TrafficTile=DutyTile>=0 && H.DutyProgress.bTraffic ? H.DutyProgress.InspectTile:-1;
    const int Tiles[]={H.GoalTile,H.SecondGoalTile,DutyTile,TrafficTile};
    for(int Index=0;Index<UE_ARRAY_COUNT(Tiles) && H.bBattle;++Index)
    {
        const int Tile=Tiles[Index];
        if(Tile<0 || TConstArrayView<int>(Tiles,Index).Contains(Tile)) continue;
        const auto Surface=Field->SurfaceLocation(Tile);
        const auto Anchor=MapLabelPosition(Surface+FVector(0,0,22),FVector2D::ZeroVector);
        if(Anchor.X<0 || Anchor.X>1600 || Anchor.Y<0 || Anchor.Y>900) continue;
        const FVector2D PlateSize=GoalLabelSize(Index);
        const auto Base=MapLabelPosition(Surface+FVector(0,0,240),PlateSize);
        const auto P=Reserve(Base,PlateSize,gunyulayout::LabelOrigin,Max,Occupied);
        GoalLabelPositions[Index]=P;
        if(P!=Hidden && (Index>=2 || !P.Equals(Base,3)))
        {
            const FVector2D Edge(FMath::Clamp(Anchor.X,P.X,P.X+PlateSize.X),FMath::Clamp(Anchor.Y,P.Y,P.Y+PlateSize.Y));
            MapLabelLeaders.Add({Anchor,Edge});
        }
    }
    TArray<int> Priority;
    if(Focused) { Priority.Add(Action.Actor);Priority.AddUnique(Action.Target); }
    Priority.AddUnique(Field->Presentation().Selected);Priority.AddUnique(5);
    for(int Id=0;Id<6;++Id) Priority.AddUnique(Id);
    for(int Id:Priority)
    {
        if(!Units.IsValidIndex(Id) || Units[Id].Id<0 || (Units[Id].Troops<=0 && !Field->IsPlaying())) continue;
        if(Focused && Id!=Action.Actor && Id!=Action.Target) continue;
        const auto& U=Units[Id];const auto Anchor=MapLabelPosition(U.Position+FVector(0,0,70),FVector2D::ZeroVector);
        if(Anchor.X<Min.X-60 || Anchor.X>Max.X+60 || Anchor.Y<Min.Y-60 || Anchor.Y>Max.Y+60) continue;
        const auto Base=MapLabelPosition(U.Position+FVector(0,0,240),Size);
        const auto Chosen=Reserve(Base,Size,Min,Max,Occupied);
        UnitLabelPositions[Id]=Chosen;
        if(Chosen!=Hidden && !Chosen.Equals(Base,3)) MapLabelLeaders.Add({Anchor,Chosen+FVector2D(Size.X*.5,U.Loss>0 ? 82:56)});
    }
}
FString ATacticalController::UnitMapStatus(int Index) const
{
    const auto& Units=Field->VisualUnits();if(!Units.IsValidIndex(Index) || Units[Index].Id<0) return FString();
    const auto& U=Units[Index];
    if(Field->IsPlaying()) return Field->ReadUnitActivity(Index).Caption;
    if(U.Troops<=0) return TEXT("戦闘不能");
    if(Field->Presentation().Position.Relief.bVisible && Field->Presentation().Position.Relief.Incoming==Index) return TEXT("後任を比較中");
    if(const auto* Read=gunyutargetui::Find(Field->Presentation(),Index))
        return Read->Badge+(U.bDisordered ? TEXT(" / 混乱"):U.bPressured ? TEXT(" / 隙"):U.bGuarded ? TEXT(" / 固守"):TEXT(""));
    if(U.bDisordered) return TEXT("混乱");
    if(U.bPressured) return TEXT("隙あり");
    if(U.Motion>.01f) return TEXT("行軍中");
    if(U.bConcealed) return TEXT("潜伏中");
    if(U.bGuarded) return TEXT("固守");
    if(U.bInspired) return TEXT("鼓舞");
    const auto& O=Field->Presentation().Officers;
    if(Index<3 && O.IsValidIndex(Index)) return O[Index].Command;
    if(U.bTransport) return TEXT("護送車");
    if(U.bCavalry) return TEXT("騎兵");
    return Index==5 ? TEXT("共闘・自動進軍"):FString();
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
