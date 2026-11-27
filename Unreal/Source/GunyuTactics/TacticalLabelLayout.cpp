#include "TacticalTrial.h"
#include "TacticalChrome.h"
#include "JourneyGrandBattle.hpp"
#include "Campaign.hpp"
#include "TacticalTargetVisual.h"
#include "TacticalActionFocus.h"
#include "TacticalBattleResponse.h"
#include "TacticalMapLabelLayout.h"
#include "TacticalCommandOverlayLayout.h"
#include "TacticalMissionSignals.h"
#include "HexGrid.hpp"

void ATacticalController::LayoutMapLabels()
{
    using namespace gunyumaplabel;
    LabelLayoutFrame=GFrameCounter;UnitLabelPositions.Init(Hidden,frontline::GrandBattleArmyCount);GoalLabelPositions.Init(Hidden,7);MapLabelLeaders.Reset();
    if(!Field || Field->Presentation().Journey.bVisible) return;
    const auto& Units=Field->VisualUnits();const auto& H=Field->Presentation();
    const bool WideShot=bTacticCinema && Field->ReadTacticCinema().bVisible;
    const auto Action=Field->ReadActionFocus();
    const bool Focused=WideShot && Action.bActive && Action.bHasTarget && Action.Weight>.20f;
    const FVector2D Min=gunyulayout::MapOrigin,Max=WideShot ? FVector2D(1584,718):Min+gunyulayout::MapSize;
    TArray<FSlateRect> Occupied;
    if(bTacticCinema && Field->ReadMissionResponse().Weight>.001f) Occupied.Add(FSlateRect(32,120,732,260));
    if(bTacticCinema && !bTacticPickerOpen && !bEpisodeOpen && !bSaveMenuOpen && !bBattleLogOpen && !bMarchPlanOpen && Field->ReadGrandMoment().Weight>.001f)
        Occupied.Add(FSlateRect(30,116,1020,270));
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
    if(!Field->IsPlaying() && !bTacticPickerOpen)
    {
        if(!bDetailedBattle) Occupied.Add(FSlateRect(16,gunyucommandlayout::EnemyTop,16+gunyucommandlayout::EnemyWidth,gunyucommandlayout::EnemyTop+gunyucommandlayout::EnemyHeight));
        else Occupied.Add(FSlateRect(16,116,716,166));
        if(bEnemyDetailsOpen)
        { const float Top=gunyucommandlayout::EnemyDetailTop(bDetailedBattle);Occupied.Add(FSlateRect(16,Top,716,Top+132)); }
    }
    Occupied.Add(FSlateRect(16,gunyucommandlayout::FeedTop,16+gunyucommandlayout::FeedWidth,gunyucommandlayout::FeedTop+gunyucommandlayout::FeedHeight));
    TArray<int> Priority;
    if(Action.bActive && Action.bHasTarget) { Priority.AddUnique(Action.Actor);Priority.AddUnique(Action.Target); }
    Priority.AddUnique(H.Selected);
    for(int Id=0;Id<Units.Num();++Id) if(gunyutargetui::MapTargetSelected(H,Units[Id])) Priority.AddUnique(Id);
    if(H.MissionFocusTile>=0)
        for(int Id=5;Id<Units.Num();++Id)
            if(Units[Id].Id>=0 && !Units[Id].bEnemy && FVector::DistSquared2D(Units[Id].Position,Field->SurfaceLocation(H.MissionFocusTile))<1)
                Priority.AddUnique(Id);
    const int AttentionCount=Priority.Num();
    for(int Id=0;Id<3;++Id) Priority.AddUnique(Id);
    Priority.AddUnique(5);
    for(int Id=0;Id<Units.Num();++Id) Priority.AddUnique(Id);
    auto PlaceUnit=[&](int Id)
    {
        if(!Units.IsValidIndex(Id) || Units[Id].Id<0 || (Units[Id].Troops<=0 && !Field->IsPlaying())) return;
        if(Focused && Id!=Action.Actor && Id!=Action.Target) return;
        const auto& U=Units[Id];const auto Anchor=MapLabelPosition(U.Position+FVector(0,0,70),FVector2D::ZeroVector);
        if(Anchor.X<Min.X-60 || Anchor.X>Max.X+60 || Anchor.Y<Min.Y-60 || Anchor.Y>Max.Y+60) return;
        const auto Size=UnitLabelSize(Id),Base=MapLabelPosition(U.Position+FVector(0,0,240),Size);
        const auto Chosen=Reserve(Base,Size,Min,Max,Occupied);
        UnitLabelPositions[Id]=Chosen;
        if(Chosen==Hidden) return;
        const auto Edge=NearestEdge(Anchor,Chosen,Size);
        if(FVector2D::Distance(Anchor,Edge)>5)
        {
            const auto* Read=gunyutargetui::Find(H,Id);
            const FLinearColor Color=Read && Read->bSelected ? gunyutargetui::Color(*Read):
                Id==H.Selected ? FLinearColor(1.f,.81f,.35f):U.bEnemy ? FLinearColor(.98f,.48f,.31f):FLinearColor(.44f,.88f,.82f);
            MapLabelLeaders.Add({Anchor,Edge,Color,Id,Id==H.Selected || gunyutargetui::MapTargetSelected(H,U) ? 1.8f:1.1f});
        }
    };
    // The currently controlled formation and observed target get space first.
    for(int I=0;I<AttentionCount;++I) PlaceUnit(Priority[I]);
    const int DutyTile=SelectedDutyTile();
    const int TrafficTile=DutyTile>=0 && H.DutyProgress.bTraffic ? H.DutyProgress.InspectTile:-1;
    const int Tiles[]={H.GoalTile,H.SecondGoalTile,DutyTile,TrafficTile,ShowOpeningOrder() ? H.OpeningOrder.Tile:-1,
        H.Crossings[0].Tile,H.Crossings[1].Tile};
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
            const FVector2D Edge=NearestEdge(Anchor,P,PlateSize);
            MapLabelLeaders.Add({Anchor,Edge,FLinearColor(.95f,.83f,.52f),-1,1.1f});
        }
    }
    for(int I=AttentionCount;I<Priority.Num();++I) PlaceUnit(Priority[I]);
}
FString ATacticalController::UnitMapStatus(int Index) const
{
    const auto& Units=Field->VisualUnits();if(!Units.IsValidIndex(Index) || Units[Index].Id<0) return FString();
    const auto& U=Units[Index];
    if(U.bTransport && !U.bEnemy)
    {
        const auto Cart=Field->ReadMissionSignals().Cart;
        if(Cart.Weight>.03f)
        {
            if(Cart.Count==0) return TEXT("護送車が前進中");
            return FString::Printf(TEXT("%s %d/%d"),Cart.Kind==gunyumission::ESignal::RoadOpened ? TEXT("車道再開"):TEXT("護送が前進"),Cart.Count,Cart.Total);
        }
    }
    if(Field->IsPlaying()) return Field->ReadUnitActivity(Index).Caption;
    if(U.Troops<=0) return TEXT("戦闘不能");
    if(U.bEnemy && !bDetailedBattle && !Field->Presentation().GrandOrderStatus.IsEmpty())
    {
        if(U.Morale<25) return TEXT("戦意低下・敗走");
        if(U.bDisordered) return TEXT("混乱");
        if(U.bGuarded) return TEXT("固守");
        return Index==8 ? TEXT("北の敵軍"):Index==9 ? TEXT("南の敵軍"):Index==10 ? TEXT("敵本陣"):TEXT("前陣の敵軍");
    }
    if(Field->Presentation().Position.Relief.bVisible && Field->Presentation().Position.Relief.Incoming==Index) return TEXT("後任を比較中");
    if(const auto* Read=gunyutargetui::Find(Field->Presentation(),Index))
        return Read->Badge+(U.bDisordered ? TEXT(" / 混乱"):U.bPressured ? TEXT(" / 隙"):U.bGuarded ? TEXT(" / 固守"):TEXT(""));
    if(U.bDisordered) return TEXT("混乱");
    if(U.bPressured) return TEXT("隙あり");
    if(U.Motion>.01f) return TEXT("行軍中");
    if(U.bConcealed) return TEXT("潜伏中");
    const auto& H=Field->Presentation();
    if(Index==7 && !H.VeteranStatus.IsEmpty()) return H.VeteranStatus;
    if(H.bGrandBattle && H.Mission==frontline::CoalitionMission && Index>=5 && Index<=6 && !H.CoalitionAllyStatus[Index-5].IsEmpty())
        return H.CoalitionAllyStatus[Index-5];
    if(U.bGuarded) return TEXT("固守");
    if(U.bInspired) return TEXT("鼓舞");
    const auto& O=Field->Presentation().Officers;
    if(Index<3 && O.IsValidIndex(Index)) return O[Index].Command;
    if(U.bTransport) return TEXT("護送車");
    if(U.bCavalry) return TEXT("騎兵");
    return Index>=5 && !U.bEnemy ? TEXT("共闘・自動進軍"):FString();
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
