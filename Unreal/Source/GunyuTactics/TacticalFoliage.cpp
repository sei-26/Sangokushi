#include "TacticalTrial.h"
#include "TacticalState.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

void ATacticalField::UpdateFoliageReadability()
{
    if (!FoliageMaterial) return;
    // Save the editable asset in the level, never a transient runtime instance.
    if (!GetWorld() || !GetWorld()->IsGameWorld())
    { Trees->SetMaterial(0,FoliageMaterial); return; }
    if (!FoliageInstance || FoliageInstanceSource.Get()!=FoliageMaterial.Get())
    {
        FoliageInstance=UMaterialInstanceDynamic::Create(FoliageMaterial,this);
        FoliageInstanceSource=FoliageMaterial;
        State->FoliageClearings.Reset();
        Trees->SetMaterial(0,FoliageInstance);
    }
    constexpr int Slots=6;
    const FLinearColor Disabled(-100000,-100000,0,0);
    TArray<FLinearColor> Positions;Positions.Init(Disabled,Slots);
    // Exactly the same observed/interpolated units used to draw the armies.
    // Hidden reserves do not clear leaves or leak a position into the material.
    for (const auto& Unit : State->Units)
    {
        if (!bReadableFoliage || Unit.Id<0 || Unit.Id>=Slots || Unit.Troops<=0) continue;
        const float Radius=Unit.Id==State->Selected ? 130.f : 105.f;
        Positions[Unit.Id]=FLinearColor(Unit.Position.X,Unit.Position.Y,Unit.Position.Z,Radius);
    }
    for (int I=0;I<Slots;++I)
        if (!State->FoliageClearings.IsValidIndex(I) || Positions[I]!=State->FoliageClearings[I])
            FoliageInstance->SetVectorParameterValue(FName(*FString::Printf(TEXT("UnitClear%d"),I)),Positions[I]);
    State->FoliageClearings=MoveTemp(Positions);
}
