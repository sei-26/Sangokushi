#include "TacticalInfantry.h"
#include "TacticalTrial.h"
#include "TacticalWeaponMotion.h"
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
    void Horse(TArray<FTransform>& Parts,FVector Base,const FRotator& Heading,float Stride,float Phase)
    {
        const float Bob=FMath::Sin(Phase)*Stride*2.f;
        auto Part=[&](FVector Local,FVector Scale,float Pitch=0.f)
        {
            Local.Z+=Bob;
            Parts.Emplace(Heading.Quaternion()*FRotator(Pitch,0,0).Quaternion(),Base+Heading.RotateVector(Local),Scale);
        };
        Part(FVector(-3,0,26),FVector(.48,.20,.22));
        Part(FVector(14,0,34),FVector(.23,.14,.26),-29);
        Part(FVector(22,0,44),FVector(.24,.12,.13),-15);
        Part(FVector(-25,0,23),FVector(.24,.047,.048),26);
        for(int Side:{-1,1})
        {
            Part(FVector(21,Side*3,50),FVector(.035,.034,.075),-13);
            for(int End:{-1,1})
            {
                const float Kick=Side*End*Stride*FMath::Sin(Phase)*4.f;
                Part(FVector(End*14+Kick,Side*6,13),FVector(.065,.059,.27),Kick*3.f);
            }
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
    TArray<FTransform> SwordParts,GlaiveParts,SerpentParts;
    TArray<FTransform> HeadParts,BeardParts,CapeParts;
    TArray<float> CapeColors;
    TArray<float> AllyColors,EnemyColors;
    TArray<FTransform> WheelParts;
    TArray<FTransform> HorseParts;
    const auto* Beat = CurrentBeat();
    for (const auto& Unit : State->Units)
    {
        if (Unit.Id < 0 || (Unit.Troops <= 0 && !IsPlaying())) continue;
        auto& Cloth = Unit.bEnemy ? EnemyParts : AllyParts;
        const int FirstCloth=Cloth.Num();
        const auto Motion=gunyumotion::Read(Unit,State->Units,Beat,BeatProgress());
        const FRotator Heading=Motion.Facing.Rotation();
        if (Unit.bTransport)
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
        const int Count=gunyucompany::RepresentativeCount(Unit);
        for(int Soldier=0;Soldier<Count;++Soldier)
        {
            const auto Equipment=gunyucompany::Equipment(Unit,Soldier,Count);
            const auto Pose=gunyuweapon::Pose(Motion,Equipment,Soldier,Unit.Id,State->VisualTime);
            FVector Base=Unit.Position+Heading.RotateVector(gunyumotion::FormationOffset(Motion,Soldier,Count));
            Base.Z=SurfaceHeight(Base);
            if(Unit.Troops<=0) { Cloth.Add(FTransform(Heading,Base+FVector(0,0,5),FVector(.30,.16,.08)));continue; }
            const float Stature=Motion.Stature;
            if(Unit.bCavalry)
            {
                Horse(HorseParts,Base,Heading,Unit.Motion,State->VisualTime*15.f+Soldier*1.8f);
                Base.Z+=27.f;
            }
            gunyuinfantry::Soldier(Body,Pose,Base,Heading.Quaternion(),Equipment.VisualScale,Stature,Equipment);
        }
        Cloth.Append(Body.Cloth);IronParts.Append(Body.Iron);SkinParts.Append(Body.Skin);PoleParts.Append(Body.Poles);
        ArmourParts.Append(Body.Armour);HelmetParts.Append(Body.Helmets);ShieldParts.Append(Body.Shields);
        LimbParts.Append(Body.Limbs);SpearParts.Append(Body.Spears);
        SwordParts.Append(Body.Swords);GlaiveParts.Append(Body.Glaives);SerpentParts.Append(Body.Serpents);
        HeadParts.Append(Body.Heads);BeardParts.Append(Body.Beards);CapeParts.Append(Body.Capes);
        auto& Colors=Unit.bEnemy ? EnemyColors:AllyColors;
        const auto Tint=gunyucompany::Livery(Unit);
        for(int I=0;I<Body.Capes.Num();++I) CapeColors.Append({Tint.R,Tint.G,Tint.B});
        for(int I=FirstCloth;I<Cloth.Num();++I) Colors.Append({Tint.R,Tint.G,Tint.B});
        const FVector Top=gunyucompany::StandardPoint({0,0});
        FVector StandardBase=Unit.Position+Heading.RotateVector(FVector(Top.X,Top.Y,0));StandardBase.Z=SurfaceHeight(StandardBase);
        PoleParts.Add(FTransform(Heading,StandardBase+FVector(0,0,71),FVector(.023,.023,1.42)));
        PoleParts.Add(FTransform(Heading,StandardBase+Heading.RotateVector(FVector(27,0,142)),FVector(.58,.023,.023)));
        IronParts.Add(FTransform(Heading,StandardBase+FVector(0,0,148),FVector(.045,.045,.10)));
    }
    Commit(Soldiers, AllyParts); Commit(Enemies, EnemyParts); Commit(Helmets, IronParts);
    Commit(SculptedHeads,HeadParts);Commit(CommanderBeards,BeardParts);Commit(CommanderCapes,CapeParts);
    for(auto Pair:{TPair<UInstancedStaticMeshComponent*,const TArray<float>*>(Soldiers,&AllyColors),TPair<UInstancedStaticMeshComponent*,const TArray<float>*>(Enemies,&EnemyColors),TPair<UInstancedStaticMeshComponent*,const TArray<float>*>(CommanderCapes,&CapeColors)})
    {
        auto* Mesh=Pair.Key;
        if(Mesh->NumCustomDataFloats!=3) Mesh->SetNumCustomDataFloats(3);
        if(Mesh->GetInstanceCount()>0) Mesh->SetCustomData(0,Mesh->GetInstanceCount()-1,MakeArrayView(*Pair.Value),true);
    }
    Commit(Weapons, PoleParts); Commit(Faces, SkinParts);
    Commit(Armour, ArmourParts); Commit(WarHelmets, HelmetParts); Commit(Shields, ShieldParts);
    Commit(Limbs,LimbParts); Commit(Spearheads,SpearParts);
    Commit(CommanderSwords,SwordParts);Commit(CommanderGlaives,GlaiveParts);Commit(CommanderSerpents,SerpentParts);
    Commit(CartWheels,WheelParts);
    Commit(CavalryHorses,HorseParts);
    RenderStandards();
}
