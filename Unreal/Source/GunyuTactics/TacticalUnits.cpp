#include "TacticalInfantry.h"
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
        const auto Motion=gunyumotion::Read(Unit,State->Units,Beat,BeatProgress());
        const FRotator Heading=Motion.Facing.Rotation();
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
        gunyuinfantry::FBatches Body;
        const int Count=FMath::Clamp(FMath::CeilToInt(Unit.Troops/35.f),6,48);
        for(int Soldier=0;Soldier<Count;++Soldier)
        {
            const auto Pose=gunyumotion::Pose(Motion,Soldier,Unit.Id,State->VisualTime);
            FVector Base=Unit.Position+Heading.RotateVector(gunyumotion::FormationOffset(Motion,Soldier,Count));
            Base.Z=SurfaceHeight(Base);
            if(Unit.Troops<=0) { Cloth.Add(FTransform(Heading,Base+FVector(0,0,5),FVector(.30,.16,.08)));continue; }
            const float Stature=Motion.Stature;
            gunyuinfantry::Soldier(Body,Pose,Base,Heading.Quaternion(),Soldier==3 ? .86f:.72f,Stature);
        }
        Cloth.Append(Body.Cloth);IronParts.Append(Body.Iron);SkinParts.Append(Body.Skin);PoleParts.Append(Body.Poles);
        ArmourParts.Append(Body.Armour);HelmetParts.Append(Body.Helmets);ShieldParts.Append(Body.Shields);
        LimbParts.Append(Body.Limbs);SpearParts.Append(Body.Spears);
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
