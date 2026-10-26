#include "TacticalFormation.h"
#include "TacticalTrial.h"
#include "TacticalState.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "ProfilingDebugging/CsvProfiler.h"

namespace
{
    void Commit(UInstancedStaticMeshComponent* Mesh, const TArray<FTransform>& Transforms)
    {
        if (Mesh->GetInstanceCount() == Transforms.Num())
            Mesh->BatchUpdateInstancesTransforms(0, Transforms, false, true, true);
        else
        {
            Mesh->ClearInstances();
            Mesh->AddInstances(Transforms, false, false);
        }
    }
}
void ATacticalField::RenderUnits()
{
    CSV_SCOPED_TIMING_STAT_GLOBAL(GunyuUnits);
    UpdateFoliageReadability();
    TArray<FTransform> AllyParts, EnemyParts, IronParts, SkinParts, PoleParts;
    TArray<FTransform> ArmourParts, HelmetParts, ShieldParts;
    TArray<FTransform> LimbParts, SpearParts;
    TArray<FTransform> WheelParts;
    const auto* Beat = CurrentBeat();
    for (const auto& Unit : State->Units)
    {
        if (Unit.Id < 0 || (Unit.Troops <= 0 && !IsPlaying())) continue;
        auto& Cloth = Unit.bEnemy ? EnemyParts : AllyParts;
        FVector Facing = Unit.bEnemy ? FVector(-1, 0, 0) : FVector(1, 0, 0);
        if (Beat && Beat->Before.IsValidIndex(Unit.Id) && Beat->After.IsValidIndex(Unit.Id))
        {
            const FVector Travel = Beat->After[Unit.Id].Position - Beat->Before[Unit.Id].Position;
            if (Travel.SizeSquared() > 1) Facing = Travel.GetSafeNormal();
            else if (Unit.Id == Beat->Actor && State->Units.IsValidIndex(Beat->Target)) Facing = (State->Units[Beat->Target].Position - Unit.Position).GetSafeNormal();
        }
        bool bSkirmish=false;
        if (Beat && Beat->Kind==EBattleCue::Enemy && Beat->After.IsValidIndex(Unit.Id) && Beat->Before.IsValidIndex(Unit.Id))
            for (const auto& Other : Beat->Before)
            {
                if (Other.Id<0 || Other.bEnemy==Unit.bEnemy || !Beat->After.IsValidIndex(Other.Id) || Other.Troops<=0) continue;
                const bool Loss=Beat->After[Other.Id].Troops<Other.Troops || Beat->After[Unit.Id].Troops<Beat->Before[Unit.Id].Troops;
                if (Loss && FVector::DistSquared2D(Unit.Position,Other.Position)<250*250)
                {
                    bSkirmish=true;
                    if (Unit.Motion<.01f) Facing=(Other.Position-Unit.Position).GetSafeNormal2D();
                    break;
                }
            }
        const FRotator Heading = Facing.Rotation();
        const bool bActive = Beat && (Unit.Id == Beat->Actor || Unit.Id == Beat->Target);
        const bool bAttack = bSkirmish || (bActive && Unit.Id == Beat->Actor &&
            (Beat->Kind==EBattleCue::Strike || Beat->Kind==EBattleCue::Ambush || Beat->Kind==EBattleCue::Breakthrough || Beat->Kind==EBattleCue::Feint || Beat->Kind==EBattleCue::Disrupt || Beat->Kind==EBattleCue::AzureDragon));
        const float AttackPose=bAttack ? FMath::Sin(FMath::Clamp((BeatProgress()-.24f)/.45f,0.f,1.f)*PI) : 0;
        if (Unit.Id == frontline::StrategistJourney::ConvoyArmy)
        {
            PoleParts.Add(FTransform(Heading, Unit.Position + FVector(0, 0, 31), FVector(1.12, .76, .12)));
            for (int Side : {-1,1})
                PoleParts.Add(FTransform(Heading,Unit.Position+Heading.RotateVector(FVector(0,Side*36,40)),FVector(1.08,.055,.14)));
            for (int Side : {-1, 1}) for (int End : {-1, 1})
                PoleParts.Add(FTransform(Heading, Unit.Position + Heading.RotateVector(FVector(End * 44, Side * 30, 49)), FVector(.045, .045, .40)));
            for (int Crate = 0; Crate < 3; ++Crate)
                SkinParts.Add(FTransform(Heading, Unit.Position + Heading.RotateVector(FVector((Crate - 1) * 29, 0, 47)), FVector(.24, .42, .23)));
            PoleParts.Add(FTransform(Heading, Unit.Position + Heading.RotateVector(FVector(65, 0, 27)), FVector(.6, .05, .05)));
            for (int Side : {-1, 1}) for (int Axle : {-1, 1})
                WheelParts.Add(FTransform(Heading, Unit.Position + Heading.RotateVector(FVector(Axle * 31, Side * 37, 20)), FVector(.38, .50, .38)));
            continue;
        }
        const auto Formation=gunyuformation::Shape(Unit,bAttack);
        const float Kneel = Unit.bConcealed ? .65f : 1.f;
        const int Count = FMath::Clamp(FMath::CeilToInt(Unit.Troops / 35.f), 6, 48);
        for (int Soldier = 0; Soldier < Count; ++Soldier)
        {
            const float Stride = FMath::Sin(State->VisualTime * 16 + Soldier * 1.7f) * Unit.Motion;
            const float Idle=FMath::Sin(State->VisualTime*2 + Soldier*.73f + Unit.Id);
            const bool ShieldFront=Formation==gunyuformation::EShape::ShieldWall && Soldier<8;
            const FVector Offset=gunyuformation::Offset(Formation,Soldier,Count)+FVector(AttackPose*5,0,0);
            const FRotator SoldierHeading=Heading+FRotator(0,Unit.bDisordered ? FMath::Sin(Soldier*1.7f)*22:0,0);
            FVector Base = Unit.Position + Heading.RotateVector(Offset);
            Base.Z = SurfaceHeight(Base) + FMath::Abs(Stride) * 2 + Idle*.25f;
            // A full-sized standard and denser, smaller figures read as an army.
            const float FigureScale = Soldier == 3 ? .86f : .72f;
            auto Part = [&](TArray<FTransform>& Batch, FVector Local, FVector Scale, FRotator Rotation = FRotator::ZeroRotator)
            {
                Local.Z *= Kneel*(ShieldFront ? .9f:1.f); Scale.Z *= Kneel*(ShieldFront ? .9f:1.f);
                Local *= FigureScale; Scale *= FigureScale;
                Batch.Add(FTransform(SoldierHeading + Rotation, Base + SoldierHeading.RotateVector(Local), Scale));
            };
            if (Unit.Troops <= 0)
            { Part(Cloth, FVector(0, 0, 7), FVector(.40, .22, .12)); continue; }
            Part(Cloth, FVector(0, 0, 37), FVector(.25, .32, .33));
            Part(ArmourParts, FVector(0, 0, 38), FVector(.46, .32, .38));
            Part(IronParts, FVector(0, 0, 24), FVector(.28, .34, .13)); // armored skirt
            Part(Cloth, FVector(0, 0, 31), FVector(.29, .355, .055)); // sash
            // Rectangular infantry shield: silhouette reads at the default tactical zoom.
            Part(ShieldParts, FVector(ShieldFront ? 23:15, -24, ShieldFront ? 43:38), FVector(.85, .32, .43), FRotator(0, -12, 0));
            Part(Cloth, FVector(20, -24, 38), FVector(.035, .22, .31), FRotator(0, -12, 0));
            Part(SkinParts, FVector(0, 0, 60), FVector(.16, .16, .20));
            Part(HelmetParts, FVector(0, 0, 68), FVector(.22, .22, .22));
            Part(Cloth, FVector(-3, 0, 79), FVector(.06, .07, .14)); // helmet crest
            for (int Side : {-1, 1})
            {
                Part(LimbParts, FVector(Stride * Side * 5, Side * 7, 14), FVector(.105, .105, .27), FRotator(Stride * Side * 16, 0, 0));
                Part(LimbParts, FVector(0, Side * 18, 39), FVector(.12, .12, .27), FRotator(Stride * Side * 13, 0, 0));
                Part(IronParts, FVector(3 + Stride*Side*5,Side*7,3),FVector(.13,.11,.07));
            }
            const FRotator SpearAngle((ShieldFront ? 52.f:8.f) + AttackPose*38 + Idle*1.4f,0,0);
            Part(PoleParts,FVector(7,22,49),FVector(.025,.025,.92),SpearAngle);
            Part(SpearParts, FVector(7,22,49)+SpearAngle.RotateVector(FVector(0,0,54)),FVector(.18,.18,.20),SpearAngle);
        }
        // Large standards make whole formations readable, including at strategic zoom.
        PoleParts.Add(FTransform(FRotator::ZeroRotator, Unit.Position + FVector(-34,0,110),FVector(.03,.03,2.2)));
        PoleParts.Add(FTransform(FRotator::ZeroRotator, Unit.Position + FVector(10,0,218),FVector(.94,.028,.028)));
        IronParts.Add(FTransform(FRotator::ZeroRotator, Unit.Position + FVector(-34,0,228),FVector(.08,.08,.15)));
    }
    Commit(Soldiers, AllyParts); Commit(Enemies, EnemyParts); Commit(Helmets, IronParts);
    Commit(Weapons, PoleParts); Commit(Faces, SkinParts);
    Commit(Armour, ArmourParts); Commit(WarHelmets, HelmetParts); Commit(Shields, ShieldParts);
    Commit(Limbs,LimbParts); Commit(Spearheads,SpearParts);
    Commit(CartWheels,WheelParts);
    RenderStandards();
}
