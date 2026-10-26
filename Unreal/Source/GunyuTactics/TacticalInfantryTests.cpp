#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalInfantry.h"
#include "Components/InstancedStaticMeshComponent.h"

bool ATacticalField::InfantryMotionSmokeTest()
{
    using namespace gunyumotion;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_INFANTRY: FAIL %s"),Why);return V; };
    TArray<FVisualUnit> Seen;Seen.SetNum(6);
    Seen[0].Id=0;Seen[0].Troops=1400;Seen[0].Position=FVector(0,0,0);
    Seen[3].Id=3;Seen[3].Troops=1800;Seen[3].bEnemy=true;Seen[3].Position=FVector(200,0,0);
    FBattleBeat Beat;Beat.Before=Beat.After=Seen;Beat.Actor=0;Beat.Target=3;Beat.Kind=EBattleCue::Strike;Beat.After[3].Troops-=300;
    auto Wind=Pose(Read(Seen[0],Seen,&Beat,.28f),0,0,0);
    auto Contact=Pose(Read(Seen[0],Seen,&Beat,.52f),0,0,0);
    const FVector Shaft=Contact.Spear.RotateVector(FVector::UpVector);
    if(!Check(Shaft.X>.95 && Wind.Spear.RotateVector(FVector::UpVector).Z>Shaft.Z,TEXT("windup becomes a forward thrust, not a backward tilt"))) return false;
    if(!Check((Contact.SpearCenter-Contact.Hand[1]).Equals(Contact.Spear.RotateVector(FVector(0,0,6)),.001),TEXT("spear grip stays attached to the right hand"))) return false;
    Seen[3].Troops=0;
    if(!Check(Read(Seen[0],Seen,&Beat,.52f).bAttack,TEXT("a killing blow follows through after public target casualties"))) return false;
    Seen[3].Troops=1500;
    for(float Stature:{.65f,1.f})
    {
        gunyuinfantry::FBatches B;gunyuinfantry::Soldier(B,Contact,FVector::ZeroVector,FQuat::Identity,.72f,Stature);
        const auto& Pole=B.Poles[0];const auto& Head=B.Spears[0];
        const FVector Tip=Pole.TransformPosition(FVector(0,0,50)),BladeRoot=Head.TransformPosition(FVector(0,0,-50));
        if(!Check(FVector::Dist(Tip,BladeRoot)<3,TEXT("spearhead meets the shaft while crouching or standing"))) return false;
    }
    Seen[3].Loss=300;
    if(!Check(Read(Seen[3],Seen,&Beat,.47f).Recoil==0 && Read(Seen[3],Seen,&Beat,.56f).Recoil>0,TEXT("casualty recoil starts only after the actual impact"))) return false;
    Beat.Kind=EBattleCue::Stop;
    if(!Check(!Read(Seen[0],Seen,&Beat,.52f).bAttack && Read(Seen[3],Seen,&Beat,.56f).Recoil==0,TEXT("stopped chains invent no thrust or impact"))) return false;
    Beat.Kind=EBattleCue::Strike;Beat.Target=4;
    if(!Check(!Read(Seen[0],Seen,&Beat,.52f).bAttack,TEXT("unknown reserve cannot provoke a visible attack pose"))) return false;
    Beat.Kind=EBattleCue::March;Beat.Target=-1;Beat.After=Beat.Before;Beat.After[0].Position=FVector(200,0,0);
    for(int I=0;I<48;++I)
    {
        const FVector Start=FormationOffset(Read(Seen[0],Seen,&Beat,0),I,40),End=FormationOffset(Read(Seen[0],Seen,&Beat,1),I,40);
        if(!Check(Start.Equals(End,.01) && Start.Size2D()<125,TEXT("column transition returns to its bounded resting formation"))) return false;
        for(float P:{.12f,.18f,.24f,.48f,.70f,.78f,.84f,.90f})
        {
            const auto A=Read(Seen[0],Seen,&Beat,P),B=Read(Seen[0],Seen,&Beat,P+.001f);
            if(!Check(FVector::Dist(FormationOffset(A,I,40),FormationOffset(B,I,40))<3,TEXT("formation transition stays continuous across travel endpoints"))) return false;
        }
    }
    const auto Walking=Read(Seen[0],Seen,&Beat,.40f);
    const auto FootA=Pose(Walking,0,0,0),FootB=Pose(Walking,0,0,.12f);
    if(!Check(!FootA.Foot[0].Equals(FootB.Foot[0]) && FootA.Foot[0].Z>=3 && FootB.Foot[1].Z>=3,TEXT("alternating steps lift feet without sinking under ground"))) return false;
    for(float Phase:{0.f,.28f,.48f,.56f,.80f,1.f})
    {
        gunyuinfantry::FBatches B;
        auto M=Read(Seen[0],Seen,&Beat,Phase);
        for(int I=0;I<48;++I) gunyuinfantry::Soldier(B,Pose(M,I,0,1.3f),FVector::ZeroVector,FQuat::Identity,.72f,.65f);
        if(!Check(B.Limbs.Num()==48*8 && B.Spears.Num()==48,TEXT("bounded eight connected limb segments per representative soldier"))) return false;
        for(const auto* Parts:{&B.Cloth,&B.Iron,&B.Skin,&B.Poles,&B.Armour,&B.Helmets,&B.Shields,&B.Limbs,&B.Spears})
            for(const auto& T:*Parts) if(!Check(!T.ContainsNaN() && T.GetScale3D().GetMin()>0,TEXT("all joint transforms remain finite and positive"))) return false;
    }
    const bool SavedRealtime=bRealtimeCommand,SavedCouncil=bStartWithCouncil;
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    PrepareOpeningPreview(false);bRealtimeCommand=true;Issue(ETacticalCommand::Deploy);
    if(!RealtimePaused()) Issue(ETacticalCommand::RealtimePause);
    auto Transforms=[&]()
    {
        TArray<FTransform> Result;
        for(auto* Mesh:{Limbs.Get(),Weapons.Get()}) for(int I=0;I<Mesh->GetInstanceCount();++I)
        { FTransform T;Mesh->GetInstanceTransform(I,T);Result.Add(T); }
        return Result;
    };
    const auto Initial=Transforms();
    Issue(ETacticalCommand::RealtimePause);AdvancePresentation(.20f);
    const auto Moving=Transforms();bool Changed=false;
    for(int I=0;I<Initial.Num() && I<Moving.Num();++I) Changed=Changed || !Initial[I].Equals(Moving[I],.001);
    if(!Check(Changed,TEXT("unpaused presentation genuinely animates articulated parts"))) return false;
    Issue(ETacticalCommand::RealtimePause);
    std::vector<std::uint8_t> Before,After;
    if(!Check(ExportSession(Before),TEXT("stable battle export"))) return false;
    AdvancePresentation(.6f);const auto Frozen=Transforms();bool Identical=Moving.Num()==Frozen.Num();
    for(int I=0;I<Moving.Num() && Identical;++I) Identical=Moving[I].Equals(Frozen[I],.001);
    if(!Check(Identical && Limbs->GetInstanceCount()==Spearheads->GetInstanceCount()*8,TEXT("paused battlefield freezes every joint and weapon at a bounded count"))) return false;
    if(!Check(ExportSession(After) && Before==After,TEXT("joint animation consumes no troops, orders, food or time"))) return false;
    if(!Check(Limbs->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("animated arms and legs cannot block orders"))) return false;
    bRealtimeCommand=SavedRealtime;bStartWithCouncil=SavedCouncil;FirstMission=SavedMission;EnemyPolicy=SavedPolicy;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_INFANTRY: PASS (forward thrust, attached grip, timed recoil, stopped/hidden target, continuous ranks, joint bounds, pause, immutable session)"));return true;
}
