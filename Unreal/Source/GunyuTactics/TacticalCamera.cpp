#include "TacticalTrial.h"
#include "Camera/CameraActor.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
using namespace gunyuui;

FVector ATacticalController::CameraOffset(float Radius) const
{
    const float Yaw=FMath::DegreesToRadians(CameraYaw),Tilt=FMath::DegreesToRadians(CameraElevation);
    return FVector(FMath::Cos(Yaw)*FMath::Cos(Tilt),FMath::Sin(Yaw)*FMath::Cos(Tilt),FMath::Sin(Tilt))*Radius;
}
void ATacticalController::ResetBattleCamera()
{
    CameraYaw=90;CameraElevation=48.f;CameraPreset=0;
    Focus=FVector(1600,1400,0);Distance=4300;
}
void ATacticalController::CycleCameraView()
{
    const bool LeavingVista=CameraPreset==3;
    CameraPreset=(CameraPreset+1)%4;
    if(CameraPreset==3) { BeforeVistaFocus=Focus;Focus=FVector(2100,650,0); }
    else if(LeavingVista) Focus=BeforeVistaFocus;
    const float Yaws[]={90,125,90,90},Tilts[]={48.f,40,72,32},Ranges[]={2800,2800,3900,5500};
    CameraYaw=Yaws[CameraPreset];CameraElevation=Tilts[CameraPreset];Distance=Ranges[CameraPreset];
}
void ATacticalController::FocusSelectedOfficer()
{
    const auto& Units=Field->VisualUnits();const int Id=Field->Presentation().Selected;
    if(!Units.IsValidIndex(Id) || Units[Id].Id<0 || Units[Id].Troops<=0) return;
    // Always use the displayed observation, including during a battle animation.
    Focus=Units[Id].Position;Distance=FMath::Min(Distance,2600.f);
}
void ATacticalController::UpdateBattleCamera(float Delta,bool bBlocked)
{
    const bool OverUI=IsPointerOverInterface();
    if(!bBlocked)
    {
        if(WasInputKeyJustPressed(EKeys::O)) bTacticCinema=!bTacticCinema;
        if(WasInputKeyJustPressed(EKeys::K)) CycleCameraView();
        if(WasInputKeyJustPressed(EKeys::J)) FocusSelectedOfficer();
        if(WasInputKeyJustPressed(EKeys::Home)) ResetBattleCamera();
        if(!OverUI && WasInputKeyJustPressed(EKeys::MouseScrollUp)) Distance-=220;
        if(!OverUI && WasInputKeyJustPressed(EKeys::MouseScrollDown)) Distance+=220;
        float X=0,Y=0;const bool HasCursor=GetMousePosition(X,Y);
        if(!OverUI && WasInputKeyJustPressed(EKeys::MiddleMouseButton) && HasCursor)
        { bOrbitDragging=true;PreviousOrbitCursor=FVector2D(X,Y); }
        if(bOrbitDragging && IsInputKeyDown(EKeys::MiddleMouseButton) && HasCursor)
        {
            const FVector2D Motion=FVector2D(X,Y)-PreviousOrbitCursor;PreviousOrbitCursor=FVector2D(X,Y);
            CameraYaw+=Motion.X*.18f;CameraElevation-=Motion.Y*.12f;CameraPreset=-1;
        }
        else bOrbitDragging=false;
        const float Angle=FMath::DegreesToRadians(CameraYaw),Speed=FMath::Min(Delta,.1f)*Distance*.45f;
        const FVector Forward(-FMath::Cos(Angle),-FMath::Sin(Angle),0),Right(FMath::Sin(Angle),-FMath::Cos(Angle),0);
        if(IsInputKeyDown(EKeys::Left)) Focus-=Right*Speed;
        if(IsInputKeyDown(EKeys::Right)) Focus+=Right*Speed;
        if(IsInputKeyDown(EKeys::Up)) Focus+=Forward*Speed;
        if(IsInputKeyDown(EKeys::Down)) Focus-=Forward*Speed;
    }
    else bOrbitDragging=false;
    Distance=FMath::Clamp(Distance,1200.f,5500.f);
    CameraElevation=FMath::Clamp(CameraElevation,32.f,76.f);CameraYaw=FMath::UnwindDegrees(CameraYaw);
    Focus.X=FMath::Clamp(Focus.X,200.,3000.);Focus.Y=FMath::Clamp(Focus.Y,200.,2400.);
    ViewDistance=FMath::FInterpTo(ViewDistance,Distance,FMath::Min(Delta,.1f),10.f);
    FVector ViewFocus=Focus;float Radius=ViewDistance,Tilt=CameraElevation;
    ApplyTacticCamera(ViewFocus,Radius,Tilt);
    const float Yaw=FMath::DegreesToRadians(CameraYaw),Elevation=FMath::DegreesToRadians(Tilt);
    const FVector Offset(FMath::Cos(Yaw)*FMath::Cos(Elevation)*Radius,FMath::Sin(Yaw)*FMath::Cos(Elevation)*Radius,FMath::Sin(Elevation)*Radius);
    BattleCamera->SetActorLocation(ViewFocus+Offset);BattleCamera->SetActorRotation((-Offset).Rotation());
    UpdateTacticLens();
    const auto Cue=Field->ReadTacticCinema();
    if(CinematicIntro) CinematicIntro->SetRenderOpacity(Cue.Intro);
    if(CinematicResult) CinematicResult->SetRenderOpacity(Cue.Impact);
    if(CinematicRelay) CinematicRelay->SetRenderOpacity(Cue.Relay);
    LabelLayoutFrame=MAX_uint64;
}
void ATacticalController::BuildCameraInterface(TSharedRef<SCanvas> Canvas)
{
    auto Controls=SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Label([] { return TEXT("中ドラッグ回転"); },11)]
        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,6,0)[Button([this]
          { const TCHAR* Names[]={TEXT("標準"),TEXT("臨場"),TEXT("俯瞰"),TEXT("遠景")};return FString(TEXT("視点 "))+(CameraPreset<0 ? TEXT("自由") : Names[CameraPreset])+TEXT(" / K"); },[this] { CycleCameraView(); },[] { return true; },true)]
        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,6,0)[Button([] { return TEXT("選択隊へ J"); },[this] { FocusSelectedOfficer(); },[] { return true; },true)]
        + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("全景 Home"); },[this] { ResetBattleCamera(); },[] { return true; },true)];
    Controls->AddSlot().AutoWidth().Padding(5,0,0,0)[Button([this] { return bTacticCinema ? TEXT("演出ON O") : TEXT("演出OFF O"); },[this] { bTacticCinema=!bTacticCinema; },[] { return true; },true)];
    auto Panel=SNew(STacticalChrome).Padding(8)
        .Visibility_Lambda([this] { return Field->Presentation().bBattle && !Field->IsPlaying() ? EVisibility::Visible : EVisibility::Collapsed; })[Controls];
    InputPanels.Add(Panel);Canvas->AddSlot().Position(FVector2D(716,116)).Size(FVector2D(540,50))[Panel];
}
