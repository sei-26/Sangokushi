#include "TacticalTrial.h"
#include "ProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "StrategistJourney.hpp"

#include "TacticalState.h"
void FJourneyDeleter::operator()(FJourneyState* Value) const { delete Value; }

namespace
{
    UInstancedStaticMeshComponent* Instances(AActor* Owner, FName Name, const TCHAR* Mesh, const TCHAR* Material)
    {
        auto* Part = Owner->CreateDefaultSubobject<UInstancedStaticMeshComponent>(Name);
        Part->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, Mesh));
        Part->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, Material));
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        return Part;
    }
}

ATacticalField::ATacticalField()
{
    PrimaryActorTick.bCanEverTick = false;
    State.Reset(new FJourneyState());
    Ground = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Ground"));
    SetRootComponent(Ground);
    Ground->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FortressMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/Materials/M_MapRockV3.M_MapRockV3"));
    DistantMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/Materials/M_Field.M_Field"));
    WaterMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/Materials/M_BattleWater.M_BattleWater"));
    FoliageMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Battle/SM_LeafCrown.SM_LeafCrown"));
    RockMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Battle/SM_RockOutcrop.SM_RockOutcrop"));
    ArmourMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Battle/SM_LamellarArmor.SM_LamellarArmor"));
    WarHelmetMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Battle/SM_WarHelmet.SM_WarHelmet"));
    ShieldMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Battle/SM_InfantryShield.SM_InfantryShield"));
    GrassMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Battle/SM_GrassClump.SM_GrassClump"));
    GateRoofMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Battle/SM_GateRoof.SM_GateRoof"));
    SpearheadMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Battle/SM_SpearheadV2.SM_SpearheadV2"));
    CartWheelMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Battle/SM_CartWheelV2.SM_CartWheelV2"));
    PalisadeMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Art/Battle/SM_PalisadeStakeV4.SM_PalisadeStakeV4"));
    WatchtowerMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Art/Battle/SM_WatchtowerV4.SM_WatchtowerV4"));
    TentMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Art/Battle/SM_CampTentV4.SM_CampTentV4"));
    SupplyCrateMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Art/Battle/SM_SupplyCrateV4.SM_SupplyCrateV4"));
    StandardMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Art/Materials/M_ArmyStandardV2.M_ArmyStandardV2"));
    Soldiers = Instances(this, TEXT("Allies"), TEXT("/Engine/BasicShapes/Cube.Cube"), TEXT("/Game/Art/Materials/M_Ally.M_Ally"));
    Enemies = Instances(this, TEXT("Enemies"), TEXT("/Engine/BasicShapes/Cube.Cube"), TEXT("/Game/Art/Materials/M_Enemy.M_Enemy"));
    Trees = Instances(this, TEXT("Trees"), TEXT("/Engine/BasicShapes/Sphere.Sphere"), TEXT("/Game/Art/Materials/M_Trees.M_Trees"));
    Trunks = Instances(this, TEXT("TreeTrunks"), TEXT("/Engine/BasicShapes/Cylinder.Cylinder"), TEXT("/Game/Art/Materials/M_Bark.M_Bark"));
    Rocks = Instances(this, TEXT("RockOutcrops"), TEXT("/Engine/BasicShapes/Sphere.Sphere"), TEXT("/Game/Art/Materials/M_BattleRock.M_BattleRock"));
    Grass = Instances(this, TEXT("GrassTufts"), TEXT("/Engine/BasicShapes/Cone.Cone"), TEXT("/Game/Art/Materials/M_BattleGrass.M_BattleGrass"));
    Buildings = Instances(this, TEXT("FieldStructures"), TEXT("/Engine/BasicShapes/Cube.Cube"), TEXT("/Game/Art/Materials/M_Bark.M_Bark"));
    Roofs = Instances(this, TEXT("GateRoofs"), TEXT("/Game/Art/Battle/SM_GateRoof.SM_GateRoof"), TEXT("/Game/Art/Materials/M_BattleRoof.M_BattleRoof"));
    for (auto* Part : {Trunks.Get(), Rocks.Get(), Grass.Get(), Buildings.Get()}) Part->SetupAttachment(Ground);
    Roofs->SetupAttachment(Ground);
    Houses=Instances(this,TEXT("VillagePlaster"),TEXT("/Engine/BasicShapes/Cube.Cube"),TEXT("/Game/Art/Materials/M_MapPlasterV3.M_MapPlasterV3"));
    Houses->SetupAttachment(Ground);
    Palisades=Instances(this,TEXT("CampPalisades"),TEXT("/Game/Art/Battle/SM_PalisadeStakeV4.SM_PalisadeStakeV4"),TEXT("/Game/Art/Materials/M_Bark.M_Bark"));
    Watchtowers=Instances(this,TEXT("CampWatchtowers"),TEXT("/Game/Art/Battle/SM_WatchtowerV4.SM_WatchtowerV4"),TEXT("/Game/Art/Materials/M_Bark.M_Bark"));
    Tents=Instances(this,TEXT("CampTents"),TEXT("/Game/Art/Battle/SM_CampTentV4.SM_CampTentV4"),TEXT("/Game/Art/Materials/M_CampCanvasV4.M_CampCanvasV4"));
    SupplyCrates=Instances(this,TEXT("CampSupplies"),TEXT("/Game/Art/Battle/SM_SupplyCrateV4.SM_SupplyCrateV4"),TEXT("/Game/Art/Materials/M_CrateWoodV4.M_CrateWoodV4"));
    Stonework=Instances(this,TEXT("FieldStonework"),TEXT("/Engine/BasicShapes/Cube.Cube"),TEXT("/Game/Art/Materials/M_MapRockV3.M_MapRockV3"));
    for (auto* Part : {Palisades.Get(),Watchtowers.Get(),Tents.Get(),SupplyCrates.Get(),Stonework.Get()}) Part->SetupAttachment(Ground);
    CityWalls=Instances(this,TEXT("BackdropMasonry"),TEXT("/Engine/BasicShapes/Cube.Cube"),TEXT("/Game/Art/Materials/M_MapRockV3.M_MapRockV3"));
    CityTimber=Instances(this,TEXT("BackdropTimber"),TEXT("/Engine/BasicShapes/Cube.Cube"),TEXT("/Game/Art/Materials/M_Bark.M_Bark"));
    CityRoofs=Instances(this,TEXT("BackdropRoofs"),TEXT("/Game/Art/Battle/SM_GateRoof.SM_GateRoof"),TEXT("/Game/Art/Materials/M_BattleRoof.M_BattleRoof"));
    for(auto* Part:{CityWalls.Get(),CityTimber.Get(),CityRoofs.Get()}) Part->SetupAttachment(Ground);
    FieldBanners=CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("FieldBanners"));
    FieldBanners->SetupAttachment(Ground);FieldBanners->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FieldBanners->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/Materials/M_ArmyStandardV2.M_ArmyStandardV2")));
    Grass->SetCastShadow(false);
    Markers = Instances(this, TEXT("Markers"), TEXT("/Engine/BasicShapes/Cube.Cube"), TEXT("/Game/Art/Materials/M_Goal.M_Goal"));
    Soldiers->SetupAttachment(Ground);
    Enemies->SetupAttachment(Ground);
    Trees->SetupAttachment(Ground);
    Markers->SetupAttachment(Ground);
    Helmets = Instances(this, TEXT("Helmets"), TEXT("/Engine/BasicShapes/Sphere.Sphere"), TEXT("/Game/Art/Materials/M_Iron.M_Iron"));
    Armour = Instances(this, TEXT("LamellarArmour"), TEXT("/Game/Art/Battle/SM_LamellarArmor.SM_LamellarArmor"), TEXT("/Game/Art/Materials/M_BattleMetal.M_BattleMetal"));
    WarHelmets = Instances(this, TEXT("WarHelmets"), TEXT("/Game/Art/Battle/SM_WarHelmet.SM_WarHelmet"), TEXT("/Game/Art/Materials/M_BattleMetal.M_BattleMetal"));
    Shields = Instances(this, TEXT("InfantryShields"), TEXT("/Game/Art/Battle/SM_InfantryShield.SM_InfantryShield"), TEXT("/Game/Art/Materials/M_BattleShield.M_BattleShield"));
    Weapons = Instances(this, TEXT("Weapons"), TEXT("/Engine/BasicShapes/Cube.Cube"), TEXT("/Game/Art/Materials/M_Bark.M_Bark"));
    Faces = Instances(this, TEXT("Faces"), TEXT("/Engine/BasicShapes/Sphere.Sphere"), TEXT("/Game/Art/Materials/M_Skin.M_Skin"));
    Dust = Instances(this, TEXT("Dust"), TEXT("/Engine/BasicShapes/Sphere.Sphere"), TEXT("/Game/Art/Materials/M_Dust.M_Dust"));
    Limbs = Instances(this, TEXT("InfantryLimbs"), TEXT("/Engine/BasicShapes/Cylinder.Cylinder"), TEXT("/Game/Art/Materials/M_InfantryLeatherV2.M_InfantryLeatherV2"));
    Spearheads = Instances(this, TEXT("InfantrySpearheads"), TEXT("/Game/Art/Battle/SM_SpearheadV2.SM_SpearheadV2"), TEXT("/Game/Art/Materials/M_BattleMetal.M_BattleMetal"));
    Limbs->SetupAttachment(Ground); Spearheads->SetupAttachment(Ground);
    CartWheels=Instances(this,TEXT("CartWheels"),TEXT("/Game/Art/Battle/SM_CartWheelV2.SM_CartWheelV2"),TEXT("/Game/Art/Materials/M_Bark.M_Bark"));
    CartWheels->SetupAttachment(Ground);
    Standards = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ArmyStandards"));
    Standards->SetupAttachment(Ground); Standards->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Standards->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Art/Materials/M_ArmyStandardV2.M_ArmyStandardV2")));
    for (auto* Part : {Helmets.Get(), Armour.Get(), WarHelmets.Get(), Shields.Get(), Weapons.Get(), Faces.Get(), Dust.Get()}) Part->SetupAttachment(Ground);
    Effects = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("BattleEffects"));
    Effects->SetupAttachment(Ground); Effects->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Effects->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Art/Materials/M_BattleFX.M_BattleFX")));
}
ATacticalField::~ATacticalField() = default;

void ATacticalField::BeginPlay()
{
    Super::BeginPlay();
    StartTrial();
}
void ATacticalField::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    StartTrial();
}
void ATacticalField::StartTrial()
{
    State->Realtime.Reset();State->bRealtimeInBattle=false;State->bRealtimeStepping=false;
    State->ChainSignal.Cancel();State->SignalRead={};
    State->BattleWatch.Reset();State->OrderReads={};State->bWatchPending=State->bWatchPauseAfterPlayback=false;
    State->bRealtimeChainRequested=false;State->bRealtimePauseAfterPlayback=false;State->RealtimeRoutes.Reset();
    State->TerrainSnapshot.Reset();State->BackdropSnapshot.Reset();
    State->Beats.Reset(); State->BeatIndex = 0; State->BeatTime = 0;
    if(bLegacyJourney) State->Journey.Reset();else State->Journey.ResetCampaign();
    if(!bStartOnStandardDifficulty) State->Journey.SelectDifficulty(frontline::JourneyDifficulty::Story);
    State->Journey.mission = FMath::Clamp(FirstMission, 0, 2);
    State->Journey.enemyPolicy = FMath::Clamp(EnemyPolicy, 0, 2);
    State->Journey.promise = bStartWithCouncil ? frontline::JourneyPromise::EscortRefugees : frontline::JourneyPromise::OpenRoad;
    State->DialogueIndex = 0; State->PreparationChoice = 0; State->BeforeBattle.reset();
    State->Interlocutor = 0; State->bConfirmWithdrawal = false;
    if (!bStartWithCouncil)
    {
        State->Journey.Meet();
        State->BeforeBattle = std::make_unique<frontline::StrategistJourney>(State->Journey);
        State->Journey.Prepare(0, State->Journey.enemyPolicy);
    }
    State->Orders.clear();
    State->SelectedTactics = {frontline::ChainTactic::Lure, frontline::ChainTactic::Ambush, frontline::ChainTactic::Breakthrough};
    State->Selected = 0; State->Target = 3; State->Input = ETacticalInput::Select; State->bConfirmEndTurn = false; State->Report = JourneyText(State->Journey.message);
    Refresh();
}

FVector ATacticalField::TileLocation(int Tile)
{
    const auto Center = hexgrid::Center(Tile % frontline::Width, Tile / frontline::Width);
    return FVector(Center.first * 200, Center.second * 200, 0);
}

void ATacticalField::Refresh()
{
    auto& J = State->Journey;
    RetargetObservedEnemy();
    SyncRealtime();
    SyncChainSignal();
    UpdateLandscapeTopology();
    BuildPresentation();
    UpdateRealtimePresentation();
    if (!IsPlaying()) State->Units = ObserveUnits();
    Markers->ClearInstances();
    BuildTerrain();
    if (FoliageMesh) Trees->SetStaticMesh(FoliageMesh);
    if (RockMesh) Rocks->SetStaticMesh(RockMesh);
    if (RockMaterial) Rocks->SetMaterial(0,RockMaterial);
    if (ArmourMesh) Armour->SetStaticMesh(ArmourMesh);
    if (WarHelmetMesh) WarHelmets->SetStaticMesh(WarHelmetMesh);
    if (ShieldMesh) Shields->SetStaticMesh(ShieldMesh);
    if (GrassMesh) Grass->SetStaticMesh(GrassMesh);
    if (GateRoofMesh) { Roofs->SetStaticMesh(GateRoofMesh);CityRoofs->SetStaticMesh(GateRoofMesh); }
    if(FortressMaterial) CityWalls->SetMaterial(0,FortressMaterial);
    if (SpearheadMesh) Spearheads->SetStaticMesh(SpearheadMesh);
    if (CartWheelMesh) CartWheels->SetStaticMesh(CartWheelMesh);
    if (StandardMaterial) Standards->SetMaterial(0,StandardMaterial);
    if (AllyMaterial) Soldiers->SetMaterial(0, AllyMaterial);
    if (EnemyMaterial) Enemies->SetMaterial(0, EnemyMaterial);
    if (PalisadeMesh) Palisades->SetStaticMesh(PalisadeMesh);
    if (WatchtowerMesh) Watchtowers->SetStaticMesh(WatchtowerMesh);
    if (TentMesh) Tents->SetStaticMesh(TentMesh);
    if (SupplyCrateMesh) SupplyCrates->SetStaticMesh(SupplyCrateMesh);
    if (RockMaterial) Stonework->SetMaterial(0,RockMaterial);
    if (StandardMaterial) FieldBanners->SetMaterial(0,StandardMaterial);
    if (ObjectiveMaterial) Markers->SetMaterial(0, ObjectiveMaterial);
    RenderUnits(); RenderEffects();
    CheckpointJourney();
    // Objective gateways and route furniture are built with the terrain.
    auto DisplayRoutes=State->HUD.Routes;
    if (bRealtimeCommand) DisplayRoutes.Append(State->RealtimeRoutes);
    if (!IsPlaying() || bRealtimeCommand) for (const auto& Route : DisplayRoutes)
    {
        const FVector From = SurfaceLocation(Route.Key) + FVector(0, 0, 15);
        const FVector To = SurfaceLocation(Route.Value) + FVector(0, 0, 15);
        Markers->AddInstance(FTransform((To - From).Rotation(), (From + To) * .5, FVector((To - From).Size() / 100, .065, .05)));
        const FVector Back = (From - To).GetSafeNormal() * 30;
        for (double Sign : {-1., 1.})
        {
            const FVector End = To + Back + FVector(-Back.Y, Back.X, 0) * Sign * .5;
            Markers->AddInstance(FTransform((End - To).Rotation(), (To + End) * .5, FVector((End - To).Size() / 100, .065, .05)));
        }
    }

}
