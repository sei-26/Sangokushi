#include "TacticalTrial.h"
#include "Camera/CameraActor.h"
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
    Exposure.AmbientOcclusionIntensity = .45f;
    Exposure.bOverride_AmbientOcclusionRadius = true;
    Exposure.AmbientOcclusionRadius = 70.f;
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
    const bool bCinemaCapture = bCapture && (FParse::Param(FCommandLine::Get(), TEXT("GunyuCinema")) || bSignatureCapture || bSoloCapture);
    const bool bSignalCapture=bCapture && FParse::Param(FCommandLine::Get(),TEXT("GunyuChainSignal"));
    if (Field->IsPlaying() || Field->Presentation().Journey.bVisible || WasInputKeyJustPressed(EKeys::Escape)) bTacticPickerOpen = false;
    if (!bCinemaCapture)
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
        if (FParse::Param(FCommandLine::Get(),TEXT("GunyuCouncilPreview")))
        { Field->bStartWithCouncil=true;Field->FirstMission=0;Field->EnemyPolicy=1;Field->Issue(ETacticalCommand::Restart); }
        if (FParse::Param(FCommandLine::Get(),TEXT("GunyuRealtime")))
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
        const bool bBattle = bCinemaCapture || bDebrief || bWithdrawal || FParse::Param(FCommandLine::Get(), TEXT("GunyuPlan")) || FParse::Param(FCommandLine::Get(), TEXT("GunyuMove")) || FParse::Param(FCommandLine::Get(), TEXT("GunyuBattlePreview")) || FParse::Param(FCommandLine::Get(), TEXT("GunyuTacticMenu"));
        if (bCouncil || bBattle)
            for (int Step = 0; Step < 3; ++Step) Field->Action(EKeys::Enter);
        if (bDebrief) Field->Issue(ETacticalCommand::PromiseRoad);
        if (bSignatureCapture || FParse::Param(FCommandLine::Get(),TEXT("GunyuReserve"))) Field->Issue(ETacticalCommand::PlanSupplies);
        if (bBattle) Field->Action(EKeys::Enter);
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
        if(bSoloCapture)
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
        Field->Issue(ETacticalCommand::Execute);
        float Time = bSoloCapture ? 1.7f : bSignatureCapture ? 9.5f : 4.7f;
        FParse::Value(FCommandLine::Get(), TEXT("CueTime="), Time);
        Field->AdvancePresentation(Time);
    }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuAccessibilityOnly")))
    {
        const bool bPassed=AccessibilitySmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return;
    }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuReliefStoryOnly")))
    { const bool bPassed=ReliefStorySmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
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
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuPreparationOnly")))
    { const bool bPassed=PreparationMeaningSmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if(bSmoke && Frames==2 && FParse::Param(FCommandLine::Get(),TEXT("GunyuCampaignOnly")))
    { const bool bPassed=CampaignPacingSmokeTest();FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0:1);return; }
    if (bSmoke && Frames == 2)
    {
        bool bPassed = Field->SmokeTest();
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
        if(bPassed) bPassed=PlanAdviceSmokeTest();
        if(bPassed) bPassed=Field->GuardControlSmokeTest();
        if(bPassed) bPassed=Field->TacticTargetSmokeTest();
        if(bPassed) bPassed=TacticTargetInterfaceSmokeTest();
        if(bPassed) bPassed=Field->TacticPositionSmokeTest();
        if(bPassed) bPassed=TacticPositionInterfaceSmokeTest();
        if(bPassed) bPassed=Field->ReliefSmokeTest();
        if(bPassed) bPassed=ReliefInterfaceSmokeTest();
        if(bPassed) bPassed=ReliefStorySmokeTest();
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
        const int Id=Beat && Units.IsValidIndex(Beat->Actor) && Units[Beat->Actor].Id>=0 ? Beat->Actor:0;
        if(Units.IsValidIndex(Id)) { Focus=Units[Id].Position;Distance=ViewDistance=1200;CameraYaw=125;CameraElevation=36;CameraPreset=-1;bTacticCinema=false; }
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
    if(bCapture && Frames==8 && FParse::Value(FCommandLine::Get(),TEXT("ReliefStory="),ReliefStory))
    {
        Field->PrepareReliefStoryPreview(ReliefStory);CloseEpisode();
        int Officer=0;FParse::Value(FCommandLine::Get(),TEXT("ReliefStoryOfficer="),Officer);
        const ETacticalCommand Talk[]={ETacticalCommand::TalkLiu,ETacticalCommand::TalkGuan,ETacticalCommand::TalkZhang};
        Field->Issue(Talk[FMath::Clamp(Officer,0,2)]);OpenOfficerNovel();
        EpisodePage=FMath::Max(0,EpisodeSnapshot.Lines.Num()-3);NovelElapsed=1000;NovelStageTime=NovelLineTime=1;
    }
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
