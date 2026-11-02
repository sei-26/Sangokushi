#include "TacticalActionFocus.h"
#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalCompanyStyle.h"
#include "JourneyArchive.hpp"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInterface.h"
#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif
namespace gunyufocus
{
    bool SmokeTest()
    {
        auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_ACTION_FOCUS: FAIL %s"),Why);return V; };
        TArray<FVisualUnit> U;U.SetNum(6);
        for(int I=0;I<4;++I) { U[I].Id=I;U[I].Troops=1400;U[I].bEnemy=I==3; }
        U[3].Position=FVector(200,0,0);
        FBattleBeat B;B.Actor=1;B.Target=3;B.Kind=EBattleCue::Ambush;B.Before=B.After=U;B.After[3].Troops-=300;
        auto Height=[](FVector P) { return float(P.X*.01); };
        const auto R=Read(U,&B,.5f,1,Height);
        if(!Check(R.bActive && R.bHasTarget && R.Actor==1 && R.Target==3 && R.Clearings.Num()==3,TEXT("only the displayed officer, blade and known victim define the focus"))) return false;
        const FVector Camera=R.Hero+FVector(0,1000,300),Between=FMath::Lerp(Camera,R.Hero,.6);
        if(!Check(ClothOpacity(Between,Camera,R)<.2 && ClothOpacity(Between+FVector(500,0,0),Camera,R)==1,TEXT("only cloth covering the subject from this view fades"))) return false;
        if(!Check(ClothOpacity(R.Hero+(R.Hero-Camera),Camera,R)==1 && ClothOpacity(Camera+(Camera-R.Hero),Camera,R)==1,TEXT("cloth behind the subject or camera stays opaque"))) return false;
        if(!Check(!Read(U,&B,0,1,Height).bActive && !Read(U,&B,1,1,Height).bActive && ClothOpacity(Between,Camera,Read(U,&B,1,1,Height))==1,TEXT("flags recover at the end without a persistent clearing"))) return false;
        B.Kind=EBattleCue::Stop;if(!Check(!Read(U,&B,.5f,1,Height).bActive,TEXT("a stopped chain cannot frame an attack"))) return false;
        B.Kind=EBattleCue::Ambush;U[3].Id=-1;
        if(!Check(!Read(U,&B,.5f,1,Height).bActive,TEXT("unobserved victim cannot expose a position through cloth"))) return false;
        U[3]=B.Before[3];B.Before[3].Id=-1;
        if(!Check(!Read(U,&B,.5f,1,Height).bActive,TEXT("focus cannot borrow a target from the later snapshot"))) return false;
        B.Before[3]=U[3];U[1].Id=-1;
        if(!Check(!Read(U,&B,.5f,1,Height).bActive,TEXT("unobserved actor has no cinematic focus"))) return false;
        U[1]=B.Before[1];B.Kind=EBattleCue::Rally;B.Target=-1;
        const auto Support=Read(U,&B,.5f,1,Height);U[3].Position=FVector(99999,99999,99999);
        if(!Check(Support.bActive && !Support.bHasTarget && Support.Clearings.Num()==2 && Read(U,&B,.5f,1,Height).Center.Equals(Support.Center),TEXT("support uses the officer, never an unrelated enemy"))) return false;
        return true;
    }
}
bool ATacticalField::ActionReadabilitySmokeTest()
{
    if(!gunyufocus::SmokeTest()) return false;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_ACTION_FOCUS: FAIL %s"),Why);return V; };
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;const float SavedSpeed=PlaybackSpeed;
    const bool SavedCouncil=bStartWithCouncil,SavedMode=bRealtimeCommand,SavedPlayback=State->bNoPlayback,SavedReadable=bReadableStandards;
    const bool SavedView=State->bHasActionCamera;const FVector SavedCamera=State->ActionCamera;
    FirstMission=0;EnemyPolicy=0;bStartWithCouncil=false;bRealtimeCommand=false;PlaybackSpeed=1;State->bNoPlayback=false;bReadableStandards=true;
    Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);Issue(ETacticalCommand::Execute);AdvancePresentation(4.7f);
    const auto R=ReadActionFocus();
    if(!Check(R.bActive && R.Actor==1 && R.bHasTarget,TEXT("publicly executed ambush produces a live commander focus"))) return false;
    auto Snapshot=[&]()
    {
        std::vector<std::uint8_t> Bytes;auto J=std::make_unique<frontline::StrategistJourney>(State->Journey);
        auto RT=State->Realtime;auto Signal=State->ChainSignal;auto Orders=State->Orders;
        frontline::JourneyArchive W{false,Bytes};W(*J,RT,Signal,Orders);return Bytes;
    };
    const auto Before=Snapshot();
    const auto& FocusUnit=VisualUnits()[R.Actor];
    const auto Motion=gunyumotion::Read(FocusUnit,VisualUnits(),CurrentBeat(),BeatProgress());
    FVector Cloth=FocusUnit.Position+Motion.Facing.Rotation().RotateVector(gunyucompany::StandardPoint({.5f,.5f}));
    const auto Top=gunyucompany::StandardPoint({0,0});
    const FVector Base=FocusUnit.Position+Motion.Facing.Rotation().RotateVector(FVector(Top.X,Top.Y,0));
    Cloth.Z+=SurfaceHeight(Base)-FocusUnit.Position.Z;
    SetActionView(R.Hero+(Cloth-R.Hero)*8);
    const auto* Section=Standards->GetProcMeshSection(0);int Minimum=255;
    if(Section) for(const auto& V:Section->ProcVertexBuffer) Minimum=FMath::Min(Minimum,int(V.Color.A));
    if(!Check(Section && Minimum<160,TEXT("real banner vertices fade where they cover the actual commander"))) return false;
    if(StandardMaterial && StandardMaterial->GetName()==TEXT("M_ArmyStandardV2"))
        if(!Check(Standards->GetMaterial(0) && Standards->GetMaterial(0)->GetName()==TEXT("M_ArmyStandardReadableV8"),TEXT("default banners use the editable opacity material"))) return false;
    auto UpdateToggle=[&]()
    {
#if WITH_EDITOR
        FPropertyChangedEvent Event(FindFProperty<FProperty>(GetClass(),GET_MEMBER_NAME_CHECKED(ATacticalField,bReadableStandards)));
        PostEditChangeProperty(Event);
#else
        RenderStandards();
#endif
    };
    bReadableStandards=false;UpdateToggle();Section=Standards->GetProcMeshSection(0);
    for(const auto& V:Section->ProcVertexBuffer) if(!Check(V.Color.A==255,TEXT("disabling readability restores every cloth vertex"))) return false;
    bReadableStandards=true;UpdateToggle();SetActionView(R.Hero+FVector(0,-900,600));
    if(!Check(Before==Snapshot(),TEXT("camera and cloth change no orders, food, damage, history or time"))) return false;
    Issue(ETacticalCommand::Skip);
    if(!Check(!ReadActionFocus().bActive,TEXT("skipping removes the transient focus immediately"))) return false;
    bReadableStandards=SavedReadable;FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedMode;
    PlaybackSpeed=SavedSpeed;State->bNoPlayback=SavedPlayback;State->bHasActionCamera=SavedView;State->ActionCamera=SavedCamera;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_ACTION_FOCUS: PASS (observed hero/blade/victim, camera-ray cloth fade, support/stop/privacy, real banner material, toggle, skip, immutable simulation)"));return true;
}
