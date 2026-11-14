#include "TacticalTrial.h"
#include "Camera/CameraActor.h"
#include "TacticalMissionResponse.h"
#include "TacticalContributionQA.h"
#include "TacticalSupportQA.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#if WITH_EDITOR
#include "AssetCompilingManager.h"
#endif
#include "Engine/TextureRenderTarget2D.h"
#include "HighResScreenshot.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "Misc/App.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "StrategistJourney.hpp"
#include "JourneySave.hpp"
#include "GameFramework/GameUserSettings.h"
#include "Widgets/SWindow.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "HAL/IConsoleManager.h"

ATacticalGameMode::ATacticalGameMode()
{
    PlayerControllerClass = ATacticalController::StaticClass();
    DefaultPawnClass = nullptr;
}

void ATacticalController::BeginPlay()
{
    Super::BeginPlay();
    bShowMouseCursor = true;
    SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
    for (TActorIterator<ATacticalField> It(GetWorld()); It; ++It) { Field = *It; break; }
    if (!Field) Field = GetWorld()->SpawnActor<ATacticalField>();
    BattleCamera = GetWorld()->SpawnActor<ACameraActor>();
    BattleCamera->GetCameraComponent()->SetFieldOfView(55);
    auto& Exposure = BattleCamera->GetCameraComponent()->PostProcessSettings;
    Exposure.bOverride_AutoExposureBias = true;
    Exposure.AutoExposureBias = .25f;
    Exposure.bOverride_VignetteIntensity = true;
    Exposure.VignetteIntensity = .22f;
    Exposure.bOverride_BloomIntensity = true;
    Exposure.BloomIntensity = .35f;
    Exposure.bOverride_ColorSaturation = true;
    Exposure.ColorSaturation = FVector4(.95f,.98f,.95f,1.f);
    Exposure.bOverride_ColorContrast = true;
    Exposure.ColorContrast = FVector4(1.06f,1.06f,1.06f,1.f);
    Exposure.bOverride_AmbientOcclusionIntensity = true;
    Exposure.AmbientOcclusionIntensity = .8f;
    Exposure.bOverride_AmbientOcclusionRadius = true;
    Exposure.AmbientOcclusionRadius = 45.f;
    SetViewTarget(BattleCamera);
    PrimaryActorTick.bCanEverTick = true;
    bCapture = FParse::Param(FCommandLine::Get(), TEXT("GunyuCapture"));
    if (bCapture && FParse::Param(FCommandLine::Get(), TEXT("GunyuClose")))
    { Focus = FVector(1950, 1810, 0); Distance = 1700; }
    if (FParse::Param(FCommandLine::Get(),TEXT("GunyuMapOverview")))
    { Focus=FVector(1600,1350,0); Distance=4500; }
    if (bCapture && FParse::Param(FCommandLine::Get(),TEXT("GunyuRiverClose")))
    { Focus=FVector(2200,1030,0); Distance=2200; }
    if (bCapture && FParse::Param(FCommandLine::Get(),TEXT("GunyuCampClose")))
    { Focus=ATacticalField::TileLocation(frontline::Campaign::At(2,10));Distance=1200; }
    if (bCapture && FParse::Param(FCommandLine::Get(),TEXT("GunyuGateClose")))
    { Focus=ATacticalField::TileLocation(frontline::Campaign::At(12,10));Distance=1400; }
    if(bCapture && FParse::Param(FCommandLine::Get(),TEXT("GunyuBackdrop")))
    { BeforeVistaFocus=Focus;Focus=FVector(2100,650,0);Distance=5500;CameraYaw=90;CameraElevation=32;CameraPreset=3; }
    if (bCapture && FParse::Param(FCommandLine::Get(),TEXT("GunyuDenseForest"))) Field->bReadableFoliage=false;
    bSmoke = FParse::Param(FCommandLine::Get(), TEXT("GunyuSmoke"));
    if(bSmoke) { Field->bStartOnStandardDifficulty=true;Field->bLegacyJourney=true;Field->Issue(ETacticalCommand::Restart); }
    bTacticCinema=!FParse::Param(FCommandLine::Get(),TEXT("GunyuNoTacticCinema"));
    // Respect editor PIE and explicit capture/windowed runs. Standalone play
    // opens borderless at the desktop resolution, including existing configs.
    if (!bCapture && !bSmoke && !GetWorld()->IsPlayInEditor() && !FParse::Param(FCommandLine::Get(), TEXT("windowed")))
    {
        auto* Settings = GEngine->GetGameUserSettings();
        Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
        Settings->SetScreenResolution(Settings->GetDesktopResolution());
        Settings->ApplyResolutionSettings(false);
        Settings->SaveSettings();
        UE_LOG(LogTemp, Display, TEXT("GUNYU_FULLSCREEN: mode=%d desktop=%dx%d"), static_cast<int>(Settings->GetFullscreenMode()), Settings->GetScreenResolution().X, Settings->GetScreenResolution().Y);
    }
    ViewDistance=Distance;
    bDetailedBattle=bSmoke || (bCapture && !FParse::Param(FCommandLine::Get(),TEXT("GunyuSimple")));
    BuildInterface();
}

int ATacticalController::PickTile() const
{
    FVector Origin, Direction;
    if (!DeprojectMousePositionToWorld(Origin, Direction) || Direction.Z >= -.001) return -1;
    return Field ? Field->PickSurfaceTile(Origin, Direction) : -1;
}

void ATacticalController::PlayerTick(float Delta)
{
    Super::PlayerTick(Delta);
    if (!Field || !BattleCamera) return;
    AdvanceNovel(Delta);
    const bool bEpisodeInputBlocked=UpdateEpisodeInput();
    const bool bReviewInputBlocked=bEpisodeInputBlocked || bChainReviewOpen || WasInputKeyJustPressed(EKeys::Y);
    if(!bEpisodeInputBlocked) UpdateChainReviewInput();
    const bool bModalInputBlocked=bSaveMenuOpen || bReviewInputBlocked || WasInputKeyJustPressed(EKeys::F6);
    if(WasInputKeyJustPressed(EKeys::F6)) ToggleSaveMenu();
    if(bSaveMenuOpen && WasInputKeyJustPressed(EKeys::Escape)) ToggleSaveMenu();
    if(bSaveMenuOpen && Field->bRealtimeCommand && !Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
    if (!bModalInputBlocked && !bMarchPlanOpen && !bBattleLogOpen && WasInputKeyJustPressed(EKeys::Tab)) bInterfaceVisible = !bInterfaceVisible;
    if (!bModalInputBlocked && !bBattleLogOpen && !bMarchPlanOpen && WasInputKeyJustPressed(EKeys::I)) bEnemyDetailsOpen=!bEnemyDetailsOpen;
    if (!bModalInputBlocked && WasInputKeyJustPressed(EKeys::L)) ToggleBattleLog();
    if (!bModalInputBlocked && WasInputKeyJustPressed(EKeys::V)) ToggleMarchPlan();
    if (bMarchPlanOpen && (!Field->bRealtimeCommand || !Field->Presentation().bBattle || Field->IsPlaying() || WasInputKeyJustPressed(EKeys::Escape))) bMarchPlanOpen=false;
    if (bBattleLogOpen && (!Field->bRealtimeCommand || !Field->Presentation().bBattle || WasInputKeyJustPressed(EKeys::Escape))) bBattleLogOpen=false;
    if (bBattleLogOpen && !Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
    if (bMarchPlanOpen && !Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
    if(Field->Presentation().Position.bVisible)
    {
        bEnemyDetailsOpen=false;
        if(bMarchPlanOpen || bBattleLogOpen) Field->Issue(ETacticalCommand::Cancel);
    }
    const bool bSignatureCapture = bCapture && FParse::Param(FCommandLine::Get(), TEXT("GunyuSignature"));
    const bool bSoloCapture=bCapture && FParse::Param(FCommandLine::Get(),TEXT("GunyuSolo"));
    const bool bWeaponCapture=bCapture && FParse::Param(FCommandLine::Get(),TEXT("GunyuWeaponCapture"));
    const bool bCinemaCapture = bCapture && (FParse::Param(FCommandLine::Get(), TEXT("GunyuCinema")) || bSignatureCapture || bSoloCapture || bWeaponCapture);
    const bool bSignalCapture=bCapture && FParse::Param(FCommandLine::Get(),TEXT("GunyuChainSignal"));
    const bool bDecisionCapture=bCapture && FParse::Param(FCommandLine::Get(),TEXT("GunyuDecision"));
    int SupportPreviewMode=0;const bool bSupportActionCapture=bCapture && FParse::Value(FCommandLine::Get(),TEXT("SupportActionPreview="),SupportPreviewMode);
    int SupportComboMode=0;const bool bSupportComboCapture=bCapture && FParse::Value(FCommandLine::Get(),TEXT("SupportComboPreview="),SupportComboMode);
    int DefenseMode=0;const bool bDefenseCapture=bCapture && FParse::Value(FCommandLine::Get(),TEXT("DefensePreview="),DefenseMode);
    int DefenseRetryPayoff=0;const bool bRetryPayoffCapture=bCapture && FParse::Value(FCommandLine::Get(),TEXT("DefenseRetryPayoff="),DefenseRetryPayoff);
    int DutyResponse=0;const bool bDutyResponseCapture=bCapture && FParse::Value(FCommandLine::Get(),TEXT("DutyResponse="),DutyResponse);
    if (Field->IsPlaying() || Field->Presentation().Journey.bVisible || WasInputKeyJustPressed(EKeys::Escape)) bTacticPickerOpen = false;
    if (!bCinemaCapture && !bDecisionCapture && !bSupportActionCapture && !bSupportComboCapture && !bDefenseCapture && !bRetryPayoffCapture && !bDutyResponseCapture)
    {
        if (Field->bRealtimeCommand && !bCapture && !bSmoke && !FApp::HasFocus() && !Field->RealtimePaused())
            Field->Issue(ETacticalCommand::RealtimePause);
        Field->AdvancePresentation(Field->bRealtimeCommand ? FMath::Min(Delta,.25f) : Delta);
        Field->AdvanceRealtime(FMath::Min(Delta,.25f));
    }
    const FKey Commands[] = {EKeys::One, EKeys::Two, EKeys::Three, EKeys::M, EKeys::Escape, EKeys::A, EKeys::G, EKeys::F, EKeys::Q, EKeys::W, EKeys::E, EKeys::T, EKeys::C, EKeys::R, EKeys::SpaceBar, EKeys::Enter, EKeys::N, EKeys::F5, EKeys::Period, EKeys::X, EKeys::B};
    for (const FKey& Key : Commands)
        if (!bModalInputBlocked && !bBattleLogOpen && !bMarchPlanOpen && WasInputKeyJustPressed(Key)) Field->Action(Key);
    if (!bModalInputBlocked && !bBattleLogOpen && !bMarchPlanOpen && !IsInputKeyDown(EKeys::MiddleMouseButton) && Frames > IgnoreMapInputThroughFrame && !IsPointerOverInterface() && WasInputKeyJustPressed(EKeys::LeftMouseButton)) Field->ClickTile(PickTile());
    if (!bModalInputBlocked && !bBattleLogOpen && !bMarchPlanOpen && !IsInputKeyDown(EKeys::MiddleMouseButton) && Frames > IgnoreMapInputThroughFrame && !IsPointerOverInterface() && WasInputKeyJustPressed(EKeys::RightMouseButton))
    {
        if (Field->bRealtimeCommand && (IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift))) Field->AppendWaypointAt(PickTile());
        else Field->CommandAt(PickTile());
    }
    UpdateBattleCamera(Delta,bModalInputBlocked || bBattleLogOpen || bMarchPlanOpen || bTacticPickerOpen);
    ++Frames;
    if (Frames==30 && bCapture && FParse::Param(FCommandLine::Get(),TEXT("GunyuProfile")))
    {
        if (auto* GPUStats=IConsoleManager::Get().FindConsoleVariable(TEXT("r.GPUCsvStatsEnabled"))) GPUStats->Set(1,ECVF_SetByCode);
        FCsvProfiler::Get()->BeginCapture(90,FPaths::ProjectSavedDir()/TEXT("Profiling"),TEXT("GunyuBattle.csv"));
    }
    if (Frames > 30 && Frames <= 120) { FrameSeconds += Delta; ++FrameSamples; }
    if (Frames == 120 && FrameSamples > 0)
        UE_LOG(LogTemp, Display, TEXT("GUNYU_FRAME_PACING: warm frame average %.2f ms (%d frames, includes GPU/presentation waits)"), FrameSeconds * 1000 / FrameSamples, FrameSamples);
    if (Frames == 60 && GEngine && GEngine->GameViewport && GEngine->GameViewport->GetWindow().IsValid())
    {
        int Width, Height; GetViewportSize(Width, Height);
        UE_LOG(LogTemp, Display, TEXT("GUNYU_WINDOW: mode=%d viewport=%dx%d"), static_cast<int>(GEngine->GameViewport->GetWindow()->GetWindowMode()), Width, Height);
    }
    if (Frames == 3 && (bCapture || FParse::Param(FCommandLine::Get(), TEXT("GunyuBattlePreview")) || FParse::Param(FCommandLine::Get(),TEXT("GunyuCouncilPreview"))))
    {
        const bool bDutyHandoffCapture=bCapture && FParse::Param(FCommandLine::Get(),TEXT("GunyuDutyHandoff"));
        // Explicit cinematic QA captures use fixed manual playback; editor map
        // speed/mode preferences must not turn CueTime into a different phase.
        if(bCinemaCapture) { Field->bRealtimeCommand=false;Field->PlaybackSpeed=1; }
        if(bDecisionCapture)
        {
            Field->PlaybackSpeed=1;int ChoicePolicy=-1;
            if(FParse::Value(FCommandLine::Get(),TEXT("ChoicePolicy="),ChoicePolicy))
            { Field->EnemyPolicy=FMath::Clamp(ChoicePolicy,0,2);Field->Issue(ETacticalCommand::Restart); }
        }
        if (FParse::Param(FCommandLine::Get(),TEXT("GunyuCouncilPreview")))
        { Field->bStartWithCouncil=true;Field->FirstMission=0;Field->EnemyPolicy=1;Field->Issue(ETacticalCommand::Restart); }
        if (bDutyHandoffCapture || FParse::Param(FCommandLine::Get(),TEXT("GunyuRealtime")))
        { Field->bRealtimeCommand=true;Field->Issue(ETacticalCommand::Restart); }
        if (bSignalCapture)
        {
            Field->bRealtimeCommand=true;
            Field->EnemyPolicy=FParse::Param(FCommandLine::Get(),TEXT("GunyuSignalReady")) || FParse::Param(FCommandLine::Get(),TEXT("GunyuSignalMotion")) ? 0 : 1;
            Field->Issue(ETacticalCommand::Restart);
        }
        if (bCapture && FParse::Param(FCommandLine::Get(), TEXT("GunyuReadScout")))
        { Field->EnemyPolicy = 2; Field->Issue(ETacticalCommand::Restart); }
        int MapMission=-1;
        if (bCapture && FParse::Value(FCommandLine::Get(),TEXT("MapMission="),MapMission))
        { Field->FirstMission=FMath::Clamp(MapMission,0,2); Field->Issue(ETacticalCommand::Restart); }
        const bool bCouncil = FParse::Param(FCommandLine::Get(), TEXT("GunyuCouncil")) || FParse::Param(FCommandLine::Get(),TEXT("GunyuCouncilPreview"));
        const bool bDebrief = FParse::Param(FCommandLine::Get(), TEXT("GunyuDebrief"));
        const bool bWithdrawal = FParse::Param(FCommandLine::Get(), TEXT("GunyuWithdrawal"));
        const bool bBattle = bCinemaCapture || bDebrief || bWithdrawal || FParse::Param(FCommandLine::Get(), TEXT("GunyuPlan")) || FParse::Param(FCommandLine::Get(), TEXT("GunyuMove")) || FParse::Param(FCommandLine::Get(), TEXT("GunyuBattlePreview")) || FParse::Param(FCommandLine::Get(), TEXT("GunyuTacticMenu")) || FParse::Param(FCommandLine::Get(),TEXT("GunyuDecision"));
        if (bCouncil || bBattle || bDutyHandoffCapture)
            for (int Step = 0; Step < 3; ++Step) Field->Action(EKeys::Enter);
        if (bDebrief) Field->Issue(ETacticalCommand::PromiseRoad);
        if (bDecisionCapture && FParse::Param(FCommandLine::Get(),TEXT("ChoiceRoad"))) Field->Issue(ETacticalCommand::PromiseRoad);
        if (bSignatureCapture || FParse::Param(FCommandLine::Get(),TEXT("GunyuReserve"))) Field->Issue(ETacticalCommand::PlanSupplies);
        if(bDutyHandoffCapture) Field->Issue(ETacticalCommand::PlanExample);
        if (bBattle || bDutyHandoffCapture) Field->Action(EKeys::Enter);
        if(bCapture && bBattle && FParse::Param(FCommandLine::Get(),TEXT("GunyuUseReserve")))
        { Field->Issue(ETacticalCommand::Resupply);Field->Issue(ETacticalCommand::Skip); }
        if (bCapture && bBattle && FParse::Param(FCommandLine::Get(), TEXT("GunyuReadAdvance")))
        { Field->SelectOfficer(0); Field->Issue(ETacticalCommand::Move); Field->CommandAt(frontline::Campaign::At(8,11)); Field->Issue(ETacticalCommand::Skip); }
        if (bCapture && bBattle && FParse::Param(FCommandLine::Get(), TEXT("GunyuReadScout"))) Field->SelectAt(frontline::Campaign::At(7,11));
        if (bDebrief)
        {
            Field->Issue(ETacticalCommand::Sample); Field->Issue(ETacticalCommand::Execute); Field->Issue(ETacticalCommand::Skip);
            Field->Issue(ETacticalCommand::EndTurn); Field->Issue(ETacticalCommand::Skip);
            Field->SelectOfficer(2); Field->Issue(ETacticalCommand::QueueTactic); Field->Issue(ETacticalCommand::Execute); Field->Issue(ETacticalCommand::Skip);
        }
        if (bWithdrawal) { Field->Issue(ETacticalCommand::Withdraw); Field->Issue(ETacticalCommand::Withdraw); Field->Issue(ETacticalCommand::Skip); }
        if (FParse::Param(FCommandLine::Get(), TEXT("GunyuGuan"))) Field->Issue(ETacticalCommand::TalkGuan);
        if (FParse::Param(FCommandLine::Get(), TEXT("GunyuZhang"))) Field->Issue(ETacticalCommand::TalkZhang);
    }
    if (bCapture && !bSignalCapture && Frames==3 && FParse::Param(FCommandLine::Get(),TEXT("GunyuRealtime")))
    {
        Field->SelectOfficer(0);Field->CommandAt(frontline::Campaign::At(10,12));
        Field->SelectOfficer(1);Field->Issue(ETacticalCommand::Hold);
        Field->SelectOfficer(2);Field->SelectAt(frontline::Campaign::At(10,10));Field->Issue(ETacticalCommand::Strike);
        Field->SelectOfficer(0);
        if (FParse::Param(FCommandLine::Get(),TEXT("GunyuRealtimeMotion")))
        {
            Field->Issue(ETacticalCommand::RealtimePause);Field->AdvanceRealtime(12.f);
            Field->AdvancePresentation(1.3f);Field->Issue(ETacticalCommand::RealtimePause);
        }
    }
    if(bCapture && Frames==4)
    {
        int FieldPreview=-1;
        if(FParse::Value(FCommandLine::Get(),TEXT("TacticField="),FieldPreview)) Field->PrepareBattlefieldPreview(FieldPreview);
        int DoctrinePreview=-1;
        if(FParse::Value(FCommandLine::Get(),TEXT("EnemyDoctrine="),DoctrinePreview)) Field->PrepareEnemyDoctrinePreview(DoctrinePreview);
    }
    if (bCapture && Frames == 3 && FParse::Param(FCommandLine::Get(), TEXT("GunyuPlan"))) Field->Issue(ETacticalCommand::Sample);
    if(bCapture && Frames==3 && FParse::Param(FCommandLine::Get(),TEXT("GunyuBrokenPlan"))) Field->EditOrder(2,-1);
    if (bSignalCapture && Frames==3)
    {
        if (FParse::Param(FCommandLine::Get(),TEXT("GunyuSignalReady")) || FParse::Param(FCommandLine::Get(),TEXT("GunyuSignalMotion"))) Field->Issue(ETacticalCommand::Sample);
        else
        {
            const frontline::ChainTactic Tactics[]={frontline::ChainTactic::Feint,frontline::ChainTactic::Disrupt,frontline::ChainTactic::Breakthrough};
            for (int Id=0;Id<3;++Id)
            { Field->SelectOfficer(Id);Field->SelectTactic(static_cast<int>(Tactics[Id]));Field->Issue(ETacticalCommand::QueueTactic); }
            Field->SelectOfficer(1);Field->CommandAt(frontline::Campaign::At(9,9));
            Field->SelectOfficer(2);Field->CommandAt(frontline::Campaign::At(9,11));
        }
        Field->SelectOfficer(0);Field->Issue(ETacticalCommand::RealtimeChainSignal);
        if (FParse::Param(FCommandLine::Get(),TEXT("GunyuSignalMotion")))
        {
            Field->Issue(ETacticalCommand::RealtimePause);Field->AdvanceRealtime(.1f);
            Field->AdvancePresentation(1.1f);Field->Issue(ETacticalCommand::RealtimePause);
        }
    }
    if(bCapture && Frames==4 && FParse::Param(FCommandLine::Get(),TEXT("GunyuEnemyDetails"))) bEnemyDetailsOpen=true;
    if (bCapture && Frames == 3 && FParse::Param(FCommandLine::Get(), TEXT("GunyuMove"))) Field->Issue(ETacticalCommand::Move);
    if (bCapture && Frames == 3 && FParse::Param(FCommandLine::Get(), TEXT("GunyuTacticMenu")))
    { Field->SelectOfficer(1); bTacticPickerOpen = true; }
    if (bCapture && Frames == 3 && FParse::Param(FCommandLine::Get(), TEXT("GunyuPhoto"))) bInterfaceVisible = false;
    if (bCapture && Frames==3 && FParse::Param(FCommandLine::Get(),TEXT("GunyuBattleWatch")))
    {
        Field->PrepareBattleWatchPreview();Focus=Field->SurfaceLocation(frontline::Campaign::At(5,4));Distance=2800;
        if (FParse::Param(FCommandLine::Get(),TEXT("GunyuBattleLog"))) ToggleBattleLog();
    }
    if (bCapture && Frames==3 && FParse::Param(FCommandLine::Get(),TEXT("GunyuMarchPlan")))
    {
        Field->PrepareMarchPreview();Focus=Field->SurfaceLocation(frontline::Campaign::At(7,8));Distance=3100;
        if (FParse::Param(FCommandLine::Get(),TEXT("GunyuMarchMenu"))) ToggleMarchPlan();
    }
    if(bCapture && Frames==4 && FParse::Param(FCommandLine::Get(),TEXT("GunyuCinematicView")))
    { CameraPreset=0;CycleCameraView();FocusSelectedOfficer(); }
    if (bCapture && Frames==3 && FParse::Param(FCommandLine::Get(),TEXT("GunyuOpeningCouncil"))) Field->PrepareOpeningPreview(false);
    if (bCapture && Frames==4 && FParse::Param(FCommandLine::Get(),TEXT("GunyuSaveMenu")))
    {
        ToggleSaveMenu();
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuSaveConfirm"))) RequestSaveAction(1,true);
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuSaveResume")))
        {
            RequestSaveAction(1,true);ConfirmSaveAction();ToggleSaveMenu();
            const bool Restored=Field->Presentation().bBattle && Field->RealtimePaused() && Field->Presentation().MarchPoints.Num()==2;
            UE_LOG(LogTemp,Display,TEXT("GUNYU_RESUME_QA: %s (saved battle read by a new process, restored itinerary, paused)"),Restored ? TEXT("PASS") : TEXT("FAIL"));
        }
    }
    if (bCapture && Frames==3 && FParse::Param(FCommandLine::Get(),TEXT("GunyuOpeningReview"))) Field->PrepareOpeningPreview(true);
    if (bCapture && Frames==3 && FParse::Param(FCommandLine::Get(),TEXT("GunyuOpeningField")))
    { Field->PrepareOpeningPreview(false);Field->bRealtimeCommand=true;Field->Issue(ETacticalCommand::Deploy); }
    if (bCinemaCapture && Frames == 3)
    {
        if(bWeaponCapture)
        {
            const int Destinations[]={frontline::Campaign::At(9,11),frontline::Campaign::At(9,9),frontline::Campaign::At(9,10)};
            for(int Id=0;Id<3;++Id)
            { Field->SelectOfficer(Id);Field->Issue(ETacticalCommand::Move);Field->CommandAt(Destinations[Id]);Field->Issue(ETacticalCommand::Skip); }
            int Officer=0;FParse::Value(FCommandLine::Get(),TEXT("CompanyOfficer="),Officer);
            Field->SelectOfficer(FMath::Clamp(Officer,0,2));Field->Issue(ETacticalCommand::Strike);
            const auto* Attack=Field->CurrentBeat();
            if(!Attack || Attack->Kind!=EBattleCue::Strike || Attack->Actor!=FMath::Clamp(Officer,0,2))
            { UE_LOG(LogTemp,Error,TEXT("GUNYU_WEAPON_CAPTURE: FAIL (public move/strike did not produce the requested attack)"));FPlatformMisc::RequestExitWithStatus(false,1);return; }
        }
        else if(bSoloCapture)
        {
            Field->SelectOfficer(0);Field->Issue(ETacticalCommand::Move);Field->CommandAt(frontline::Campaign::At(9,11));Field->Issue(ETacticalCommand::Skip);
            Field->SelectOfficer(2);Field->Issue(ETacticalCommand::Move);Field->CommandAt(frontline::Campaign::At(9,10));Field->Issue(ETacticalCommand::Skip);
            Field->SelectTactic(int(frontline::ChainTactic::Breakthrough));Field->Issue(ETacticalCommand::QueueTactic);
        }
        else if (bSignatureCapture)
        {
            const int Destinations[] = {frontline::Campaign::At(9,11), frontline::Campaign::At(9,9), frontline::Campaign::At(9,10)};
            for (int Id = 0; Id < 3; ++Id)
            { Field->SelectOfficer(Id); Field->Issue(ETacticalCommand::Move); Field->CommandAt(Destinations[Id]); Field->Issue(ETacticalCommand::Skip); }
            const int Officers[] = {0,2,1};
            const frontline::ChainTactic Tactics[] = {frontline::ChainTactic::Oath,frontline::ChainTactic::SerpentRoar,frontline::ChainTactic::AzureDragon};
            for (int I = 0; I < 3; ++I)
            { Field->SelectOfficer(Officers[I]); Field->SelectTactic(static_cast<int>(Tactics[I])); Field->Issue(ETacticalCommand::QueueTactic); }
        }
        else
        {
            Field->Issue(ETacticalCommand::Sample);
            if(FParse::Param(FCommandLine::Get(),TEXT("GunyuCinemaStop")))
            {
                Field->SelectOfficer(2);Field->Issue(ETacticalCommand::Move);Field->CommandAt(frontline::Campaign::At(7,10));Field->Issue(ETacticalCommand::Skip);
                Field->EditOrder(2,-1);
            }
        }
        if(!bWeaponCapture) Field->Issue(ETacticalCommand::Execute);
        float WeaponPhase=.50f;FParse::Value(FCommandLine::Get(),TEXT("WeaponPhase="),WeaponPhase);
        float Time = bWeaponCapture ? Field->CurrentBeat()->Duration*FMath::Clamp(WeaponPhase,0.f,.95f):bSoloCapture ? 1.7f : bSignatureCapture ? 9.5f : 4.7f;
        FParse::Value(FCommandLine::Get(), TEXT("CueTime="), Time);
        Field->AdvancePresentation(Time);
        if(bWeaponCapture) UE_LOG(LogTemp,Display,TEXT("GUNYU_WEAPON_CAPTURE: PASS (public strike actor=%d, progress=%.3f)"),Field->CurrentBeat() ? Field->CurrentBeat()->Actor:-1,Field->BeatProgress());
    }
    if(bCapture && Frames==4 && FParse::Param(FCommandLine::Get(),TEXT("GunyuAuto"))) Field->Issue(ETacticalCommand::AutoBattle);
    if(bCapture && Frames==4 && FParse::Param(FCommandLine::Get(),TEXT("GunyuGoalFocus"))) FocusMissionObjective();
    if(bCapture && Frames==4 && FParse::Param(FCommandLine::Get(),TEXT("GunyuDutyFocus")))
    { Field->SelectOfficer(0);FocusSelectedDuty(); }
    if(bCapture && Frames==3 && FParse::Param(FCommandLine::Get(),TEXT("GunyuDecision")))
    {
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuTripleSpeed")))
        { Field->Issue(ETacticalCommand::RealtimeSpeed);Field->Issue(ETacticalCommand::RealtimeSpeed); }
        int Choice=-1;
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuMissionAfterChain")))
        {
            Field->Issue(ETacticalCommand::Sample);Field->Issue(ETacticalCommand::Execute);Field->Issue(ETacticalCommand::Skip);
            if(FParse::Param(FCommandLine::Get(),TEXT("GunyuMissionAdvance")))
            { Field->Issue(ETacticalCommand::EndTurn);Field->Issue(ETacticalCommand::Skip); }
        }
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuMissionMoment")))
        {
            const bool Guard=FParse::Param(FCommandLine::Get(),TEXT("GunyuMomentGuard"));
            if(Guard)
            {
                Field->SelectOfficer(0);Field->Issue(ETacticalCommand::Move);Field->CommandAt(frontline::Campaign::At(7,6));Field->Issue(ETacticalCommand::Skip);Field->Issue(ETacticalCommand::Hold);Field->Issue(ETacticalCommand::Skip);
            }
            Field->Issue(ETacticalCommand::EndTurn);Field->AdvancePresentation(10);
            if(Guard && FParse::Param(FCommandLine::Get(),TEXT("GunyuMomentLost")))
            {
                Field->SelectOfficer(0);Field->Issue(ETacticalCommand::Move);Field->CommandAt(frontline::Campaign::At(7,7));Field->Issue(ETacticalCommand::Skip);
                Field->Issue(ETacticalCommand::EndTurn);Field->AdvancePresentation(10);
            }
            const auto Response=Field->ReadMissionResponse();
            if(Response.Line.IsEmpty()) { UE_LOG(LogTemp,Error,TEXT("GUNYU_MISSION_MOMENT_CAPTURE: FAIL (actual progress produced no response)"));FPlatformMisc::RequestExitWithStatus(false,1);return; }
            UE_LOG(LogTemp,Display,TEXT("GUNYU_MISSION_MOMENT_CAPTURE: PASS (%s / actor %d / %s)"),*Response.Caption,Response.Actor,*Response.Line);
        }
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuMissionCredit")))
        {
            const bool Relief=FParse::Param(FCommandLine::Get(),TEXT("GunyuCreditRelief"));
            if(Relief)
            {
                Field->PrepareReliefPreview(0);
                if(!Field->CommitRelief()) { UE_LOG(LogTemp,Error,TEXT("GUNYU_MISSION_CREDIT_CAPTURE: FAIL (handover unavailable)"));FPlatformMisc::RequestExitWithStatus(false,1);return; }
                Field->AdvancePresentation(10);Field->Issue(ETacticalCommand::EndTurn);Field->AdvancePresentation(10);
            }
            else
            {
                auto Move=[this](int Id,int Tile) { Field->SelectOfficer(Id);Field->Issue(ETacticalCommand::Move);Field->CommandAt(Tile);Field->Issue(ETacticalCommand::Skip); };
                Field->Issue(ETacticalCommand::EndTurn);Field->AdvancePresentation(10);
                Move(0,frontline::Campaign::At(8,10));Move(2,frontline::Campaign::At(8,11));
                Field->Issue(ETacticalCommand::EndTurn);Field->AdvancePresentation(10);
                Move(2,frontline::Campaign::At(9,10));Field->Issue(ETacticalCommand::EndTurn);Field->AdvancePresentation(10);
            }
            const auto Response=Field->ReadMissionResponse();
            if(Response.Actor!=(Relief ? 0:2) || !Response.Caption.Contains(Relief ? TEXT("→"):TEXT("隊が車道")))
            { UE_LOG(LogTemp,Error,TEXT("GUNYU_MISSION_CREDIT_CAPTURE: FAIL (%s / actor %d)"),*Response.Caption,Response.Actor);FPlatformMisc::RequestExitWithStatus(false,1);return; }
            UE_LOG(LogTemp,Display,TEXT("GUNYU_MISSION_CREDIT_CAPTURE: PASS (%s / %s / %s)"),*Response.Caption,*Response.Line,*Response.Reply);
        }
        if(FParse::Value(FCommandLine::Get(),TEXT("Choice="),Choice))
        {
            if(!Field->UseBattleChoice(Choice,Field->Presentation().BattleChoiceVersion))
            { UE_LOG(LogTemp,Error,TEXT("GUNYU_DECISION_CAPTURE: FAIL (choice unavailable)"));FPlatformMisc::RequestExitWithStatus(false,1);return; }
            float Time=1.7f;FParse::Value(FCommandLine::Get(),TEXT("ChoiceTime="),Time);Field->AdvancePresentation(Time);
            UE_LOG(LogTemp,Display,TEXT("GUNYU_DECISION_CAPTURE: PASS (direct public choice %d, actual playback)"),Choice);
        }
    }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuChoicesOnly")))
    { const bool Passed=Field->BattleChoicesSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuSupportOnly")))
    { const bool Passed=Field->SupportChoicesSmokeTest() && Field->BattleChoicesSmokeTest() && PlanAdviceSmokeTest() && ReliefAdviceSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuSupportActionsOnly")))
    { const bool Passed=gunyumission::SupportSmokeTest() && Field->SupportActionsSmokeTest() && Field->SupportChoicesSmokeTest() && Field->BattleChoicesSmokeTest() && Field->MissionMomentsSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuSupportComboOnly")))
    { const bool Passed=gunyumission::SupportComboSmokeTest() && Field->SupportCombinationSmokeTest() && gunyumission::SupportSmokeTest() && Field->SupportActionsSmokeTest() && Field->SupportChoicesSmokeTest() && Field->BattleChoicesSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuDefenseOnly")))
    { const bool Passed=gunyumission::DefenseSmokeTest() && Field->DefenseSmokeTest() && gunyumission::SupportComboSmokeTest() && Field->SupportCombinationSmokeTest() && Field->MissionMomentsSmokeTest() && MissionFocusSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuDefenseAftermathOnly")))
    { const bool Passed=DefenseAftermathSmokeTest() && Field->DefenseSmokeTest() && CampaignPacingSmokeTest() && ReliefStorySmokeTest() && EpisodeInterfaceSmokeTest() && NovelFlowSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuDutyAftermathOnly")))
    { const bool Passed=DutyAftermathSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuCoalitionOnly")))
    { const bool Passed=CoalitionStorySmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuYangrenOnly")))
    { const bool Passed=YangrenSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuLuoyangOnly")))
    { const bool Passed=LuoyangSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuPlanningOnly")))
    { const bool Passed=PreparationReadSmokeTest() && PlanningMapSmokeTest() && PlanningDutySmokeTest() && PlanningGuideSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuDefenseRetryOnly")))
    { const bool Passed=DefenseRetryPayoffSmokeTest() && DefenseRetrySmokeTest() && Field->DefenseSmokeTest() && ReliefAdviceSmokeTest() && PlanAdviceSmokeTest() && CouncilSceneSmokeTest() && DefenseAftermathSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuTempoOnly")))
    { const bool Passed=Field->RealtimeSmokeTest() && CouncilSceneSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuMissionFocusOnly")))
    { const bool Passed=MissionFocusSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuDutyProgressOnly")))
    { const bool Passed=DutyProgressSmokeTest() && MissionFocusSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuDutyResponseOnly")))
    { const bool Passed=DutyResponseSmokeTest() && Field->MissionMomentsSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuMissionMomentsOnly")))
    { const bool Passed=Field->MissionMomentsSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuMissionCreditOnly")))
    { const bool Passed=Field->MissionCreditSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuResponseOnly")))
    { const bool Passed=Field->TacticCinemaSmokeTest() && Field->BattlefieldTacticsSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuBattlefieldOnly")))
    { const bool Passed=Field->BattlefieldTacticsSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuFieldArtOnly")))
    { const bool Passed=Field->VisualGeometrySmokeTest() && Field->MapDetailSmokeTest() && Field->BackdropSmokeTest() && Field->ActivityReadSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuCompanyOnly")))
    { const bool Passed=Field->InfantryMotionSmokeTest() && CameraPresentationSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuFieldCinemaOnly")))
    { const bool Passed=CameraPresentationSmokeTest() && Field->ActivityReadSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuEnemyDoctrineOnly")))
    { const bool Passed=Field->EnemyDoctrineSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuCouncilSceneOnly")))
    { const bool Passed=CouncilSceneSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuAutoOnly")))
    { const bool Passed=Field->AutoBattleSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuAccessibilityOnly")))
    {
        const bool bPassed=AccessibilitySmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return;
    }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuReliefApproachOnly")))
    { const bool bPassed=Field->ReliefApproachSmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuReliefReadinessOnly")))
    { const bool bPassed=Field->ReliefReadinessSmokeTest() && ReliefInterfaceSmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuReliefAdviceOnly")))
    { const bool bPassed=ReliefAdviceSmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuReliefStoryOnly")))
    { const bool bPassed=ReliefStorySmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if(bSmoke && Frames==2 && (FParse::Param(FCommandLine::Get(),TEXT("GunyuRoleAftermathOnly")) || FParse::Param(FCommandLine::Get(),TEXT("GunyuContributionOnly"))))
    { const bool Passed=NovelFlowSmokeTest() && EpisodeInterfaceSmokeTest() && CampaignPacingSmokeTest() && ReliefStorySmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuReliefOnly")))
    { const bool bPassed=Field->ReliefSmokeTest() && ReliefInterfaceSmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuPositionOnly")))
    { const bool bPassed=Field->TacticPositionSmokeTest() && TacticPositionInterfaceSmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuTargetOnly")))
    { const bool bPassed=Field->TacticTargetSmokeTest() && TacticTargetInterfaceSmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuGuardOnly")))
    { const bool bPassed=Field->GuardControlSmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuPlanAdviceOnly")))
    { const bool bPassed=PlanAdviceSmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuCooperationPlanOnly")))
    { const bool Passed=PlanAdviceSmokeTest() && CouncilSceneSmokeTest() && NovelFlowSmokeTest() && CampaignPacingSmokeTest() && ReliefAdviceSmokeTest();FPlatformMisc::RequestExitWithStatus(false,Passed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuPreparationOnly")))
    { const bool bPassed=PreparationMeaningSmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuCampaignOnly")))
    { const bool bPassed=CampaignPacingSmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if (bSmoke && Frames == 2)
    {
        bool bPassed = Field->AutoBattleSmokeTest();
        if(bPassed) bPassed=Field->SmokeTest();
        if (bPassed)
        {
            Field->PrepareBattleWatchPreview();Field->Issue(ETacticalCommand::RealtimePause);bInterfaceVisible=false;ToggleBattleLog();
            const auto& Items=Field->Presentation().BattleNotices;
            bPassed=bBattleLogOpen && bInterfaceVisible && Field->RealtimePaused() && Items.Num()==2;
            if (bPassed)
            {
                const int Id=Items[0].Army;const FVector Expected=Field->VisualUnits()[Id].Position;
                FocusBattleNotice(0);
                bPassed=!bBattleLogOpen && Focus.Equals(Expected) && Field->Presentation().Selected==Id && Field->RealtimePaused();
            }
            UE_LOG(LogTemp,Display,TEXT("GUNYU_BATTLE_LOG_UI: %s (history opens paused, focus uses current displayed unit, closing retains pause)"),bPassed ? TEXT("PASS") : TEXT("FAIL"));
        }
        if (bPassed)
        {
            Field->PrepareMarchPreview();Field->Issue(ETacticalCommand::RealtimePause);bInterfaceVisible=false;ToggleMarchPlan();
            bPassed=bMarchPlanOpen && bInterfaceVisible && Field->RealtimePaused() && Field->Presentation().MarchPoints.Num()==3;
            if (bPassed)
            {
                const FVector Expected=Field->SurfaceLocation(Field->Presentation().MarchPoints[2].Tile);
                FocusMarchPoint(2);bPassed=!bMarchPlanOpen && Focus.Equals(Expected) && Field->RealtimePaused();
            }
            UE_LOG(LogTemp,Display,TEXT("GUNYU_MARCH_UI: %s (planner opens paused, endpoint focus, closing retains pause)"),bPassed ? TEXT("PASS") : TEXT("FAIL"));
        }
        if(bPassed)
        {
            Field->PrepareOpeningPreview(false);ToggleSaveMenu();
            const auto Before=Field->Summary();
            bPassed=bSaveMenuOpen && bInterfaceVisible && SaveSlotAvailable.Num()==4 && SaveSlotAvailable[1];
            RequestSaveAction(1,true);
            bPassed=bPassed && PendingSaveSlot==1 && Field->Summary()==Before;
            ConfirmSaveAction();
            bPassed=bPassed && bSaveMenuOpen && Field->Presentation().bBattle && Field->RealtimePaused() && Field->Presentation().MarchPoints.Num()==2;
            ToggleSaveMenu();bPassed=bPassed && !bSaveMenuOpen && Field->RealtimePaused();
            UE_LOG(LogTemp,Display,TEXT("GUNYU_SAVE_UI: %s (slots, load confirmation, restored orders, close retains pause)"),bPassed ? TEXT("PASS") : TEXT("FAIL"));
        }
        if(bPassed) bPassed=CameraPresentationSmokeTest();
        if(bPassed) bPassed=ChainReviewInterfaceSmokeTest();
        if(bPassed) bPassed=EpisodeInterfaceSmokeTest();
        if(bPassed) bPassed=NovelInterfaceSmokeTest();
        if(bPassed) bPassed=NovelFlowSmokeTest();
        if(bPassed) bPassed=PreparationReadSmokeTest();
        if(bPassed) bPassed=PreparationMeaningSmokeTest();
        if(bPassed) bPassed=PlanningMapSmokeTest();
        if(bPassed) bPassed=PlanningDutySmokeTest();
        if(bPassed) bPassed=PlanningGuideSmokeTest();
        if(bPassed) bPassed=AccessibilitySmokeTest();
        if(bPassed) bPassed=CampaignPacingSmokeTest();
        if(bPassed) bPassed=CouncilSceneSmokeTest();
        if(bPassed) bPassed=Field->BattlefieldTacticsSmokeTest();
        if(bPassed) bPassed=Field->EnemyDoctrineSmokeTest();
        if(bPassed) bPassed=PlanAdviceSmokeTest();
        if(bPassed) bPassed=Field->GuardControlSmokeTest();
        if(bPassed) bPassed=Field->TacticTargetSmokeTest();
        if(bPassed) bPassed=TacticTargetInterfaceSmokeTest();
        if(bPassed) bPassed=Field->TacticPositionSmokeTest();
        if(bPassed) bPassed=TacticPositionInterfaceSmokeTest();
        if(bPassed) bPassed=Field->ReliefSmokeTest();
        if(bPassed) bPassed=ReliefInterfaceSmokeTest();
        if(bPassed) bPassed=ReliefStorySmokeTest();
        if(bPassed) bPassed=DefenseAftermathSmokeTest();
        if(bPassed) bPassed=DefenseRetrySmokeTest();
        if(bPassed) bPassed=DefenseRetryPayoffSmokeTest();
        if(bPassed) bPassed=ReliefAdviceSmokeTest();
        if(bPassed) bPassed=Field->ReliefReadinessSmokeTest();
        if(bPassed) bPassed=Field->ReliefApproachSmokeTest();
        if(bPassed) bPassed=MissionFocusSmokeTest();
        if(bPassed) bPassed=DutyProgressSmokeTest();
        if(bPassed) bPassed=DutyResponseSmokeTest();
        FPlatformMisc::RequestExitWithStatus(false, bPassed ? 0 : 1);
    }
#if WITH_EDITOR
    if (bCapture && Frames == 120) FAssetCompilingManager::Get().FinishAllCompilation();
#endif
    if(bCapture && Frames==5 && FParse::Param(FCommandLine::Get(),TEXT("GunyuBattleStory"))) Field->PrepareBattleStoryPreview(FParse::Param(FCommandLine::Get(),TEXT("GunyuStoryCostly")));
    if(bCapture && Frames==6 && FParse::Param(FCommandLine::Get(),TEXT("GunyuStoryCouncil")))
    { Field->Issue(ETacticalCommand::StoryReplyCoordinate);Field->Issue(ETacticalCommand::Continue);Field->Issue(ETacticalCommand::TalkZhang); }
    if(bCapture && Frames==5 && FParse::Param(FCommandLine::Get(),TEXT("GunyuTacticStatus"))) Field->PrepareTacticAvailabilityPreview(FParse::Param(FCommandLine::Get(),TEXT("GunyuTacticUsed")));
    if(bCapture && Frames==5 && FParse::Param(FCommandLine::Get(),TEXT("GunyuChainReview"))) PrepareChainReviewCapture();
    if(bCapture && Frames==5 && FParse::Param(FCommandLine::Get(),TEXT("GunyuInfantryClose")))
    {
        const auto* Beat=Field->CurrentBeat();const auto& Units=Field->VisualUnits();
        int Id=Beat && Units.IsValidIndex(Beat->Actor) && Units[Beat->Actor].Id>=0 ? Beat->Actor:0;
        FParse::Value(FCommandLine::Get(),TEXT("CompanyOfficer="),Id);
        if(Units.IsValidIndex(Id) && Units[Id].Id>=0) { Focus=Units[Id].Position;Distance=ViewDistance=850;CameraYaw=125;CameraElevation=36;CameraPreset=-1;bTacticCinema=false; }
    }
    if (bCapture && Frames == 130)
        FScreenshotRequest::RequestScreenshot(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("trial.png")), true, false);
    if(bCapture && Frames==7 && FParse::Param(FCommandLine::Get(),TEXT("GunyuEpisode")))
    {
        int AdviceFixture=0;
        if(FParse::Value(FCommandLine::Get(),TEXT("CouncilAdvice="),AdviceFixture))
        {
            // Presentation-only settled-history fixture, never a claimed combat playthrough.
            std::vector<std::uint8_t> Bytes;auto Fixture=MakeUnique<frontline::JourneySaveData>();
            if(Field->ExportSession(Bytes) && frontline::DecodeJourneySave(Bytes,*Fixture) && Fixture->journey.mission>0 && Fixture->journey.stage==frontline::JourneyStage::Preparation)
            {
                auto& J=Fixture->journey;J.campaignFlow=true;auto& H=J.history[J.mission-1];H={};H.completed=H.won=H.campaignFlow=true;H.survivors=4200;H.review.preparationTracked=true;
                if(AdviceFixture==1) H.review.openingAmbushes=1;
                else if(AdviceFixture==3) { H.survivors=3000;H.casualties=1200; }
                else { H.review.resupplyTurn=2;H.review.postSupplySteps=H.review.tacticSteps=AdviceFixture==2 ? 1:0; }
                if(frontline::EncodeJourneySave(*Fixture,Bytes)) Field->ImportSession(Bytes);
            }
        }
        OpenEpisode();int Page=0;FParse::Value(FCommandLine::Get(),TEXT("EpisodePage="),Page);
        EpisodePage=FMath::Clamp(Page,0,FMath::Max(0,EpisodeSnapshot.Lines.Num()-1));NovelElapsed=1000;
        NovelStageTime=NovelLineTime=1;
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuNovelHistory"))) bNovelHistory=true;
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuNovelPlan"))) OpenNovelPlan();
        int CouncilVoice=-2,CouncilAnswer=0,CouncilLine=0;
        if(FParse::Value(FCommandLine::Get(),TEXT("CouncilVoice="),CouncilVoice))
        {
            OpenNovelPlan();OpenCouncilVoice(CouncilVoice);
            if(FParse::Value(FCommandLine::Get(),TEXT("CouncilAnswer="),CouncilAnswer)) ChooseCouncilDecision(CouncilAnswer);
            if(FParse::Value(FCommandLine::Get(),TEXT("CouncilLine="),CouncilLine)) CouncilPage=FMath::Clamp(CouncilLine,0,CouncilExchange.Lines.Num()-1);
            CouncilElapsed=1000;CouncilStageTime=1;
        }
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuCouncilLedger"))) bCouncilLedger=true;
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuPlanAdvice"))) bNovelAdvice=true;
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuPlanningMap")))
        {
            OpenNovelPlan();TogglePlanningMap();
            if(FParse::Param(FCommandLine::Get(),TEXT("GunyuPlanningSouth")) && !Field->Presentation().Journey.Map.bSouthern) TogglePlanningRoute();
            if(FParse::Param(FCommandLine::Get(),TEXT("GunyuPlanningDuties")))
            {
                bPlanningDuties=true;
                PlanningOfficer=0;SelectPlanningTile(frontline::Campaign::At(7,6));SetPlanningDuty(1);
                PlanningOfficer=1;SelectPlanningTile(frontline::Campaign::At(7,5));SetPlanningDuty(2);
                PlanningOfficer=2;SelectPlanningTile(frontline::Campaign::At(8,9));SetPlanningDuty(1);
            }
            int Site=0;FParse::Value(FCommandLine::Get(),TEXT("PlanningSite="),Site);
            const auto& Sites=Field->Presentation().Journey.Map.Sites;
            if(Sites.IsValidIndex(Site)) SelectPlanningTile(Sites[Site].Tile);
            if(FParse::Param(FCommandLine::Get(),TEXT("GunyuStandardDifficulty"))) Field->Issue(ETacticalCommand::DifficultyStandard);
            if(FParse::Param(FCommandLine::Get(),TEXT("GunyuPlanExample"))) bPlanningExample=true;
            if(FParse::Param(FCommandLine::Get(),TEXT("GunyuApplyExample"))) { bPlanningExample=true;ApplyPlanningExample(); }
            if(FParse::Param(FCommandLine::Get(),TEXT("GunyuGuideEmpty"))) SelectPlanningOfficer(0);
            if(FParse::Param(FCommandLine::Get(),TEXT("GunyuGuidePreview"))) { SelectPlanningOfficer(0);SelectPlanningTile(frontline::Campaign::At(7,6)); }
            if(FParse::Param(FCommandLine::Get(),TEXT("GunyuGuideBlocked"))) { SelectPlanningOfficer(2);SelectPlanningTile(frontline::Campaign::At(9,7)); }
            if(FParse::Param(FCommandLine::Get(),TEXT("GunyuCampaignAftermath")))
            {
                DeployNovelPlan();Field->Issue(ETacticalCommand::Skip);
                std::vector<std::uint8_t> Bytes;auto Settled=MakeUnique<frontline::JourneySaveData>();
                if(Field->ExportSession(Bytes) && frontline::DecodeJourneySave(Bytes,*Settled))
                {
                    Settled->journey.world.armies[3].troops=Settled->journey.world.armies[4].troops=0;
                    Settled->journey.world.armies[0].tile=Settled->journey.goal;
                    Settled->journey.RefreshIntelligence();Settled->journey.EndTurn();
                    if(frontline::EncodeJourneySave(*Settled,Bytes)) { Field->ImportSession(Bytes);CloseEpisode();OpenJourneyEntry(); }
                }
            }
            if(FParse::Param(FCommandLine::Get(),TEXT("GunyuDutyBattle"))) { Field->bRealtimeCommand=true;DeployNovelPlan(); }
        }
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuPlanShortage")))
        {
            std::vector<std::uint8_t> Bytes;auto Snapshot=MakeUnique<frontline::JourneySaveData>();
            if(Field->ExportSession(Bytes) && frontline::DecodeJourneySave(Bytes,*Snapshot))
            {
                auto& Camp=Snapshot->journey.world.cities[0];Camp.food=0;Camp.gold=0;Camp.troops=0;
                if(frontline::EncodeJourneySave(*Snapshot,Bytes)) Field->ImportSession(Bytes);
            }
        }
    }
    int AftermathOfficer=-1;
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("AftermathOfficer="),AftermathOfficer) && Field->Presentation().Journey.bDebrief)
    {
        const ETacticalCommand Talk[]={ETacticalCommand::TalkLiu,ETacticalCommand::TalkGuan,ETacticalCommand::TalkZhang};
        Field->Issue(Talk[FMath::Clamp(AftermathOfficer,0,2)]);OpenOfficerNovel();
        EpisodePage=FMath::Max(0,EpisodeSnapshot.Lines.Num()-3);NovelElapsed=1000;NovelStageTime=NovelLineTime=1;
    }
    if(bCapture && Frames==8 && FParse::Param(FCommandLine::Get(),TEXT("GunyuGuardPreview")))
    {
        Field->PrepareGuardControlPreview(FParse::Param(FCommandLine::Get(),TEXT("GunyuGuardSpent")));CloseEpisode();
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuGuardReleased"))) Field->Issue(ETacticalCommand::ReleaseGuard);
    }
    int TargetPreview=0;
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("TacticTargets="),TargetPreview))
    {
        Field->PrepareTacticTargetPreview(TargetPreview);CloseEpisode();
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuTargetDestination"))) Field->Issue(ETacticalCommand::Move);
    }
    int PositionPreview=0;
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("TacticPosition="),PositionPreview))
    {
        Field->PrepareTacticPositionPreview(PositionPreview);CloseEpisode();bEnemyDetailsOpen=false;
        Focus=Field->VisualUnits()[Field->Presentation().Selected].Position;LabelLayoutFrame=MAX_uint64;
    }
    int ReliefPreview=0;
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("ReliefPreview="),ReliefPreview))
    {
        Field->PrepareReliefPreview(ReliefPreview);CloseEpisode();bEnemyDetailsOpen=false;
        Focus=Field->VisualUnits()[Field->Presentation().Selected].Position;LabelLayoutFrame=MAX_uint64;
        float ReliefBeat=0;
        if(FParse::Value(FCommandLine::Get(),TEXT("ReliefBeat="),ReliefBeat))
        { Field->CommitRelief();Field->AdvancePresentation(ReliefBeat); }
    }
    int ReliefStory=0;
    int CooperationCouncil=0;
    int SupportAction=0;
    int SupportCombo=0;
    int Defense=0;
    int DefenseAftermath=0;
    int DefenseRetry=0;
    int DutyProgress=0,DutyAftermath=0,CoalitionPage=0;
    int BattlefieldLayout=0;
    int LuoyangView=0;
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("LuoyangView="),LuoyangView))
    {
        const bool Valid=PrepareLuoyangPreview(LuoyangView);
        UE_LOG(LogTemp,Display,TEXT("GUNYU_LUOYANG_CAPTURE: %s"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    int YangrenView=0;
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("YangrenView="),YangrenView))
    {
        const bool Valid=PrepareYangrenPreview(YangrenView);
        UE_LOG(LogTemp,Display,TEXT("GUNYU_YANGREN_CAPTURE: %s"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("BattlefieldLayout="),BattlefieldLayout))
    {
        const bool Valid=PrepareBattlefieldLayoutCapture(BattlefieldLayout);
        UE_LOG(LogTemp,Display,TEXT("GUNYU_BATTLEFIELD_LAYOUT_CAPTURE: %s"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("CoalitionPage="),CoalitionPage))
    {
        const bool Valid=PrepareCoalitionStoryPreview(CoalitionPage);
        UE_LOG(LogTemp,Display,TEXT("GUNYU_COALITION_CAPTURE: %s"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("DutyAftermath="),DutyAftermath))
    {
        const bool Valid=PrepareDutyAftermathPreview(DutyAftermath,FParse::Param(FCommandLine::Get(),TEXT("DutyAftermathSummary")));
        UE_LOG(LogTemp,Display,TEXT("GUNYU_DUTY_AFTERMATH_CAPTURE: %s"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bDutyResponseCapture && Frames==8)
    {
        const bool Valid=PrepareDutyResponseCapture(DutyResponse);
        UE_LOG(LogTemp,Display,TEXT("GUNYU_DUTY_RESPONSE_CAPTURE: %s"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("DutyProgress="),DutyProgress))
    {
        const bool Valid=PrepareDutyProgressPreview(DutyProgress);
        UE_LOG(LogTemp,Display,TEXT("GUNYU_DUTY_PROGRESS_CAPTURE: %s"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bRetryPayoffCapture && Frames==8)
    {
        const bool Valid=PrepareDefenseRetryPayoffCapture(DefenseRetryPayoff);
        UE_LOG(LogTemp,Display,TEXT("GUNYU_DEFENSE_RETRY_PAYOFF_CAPTURE: %s (normal aid and defense update from a paid retry)"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("DefenseRetry="),DefenseRetry))
    {
        CloseEpisode();bool Valid=PrepareCooperationCouncilPreview(Field,DefenseRetry==2 ? 2:0);
        if(Valid) { Field->Issue(ETacticalCommand::PlanExample);Field->Issue(ETacticalCommand::PlanAmbush);Field->Issue(ETacticalCommand::Deploy);Valid=PrepareDefenseRetryPreview(Field,DefenseRetry); }
        if(Valid)
        {
            OpenJourneyEntry();int Page=0;FParse::Value(FCommandLine::Get(),TEXT("DefenseRetryPage="),Page);CouncilPage=FMath::Clamp(Page,0,CouncilExchange.Lines.Num()-1);CouncilElapsed=1000;CouncilStageTime=CouncilLineTime=1;
            if(FParse::Param(FCommandLine::Get(),TEXT("DefenseRetryMap"))) OpenPlanningExample();
            if(FParse::Param(FCommandLine::Get(),TEXT("DefenseRetryAdopt"))) { OpenPlanningExample();ApplyPlanningExample(); }
            if(FParse::Param(FCommandLine::Get(),TEXT("DefenseRetryDeployed"))) { OpenPlanningExample();ApplyPlanningExample();CloseEpisode();Field->Issue(ETacticalCommand::Deploy);Focus=ATacticalField::TileLocation(frontline::Campaign::At(7,7));Distance=3300; }
        }
        UE_LOG(LogTemp,Display,TEXT("GUNYU_DEFENSE_RETRY_CAPTURE: %s (actual failed attempt, normal council rollback, explicit proposed/adopted roles)"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("DefenseAftermath="),DefenseAftermath))
    {
        CloseEpisode();bool Valid=PrepareCooperationCouncilPreview(Field,0);
        if(Valid) { Field->Issue(ETacticalCommand::PlanExample);Field->Issue(ETacticalCommand::PlanAmbush);Field->Issue(ETacticalCommand::Deploy);Valid=PrepareDefenseAftermathPreview(Field,DefenseAftermath); }
        if(Valid && !FParse::Param(FCommandLine::Get(),TEXT("DefenseAftermathSummary")))
        { OpenEpisode();int Page=1;FParse::Value(FCommandLine::Get(),TEXT("DefenseAftermathPage="),Page);EpisodePage=FMath::Clamp(Page,0,EpisodeSnapshot.Lines.Num()-1);NovelElapsed=1000;NovelStageTime=NovelLineTime=1; }
        UE_LOG(LogTemp,Display,TEXT("GUNYU_DEFENSE_AFTERMATH_CAPTURE: %s (actual settled command results after an authored contact boundary)"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("DefensePreview="),Defense))
    {
        CloseEpisode();bool Valid=PrepareCooperationCouncilPreview(Field,0);
        if(Valid) { Field->Issue(ETacticalCommand::PlanExample);Field->Issue(ETacticalCommand::PlanAmbush);Field->Issue(ETacticalCommand::Deploy);Valid=PrepareDefensePreview(Field,Defense); }
        Focus=ATacticalField::TileLocation(frontline::Campaign::At(7,7));Distance=2500;LabelLayoutFrame=MAX_uint64;
        UE_LOG(LogTemp,Display,TEXT("GUNYU_DEFENSE_CAPTURE: %s (explicit contact fixture, actual tactics and completed defense cycles)"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("SupportComboPreview="),SupportCombo))
    {
        CloseEpisode();bool Valid=PrepareCooperationCouncilPreview(Field,0);
        if(Valid) { Field->Issue(ETacticalCommand::PlanExample);Field->Issue(ETacticalCommand::PlanAmbush);Field->Issue(ETacticalCommand::Deploy);Valid=PrepareSupportCombinationPreview(Field,SupportCombo); }
        if(Valid && FParse::Param(FCommandLine::Get(),TEXT("SupportComboAct")))
        {
            const int Index=SupportCombo==2 ? 0:1;Valid=Field->UseBattleChoice(Index,Field->Presentation().BattleChoiceVersion);
            if(FParse::Param(FCommandLine::Get(),TEXT("SupportComboResolved"))) Field->AdvancePresentation(20);
            else { float Time=4.6f;FParse::Value(FCommandLine::Get(),TEXT("SupportComboTime="),Time);Field->AdvancePresentation(Time); }
        }
        Focus=ATacticalField::TileLocation(frontline::Campaign::At(6,6));Distance=2200;LabelLayoutFrame=MAX_uint64;
        UE_LOG(LogTemp,Display,TEXT("GUNYU_SUPPORT_COMBO_CAPTURE: %s (explicit tactical fixture, ordinary combination/followup)"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("SupportActionPreview="),SupportAction))
    {
        CloseEpisode();bool Valid=PrepareCooperationCouncilPreview(Field,0);
        if(Valid) { Field->Issue(ETacticalCommand::PlanExample);Field->Issue(ETacticalCommand::PlanAmbush);Field->Issue(ETacticalCommand::Deploy);Valid=PrepareSupportActionPreview(Field,SupportAction); }
        int Choice=-1;
        if(Valid && FParse::Value(FCommandLine::Get(),TEXT("SupportActionChoice="),Choice))
        {
            Valid=Field->UseBattleChoice(Choice,Field->Presentation().BattleChoiceVersion);
            if(FParse::Param(FCommandLine::Get(),TEXT("SupportActionResolved"))) Field->AdvancePresentation(20);
            else { float Time=1.7f;FParse::Value(FCommandLine::Get(),TEXT("SupportActionTime="),Time);Field->AdvancePresentation(Time); }
        }
        Focus=ATacticalField::TileLocation(frontline::Campaign::At(8,6));Distance=2200;LabelLayoutFrame=MAX_uint64;
        UE_LOG(LogTemp,Display,TEXT("GUNYU_SUPPORT_ACTION_CAPTURE: %s (explicit tactical fixture after normal council/deployment)"),Valid ? TEXT("PASS"):TEXT("FAIL"));
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("CooperationCouncil="),CooperationCouncil))
    {
        CloseEpisode();if(!PrepareCooperationCouncilPreview(Field,CooperationCouncil))
        { UE_LOG(LogTemp,Error,TEXT("GUNYU_COOPERATION_CAPTURE: FAIL (actual prior battle)"));FPlatformMisc::RequestExitWithStatus(false,1);return; }
        OpenJourneyEntry();CouncilElapsed=1000;CouncilStageTime=CouncilLineTime=1;
        if(FParse::Param(FCommandLine::Get(),TEXT("CooperationMap"))) OpenPlanningExample();
        if(FParse::Param(FCommandLine::Get(),TEXT("CooperationAdopt"))) ApplyPlanningExample();
        if(FParse::Param(FCommandLine::Get(),TEXT("SupportAttackCase"))) Field->Issue(ETacticalCommand::PlanAmbush);
        if(FParse::Param(FCommandLine::Get(),TEXT("CooperationDeploy"))) DeployNovelPlan();
        if(FParse::Param(FCommandLine::Get(),TEXT("SupportPreview")))
        {
            int Updates=0;FParse::Value(FCommandLine::Get(),TEXT("SupportAdvance="),Updates);
            for(int N=0;N<FMath::Clamp(Updates,0,4);++N) { Field->Issue(ETacticalCommand::RealtimeStep);Field->AdvancePresentation(30); }
            if(FParse::Param(FCommandLine::Get(),TEXT("SupportAttackCase")) && !PrepareSupportContactPreview(Field))
            { UE_LOG(LogTemp,Error,TEXT("GUNYU_SUPPORT_CAPTURE: FAIL (explicit visible-contact fixture)"));FPlatformMisc::RequestExitWithStatus(false,1);return; }
            Field->SelectOfficer(0);const auto& H=Field->Presentation();int Choice=-1;
            if(H.SupportChoices.Num()!=2) { UE_LOG(LogTemp,Error,TEXT("GUNYU_SUPPORT_CAPTURE: FAIL (missing choices)"));FPlatformMisc::RequestExitWithStatus(false,1);return; }
            const bool Apply=FParse::Value(FCommandLine::Get(),TEXT("SupportChoice="),Choice);
            if(Apply && !Field->UseSupportChoice(Choice,H.SupportVersion)) { UE_LOG(LogTemp,Error,TEXT("GUNYU_SUPPORT_CAPTURE: FAIL (choice rejected)"));FPlatformMisc::RequestExitWithStatus(false,1);return; }
            Focus=Field->VisualUnits()[0].Position;LabelLayoutFrame=MAX_uint64;
            UE_LOG(LogTemp,Display,TEXT("GUNYU_SUPPORT_CAPTURE: PASS (real cooperation council, normal deployment, attacks=%d, %s)"),H.BattleChoices.Num(),Apply ? TEXT("ordinary support order"):TEXT("two readable choices"));
        }
        const auto& P=Field->Presentation().Journey;const bool Deployed=FParse::Param(FCommandLine::Get(),TEXT("CooperationDeploy"));
        const bool Valid=Deployed ? Field->Presentation().bBattle && Field->RealtimePaused():P.bPreparation && P.ExampleAdviser==(CooperationCouncil==3 ? 2:1);
        UE_LOG(LogTemp,Display,TEXT("GUNYU_COOPERATION_CAPTURE: %s (%s)"),Valid ? TEXT("PASS"):TEXT("FAIL"),*P.ExampleReason);
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    int ContributionScene=0;
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("ContributionScene="),ContributionScene))
    {
        CloseEpisode();if(!PrepareContributionPreview(Field,ContributionScene))
        { UE_LOG(LogTemp,Error,TEXT("GUNYU_CONTRIBUTION_CAPTURE: FAIL (public playthrough import)"));FPlatformMisc::RequestExitWithStatus(false,1);return; }
        if(!FParse::Param(FCommandLine::Get(),TEXT("GunyuContributionSummary")))
        { OpenEpisode();int Page=1;FParse::Value(FCommandLine::Get(),TEXT("ContributionPage="),Page);EpisodePage=FMath::Clamp(Page,0,FMath::Max(0,EpisodeSnapshot.Lines.Num()-1));NovelElapsed=1000;NovelStageTime=NovelLineTime=1; }
        const auto& P=Field->Presentation().Journey;
        const bool Valid=P.bDebrief && P.Episode.Lines.Num()>=3 && P.Episode.Lines.Num()<=4 &&
            (ContributionScene==3 ? P.Episode.Setting.Contains(TEXT("連鎖に参加")):P.Episode.Lines[1].Speaker==ContributionScene);
        UE_LOG(LogTemp,Display,TEXT("GUNYU_CONTRIBUTION_CAPTURE: %s (%s)"),Valid ? TEXT("PASS"):TEXT("FAIL"),*P.Episode.Setting);
        if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    }
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("ReliefStory="),ReliefStory))
    {
        Field->PrepareReliefStoryPreview(ReliefStory);CloseEpisode();
        if(FParse::Param(FCommandLine::Get(),TEXT("GunyuRoleAftermath")))
        {
            if(!FParse::Param(FCommandLine::Get(),TEXT("GunyuRoleSummary")))
            { OpenEpisode();int Page=1;FParse::Value(FCommandLine::Get(),TEXT("RolePage="),Page);EpisodePage=FMath::Clamp(Page,0,FMath::Max(0,EpisodeSnapshot.Lines.Num()-1));NovelElapsed=1000;NovelStageTime=NovelLineTime=1; }
            const auto& P=Field->Presentation().Journey;
            const bool Valid=P.bDebrief && P.Episode.Lines.Num()==4 && P.Episode.Lines[1].Speaker==0 && P.Episode.Lines[2].Speaker==2 && P.CampaignChange.Contains(TEXT("張飛 → 劉備"));
            const auto Text=P.Episode.Lines.IsValidIndex(1) ? P.Episode.Lines[1].Text:FString();
            UE_LOG(LogTemp,Display,TEXT("GUNYU_ROLE_AFTERMATH_CAPTURE: %s (%s / %s)"),Valid ? TEXT("PASS"):TEXT("FAIL"),*P.Episode.Setting,*Text);
            if(!Valid) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
        }
        else
        {
            int Officer=0;FParse::Value(FCommandLine::Get(),TEXT("ReliefStoryOfficer="),Officer);
            const ETacticalCommand Talk[]={ETacticalCommand::TalkLiu,ETacticalCommand::TalkGuan,ETacticalCommand::TalkZhang};
            Field->Issue(Talk[FMath::Clamp(Officer,0,2)]);OpenOfficerNovel();
            EpisodePage=FMath::Max(0,EpisodeSnapshot.Lines.Num()-3);NovelElapsed=1000;NovelStageTime=NovelLineTime=1;
        }
    }
    int ReliefCouncil=1;
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("ReliefCouncil="),ReliefCouncil))
    { PrepareReliefAdviceCapture(ReliefCouncil);if(FParse::Param(FCommandLine::Get(),TEXT("GunyuApplyExample"))) ApplyPlanningExample(); }
    int ReliefReady=0;
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("ReliefReady="),ReliefReady))
    { Field->PrepareReliefReadinessPreview(ReliefReady);CloseEpisode();bEnemyDetailsOpen=false;Focus=Field->VisualUnits()[Field->Presentation().Selected].Position;LabelLayoutFrame=MAX_uint64; }
    int ReliefRoute=0;
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("ReliefRoute="),ReliefRoute))
    { Field->PrepareReliefApproachPreview(ReliefRoute);CloseEpisode();bEnemyDetailsOpen=false;Focus=Field->VisualUnits()[Field->Presentation().Selected].Position;LabelLayoutFrame=MAX_uint64; }
    if (bCapture && Frames == 150) FPlatformMisc::RequestExit(false);
}

void ATacticalController::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Overlay.IsValid() && GEngine && GEngine->GameViewport)
        GEngine->GameViewport->RemoveViewportWidgetContent(Overlay.ToSharedRef());
    Overlay.Reset();
    InputPanels.Reset();
    Super::EndPlay(Reason);
}
