#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "TacticalPresentation.h"
#include "TacticalActivity.h"
#include <vector>
#include <cstdint>
#include "TacticalTrial.generated.h"

class UProceduralMeshComponent;
class UInstancedStaticMeshComponent;
class ACameraActor;
struct FJourneyState;
struct FChainReview;
namespace frontline { struct JourneyWatchSnapshot; }
namespace TacticalMap { struct FSurface; }
struct FJourneyDeleter
{
    void operator()(FJourneyState* Value) const;
};

UCLASS()
class GUNYUTACTICS_API ATacticalField : public AActor
{
    GENERATED_BODY()
public:
    ATacticalField();
    virtual ~ATacticalField() override;
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;
    UPROPERTY(EditAnywhere, Category="Trial|Scenario", meta=(ClampMin="0", ClampMax="2"))
    int32 FirstMission = 0;
    UPROPERTY(EditAnywhere, Category="Trial|Scenario", meta=(ClampMin="0", ClampMax="2"))
    int32 EnemyPolicy = 0;
    UPROPERTY(EditAnywhere, Category="Trial|Scenario")
    bool bStartWithCouncil = true;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UMaterialInterface> TerrainMaterial;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UMaterialInterface> WaterMaterial;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UMaterialInterface> FortressMaterial;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UMaterialInterface> DistantMaterial;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UMaterialInterface> AllyMaterial;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UMaterialInterface> EnemyMaterial;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UMaterialInterface> FoliageMaterial;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UMaterialInterface> ObjectiveMaterial;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> FoliageMesh;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> RockMesh;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> ArmourMesh;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> WarHelmetMesh;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> ShieldMesh;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> GrassMesh;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> GateRoofMesh;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> SpearheadMesh;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> CartWheelMesh;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UMaterialInterface> StandardMaterial;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UMaterialInterface> RockMaterial;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> PalisadeMesh;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> WatchtowerMesh;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> TentMesh;
    UPROPERTY(EditAnywhere, Category="Trial|Art")
    TObjectPtr<class UStaticMesh> SupplyCrateMesh;
    void SelectAt(int Tile);
    void CommandAt(int Tile);
    void AppendWaypointAt(int Tile);
    void Action(FKey Key);
    void Issue(ETacticalCommand Command);
    bool CanIssue(ETacticalCommand Command) const;
    FString TacticReservationBlockReason() const;
    void SelectOfficer(int Index);
    void SelectTactic(int Id);
    void EditOrder(int Index, int Direction);
    void ClickTile(int Tile);
    const FTacticalPresentation& Presentation() const;
    FString Summary() const;
    FString Detail() const;
    bool SmokeTest();
    bool PlaybackSmokeTest();
    bool JourneySmokeTest();
    bool StoryPromiseSmokeTest();
    bool TacticRepertoireSmokeTest();
    bool ComboSmokeTest();
    bool TacticAvailabilitySmokeTest();
    void PrepareTacticAvailabilityPreview(bool bUsedSignature);
    void PrepareBattleStoryPreview(bool bCostly);
    bool EnemyReadSmokeTest();
    bool VisualGeometrySmokeTest();
    bool MapDetailSmokeTest();
    bool BackdropSmokeTest();
    bool InfantryMotionSmokeTest();
    bool ActivityReadSmokeTest();
    bool SpectacleSmokeTest();
    bool ChainReviewSmokeTest();
    bool HasChainReview() const;
    FChainReview ReadChainReview() const;
    bool RealtimeSmokeTest();
    bool ChainSignalSmokeTest();
    bool BattleWatchSmokeTest();
    bool MarchPlanSmokeTest();
    bool OpeningUpgradeSmokeTest();
    bool SaveSessionSmokeTest();
    bool ChainPresentationSmokeTest();
    bool TacticCinemaSmokeTest();
    bool ExportSession(std::vector<std::uint8_t>&) const;
    bool ImportSession(const std::vector<std::uint8_t>&);
    void CheckpointJourney(bool bForce=false);
    void PrepareOpeningPreview(bool bDebrief);
    void PrepareMarchPreview();
    void PrepareBattleWatchPreview();
    void AdvanceRealtime(float Delta);
    bool RealtimePaused() const;
    void ToggleThreatOverlay();
    void AdvancePresentation(float Delta);
    bool IsPlaying() const;
    const TArray<FVisualUnit>& VisualUnits() const;
    const FBattleBeat* CurrentBeat() const;
    float BeatProgress() const;
    FString CueTitle() const;
    FString CueReason() const;
    FUnitActivity ReadUnitActivity(int Id) const;
    FTacticCinematicRead ReadTacticCinema() const;
    static FVector TileLocation(int Tile);
    float SurfaceHeight(FVector Point) const;
    FVector SurfaceLocation(int Tile) const;
    int PickSurfaceTile(FVector Origin, FVector Direction) const;
    UPROPERTY(EditAnywhere, Category="Trial|Presentation", meta=(ClampMin="0.5", ClampMax="2.0"))
    float PlaybackSpeed = 1.0f;
    UPROPERTY(EditAnywhere, Category="Trial|Presentation", meta=(ClampMin="0", ClampMax="1"))
    float EffectsVolume = 0.45f;
    UPROPERTY(EditAnywhere, Category="Trial|Presentation")
    bool bReadableFoliage = true;
    UPROPERTY(EditAnywhere, Category="Trial|Scenario")
    bool bRealtimeCommand = false;
    UPROPERTY(EditAnywhere, Category="Trial|Scenario")
    bool bPauseOnBattleAlert = true;
private:
    TUniquePtr<FJourneyState, FJourneyDeleter> State;
    UPROPERTY() TObjectPtr<UProceduralMeshComponent> Ground;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Soldiers;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Enemies;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Trees;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Trunks;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Rocks;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Grass;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Buildings;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Roofs;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Houses;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Palisades;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Watchtowers;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Tents;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> SupplyCrates;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Stonework;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> CityWalls;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> CityTimber;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> CityRoofs;
    UPROPERTY() TObjectPtr<UProceduralMeshComponent> FieldBanners;
    UPROPERTY(Transient) TObjectPtr<class UMaterialInstanceDynamic> FoliageInstance;
    TWeakObjectPtr<class UMaterialInterface> FoliageInstanceSource;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Markers;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Helmets;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Armour;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> WarHelmets;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Shields;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Weapons;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Faces;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Dust;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Limbs;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Spearheads;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> CartWheels;
    UPROPERTY() TObjectPtr<UProceduralMeshComponent> Standards;
    UPROPERTY() TObjectPtr<UProceduralMeshComponent> Effects;
    UPROPERTY() TObjectPtr<class UAudioComponent> CueAudio;
    TArray<FVisualUnit> ObserveUnits() const;
    void QueueAction(EBattleCue Kind, int Actor, int Target, const TArray<FVisualUnit>& Before);
    void QueueChain(const TArray<FVisualUnit>& Before);
    void DispatchQueuedChain();
    void BeginPlayback();
    void FinishPlayback();
    void RenderUnits();
    FString ReadPlaybackReport() const;
    void RenderStandards();
    void BuildTerrain();
    void BuildMapScenery();
    void BuildFortressBackdrop();
    void BuildVillageBackdrop();
    void BuildRiverPort();
    void BuildSceneryWater();
    void BuildMountainBackdrop();
    void BuildBackdropGround();
    float BackdropGroundHeight(FVector P) const;
    float SceneryFloor(FVector P) const;
    void BuildFortifications();
    void UpdateFoliageReadability();
    void BuildMapWater();
    void UpdateRoadTopology();
    float RoadDistance(FVector Point) const;
    void BuildRoadRuts(TacticalMap::FSurface& Land);
    void UpdateLandscapeTopology();
    void RenderEffects();
    void PlayCueSound(bool bImpact);
    void Refresh();
    void StartTrial();
    bool IsRealtimeAction(ETacticalCommand Command) const;
    bool CanRealtimeIssue(ETacticalCommand Command) const;
    void IssueRealtime(ETacticalCommand Command);
    void RunRealtimeCycle(bool bSingleStep);
    void SyncRealtime();
    void UpdateRealtimePresentation();
    void RecordBattleWatch(const frontline::JourneyWatchSnapshot& Before);
    void PublishBattleNotices();
    void BuildBattleWatchPresentation();
    void SyncChainSignal();
    void IssueChainSignal();
    bool DispatchReadySignal();
    bool SignalHasPendingMarch() const;
    void BuildMarchPresentation();
    void IssueMarchPlan(ETacticalCommand Command);
    void BuildPresentation();
    void BuildTacticsPresentation();
    void BuildTacticAvailability();
    void RetargetObservedEnemy();
    void BuildEnemyPresentation();
    void BuildJourneyPresentation();
    void BuildOfficerConversation();
    void BuildStoryPresentation();
    void BuildEpisodePresentation();
    void BuildPreparationPresentation();
    void BuildPreparationMap();
    bool BattleStorySmokeTest();
    void BuildOpeningPresentation();
    void IssueJourney(ETacticalCommand Command);
};

UCLASS()
class GUNYUTACTICS_API ATacticalController : public APlayerController
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual void PlayerTick(float Delta) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY() TObjectPtr<ATacticalField> Field;
    UPROPERTY() TObjectPtr<ACameraActor> BattleCamera;
    TSharedPtr<class SWidget> Overlay;
    TArray<TSharedPtr<class SWidget>> InputPanels;
    UPROPERTY() TObjectPtr<class UTexture2D> PortraitSheet;
    TArray<TSharedPtr<struct FSlateBrush>> PortraitBrushes,FaceBrushes;
    void BuildInterface();
    void AddBattlePanel(TSharedRef<class SCanvas>,FVector2D,FVector2D,TSharedRef<class SWidget>,float Inset=12);
    void BuildMapLabels(TSharedRef<class SCanvas>);
    void BuildBattleHeader(TSharedRef<class SCanvas>);
    void BuildCameraInterface(TSharedRef<class SCanvas>);
    void BuildTacticCinema(TSharedRef<class SCanvas>);
    void BuildTacticRelay(TSharedRef<class SCanvas>);
    void UpdateTacticLens();
    FVector2D TacticResultPosition() const;
    void ApplyTacticCamera(FVector& ViewFocus,float& ViewRadius,float& ViewTilt) const;
    bool bTacticCinema=true;
    TSharedPtr<class SWidget> CinematicIntro,CinematicResult,CinematicRelay;
    UPROPERTY() TArray<TObjectPtr<class UTexture2D>> CutinTextures;
    TArray<TSharedPtr<struct FSlateBrush>> CutinBrushes;
    void LoadCutinPortraits();
    TSharedRef<class SWidget> CutinPortraitWidget(TFunction<int()> Id,float Width,float Height);
    void UpdateBattleCamera(float Delta,bool bBlocked);
    void CycleCameraView();
    void FocusSelectedOfficer();
    void ResetBattleCamera();
    bool CameraPresentationSmokeTest();
    FVector CameraOffset(float Radius) const;
    void LayoutMapLabels();
    FString UnitMapStatus(int Index) const;
    void SelectMapUnit(int Index);
    TArray<FVector2D> UnitLabelPositions;
    TArray<TPair<FVector2D,FVector2D>> UnitLabelLeaders;
    uint64 LabelLayoutFrame=MAX_uint64;
    TSharedRef<class SWidget> MapLabelLeaderOverlay();
    TSharedRef<class SWidget> BattleActivityOverlay();
    void BuildRosterInterface(TSharedRef<class SCanvas>);
    void BuildCommandInterface(TSharedRef<class SCanvas>);
    TSharedRef<class SWidget> ChainPlannerWidget();
    TSharedRef<class SWidget> ChainPlanOverlay();
    FString UnitChainNumber(int Index) const;
    void BuildBattleFeed(TSharedRef<class SCanvas>);
    bool bEnemyDetailsOpen=false;
    void BuildChainReviewInterface(TSharedRef<class SCanvas> Canvas);
    TSharedRef<class SWidget> ChainReviewRow(int Index);
    void ToggleChainReview();
    void CloseChainReview();
    void UpdateChainReviewInput();
    bool ChainReviewInterfaceSmokeTest();
    void PrepareChainReviewCapture();
    bool bChainReviewOpen=false;
    TSharedPtr<FChainReview> ChainReviewSnapshot;
    void BuildSaveInterface(TSharedRef<class SCanvas> Canvas);
    void ToggleSaveMenu();
    void RefreshSaveSlots();
    void RequestSaveAction(int Slot,bool bLoad);
    void ConfirmSaveAction();
    bool bSaveMenuOpen=false;
    int PendingSaveSlot=-1;
    bool bPendingLoad=false;
    FString SaveStatus;
    TArray<FString> SaveSlotLabels;
    TArray<bool> SaveSlotAvailable,SaveSlotExists;
    void BuildRealtimeInterface(TSharedRef<class SCanvas> Canvas);
    void BuildBattleWatchInterface(TSharedRef<class SCanvas> Canvas);
    void BuildMarchInterface(TSharedRef<class SCanvas> Canvas);
    void ToggleMarchPlan();
    void FocusMarchPoint(int Index);
    TSharedRef<class SWidget> MarchRouteOverlay(bool bConvoy=false);
    TSharedRef<class SWidget> OpeningRouteChoices();
    void ToggleBattleLog();
    void FocusBattleNotice(int Index);
    void BuildJourneyInterface(TSharedRef<class SCanvas> Canvas);
    TSharedRef<class SWidget> StoryConversationWidget();
    void BuildEpisodeInterface(TSharedRef<class SCanvas> Canvas);
    void LoadNovelArt();
    TSharedRef<class SWidget> NovelBackdropWidget(TFunction<bool()> Night);
    TSharedRef<class SWidget> NovelPortraitWidget(TFunction<int()> Speaker,float Width,float Height);
    void BuildNovelHistory(TSharedRef<class SCanvas> Canvas);
    void BuildNovelPlan(TSharedRef<class SCanvas> Canvas);
    void OpenNovelPlan();
    TSharedRef<class SWidget> BuildPlanningMap();
    void TogglePlanningMap();
    void SelectPlanningTile(int Tile);
    void TogglePlanningRoute();
    FString PlanningTitle() const;
    FString PlanningDescription() const;
    FString PlanningHint(int Officer) const;
    bool PlanningMapSmokeTest();
    bool bNovelMap=false;
    int PlanningTile=-1;
    TSharedPtr<class SPlanningMap> PlanningWidget;
    void SelectNovelPlan(int Index);
    void DeployNovelPlan();
    bool NovelFlowSmokeTest();
    bool PreparationReadSmokeTest();
    bool bNovelAdvice=false;
    FLinearColor NovelPortraitTone(int Speaker) const;
    const struct FSlateBrush* NovelPortraitBrush(int Speaker) const;
    UPROPERTY() TArray<TObjectPtr<class UTexture2D>> ConversationTextures;
    TArray<TSharedPtr<struct FSlateBrush>> ConversationBrushes;
    bool bNovelPlan=false,bNovelPersonal=false;
    float NovelStageTime=0,NovelLineTime=0;
    int NovelPreviousSpeaker=-1;
    void AdvanceNovel(float Delta);
    void ReadNextNovel();
    void OpenOfficerNovel();
    void ChooseNovelReply(bool Preserve);
    FString VisibleNovelText() const;
    static FString SafeNovelPrefix(const FString& Text,int Count);
    bool NovelTextComplete() const;
    FString NovelHistoryText(int Index) const;
    bool NovelInterfaceSmokeTest();
    int NovelOfficer() const;
    UPROPERTY() TArray<TObjectPtr<class UTexture2D>> NovelBackgrounds;
    TArray<TSharedPtr<struct FSlateBrush>> NovelBackgroundBrushes;
    bool bNovelHistory=false,bNovelInstant=false;
    float NovelElapsed=0;
    void OpenEpisode();
    void CloseEpisode();
    void StepEpisode(int Direction);
    bool UpdateEpisodeInput();
    void HandleEpisodeKey(FKey Key);
    bool EpisodeInterfaceSmokeTest();
    FDialoguePresentation CurrentEpisodeLine() const;
    FJourneyEpisodePresentation EpisodeSnapshot;
    FString EpisodeSeenKey;
    int EpisodePage=0;
    bool bEpisodeOpen=false;
    void BuildEncounterInterface(TSharedRef<class SCanvas> Canvas);
    void BuildTacticInterface(TSharedRef<class SCanvas> Canvas);
    void BuildEnemyInterface(TSharedRef<class SCanvas> Canvas);
    TSharedRef<class SWidget> TacticPickerButton();
    TSharedRef<class SWidget> PortraitWidget(TFunction<int()> Id, float Width, float Height);
    bool IsPointerOverInterface() const;
    FVector2D LabelPosition(int Index) const;
    FVector2D MapLabelPosition(FVector World, FVector2D Size) const;
    FVector Focus = FVector(2250, 1800, 0);
    FVector BeforeVistaFocus=FVector(2250,1800,0);
    float Distance = 2800,ViewDistance=2800;
    float CameraYaw=90,CameraElevation=48.f;
    int CameraPreset=0, VistaMission=-1;
    bool bOrbitDragging=false;
    FVector2D PreviousOrbitCursor;
    int Frames = 0;
    double FrameSeconds = 0;
    int FrameSamples = 0;
    bool bTacticPickerOpen = false;
    bool bInterfaceVisible = true;
    bool bBattleLogOpen = false;
    bool bMarchPlanOpen = false;
    int IgnoreMapInputThroughFrame = -1;
    bool bCapture = false;
    bool bSmoke = false;
    int PickTile() const;
};

UCLASS()
class GUNYUTACTICS_API ATacticalGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ATacticalGameMode();
};
