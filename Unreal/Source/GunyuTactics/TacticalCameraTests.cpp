#include "TacticalTrial.h"
#include "TacticalFormation.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "StrategistJourney.hpp"

bool ATacticalController::CameraPresentationSmokeTest()
{
    const auto SavedFocus=Focus;const float SavedYaw=CameraYaw,SavedTilt=CameraElevation,SavedDistance=Distance;
    const int SavedPreset=CameraPreset;
    auto Check=[](bool Value,const TCHAR* Reason) { if(!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_BATTLE_VIEW: FAIL %s"),Reason);return Value; };
    Field->PrepareOpeningPreview(false);Field->bRealtimeCommand=true;Field->Issue(ETacticalCommand::Deploy);
    std::vector<std::uint8_t> Before,After;
    if(!Check(Field->ExportSession(Before),TEXT("stable battle snapshot"))) return false;
    CameraYaw=90;CameraElevation=53.130102f;
    if(!Check(CameraOffset(1000).Equals(FVector(0,600,800),.02),TEXT("standard angle matches the previous view"))) return false;
    for(float Yaw:{0.f,90.f,135.f,230.f}) for(float Tilt:{32.f,48.f,53.f,76.f})
    {
        CameraYaw=Yaw;CameraElevation=Tilt;
        if(!Check(FMath::IsNearlyEqual(CameraOffset(1800).Size(),1800.,.1),TEXT("orbit preserves distance"))) return false;
        for(int Tile:{frontline::Campaign::At(7,9),frontline::Campaign::At(9,10),frontline::Campaign::At(12,11)})
        {
            const FVector Goal=Field->SurfaceLocation(Tile),Offset=CameraOffset(2600);
            if(!Check(Field->PickSurfaceTile(Goal+Offset,-Offset.GetSafeNormal())==Tile,TEXT("rotated view picks the same visible hex center"))) return false;
        }
    }
    CameraPreset=0;CycleCameraView();FocusSelectedOfficer();
    if(!Check(CameraPreset==1 && CameraElevation==40 && Focus.Equals(Field->VisualUnits()[0].Position),TEXT("cinematic preset and focus use displayed ally"))) return false;
    ResetBattleCamera();if(!Check(CameraPreset==0 && Distance==4300,TEXT("Home restores a full-map view"))) return false;
    const FVector TacticalFocus=Focus;
    CycleCameraView();CycleCameraView();CycleCameraView();
    if(!Check(CameraPreset==3 && Focus.Equals(FVector(2100,650,0)),TEXT("vista preset frames the public backdrop"))) return false;
    CycleCameraView();if(!Check(CameraPreset==0 && Focus.Equals(TacticalFocus),TEXT("leaving vista returns to prior tactical focus"))) return false;
    if(!Check(Field->ExportSession(After) && Before==After,TEXT("view controls consume no command, food or time"))) return false;
    const bool SavedDetails=bEnemyDetailsOpen;
    SelectMapUnit(2);if(!Check(Field->Presentation().Selected==2,TEXT("nameplate selects the displayed ally"))) return false;
    SelectMapUnit(4);if(!Check(Field->Presentation().Selected==2,TEXT("unobserved reserve cannot be selected by a plate"))) return false;
    SelectMapUnit(3);if(!Check(bEnemyDetailsOpen,TEXT("enemy plate opens observed counter information"))) return false;
    SelectMapUnit(0);bEnemyDetailsOpen=SavedDetails;
    if(!Check(Field->ExportSession(After) && Before==After,TEXT("plate selection consumes no action or resources"))) return false;
    Field->Issue(ETacticalCommand::Sample);
    if(!Check(UnitChainNumber(0)==TEXT("① ") && UnitChainNumber(2)==TEXT("③ ") && UnitChainNumber(4).IsEmpty(),TEXT("map numbers match own reserved chain only"))) return false;
    Field->EditOrder(2,-1);
    if(!Check(UnitChainNumber(2)==TEXT("② ") && UnitChainNumber(1)==TEXT("③ "),TEXT("map numbers follow reordered actors"))) return false;
    Field->Issue(ETacticalCommand::Clear);
    if(!Check(UnitChainNumber(0).IsEmpty(),TEXT("clear removes map reservation badges"))) return false;
    const bool OriginalMode=Field->bRealtimeCommand;Field->bRealtimeCommand=false;
    Field->Issue(ETacticalCommand::Sample);Field->Issue(ETacticalCommand::Execute);
    Field->AdvancePresentation(1.6f);
    if(!Check(Field->IsPlaying() && Field->ReadTacticCinema().CameraWeight>0,TEXT("camera fixture enters an actual tactic replay"))) return false;
    const auto OriginalFocus=Focus;const float OriginalRange=Distance,OriginalTilt=CameraElevation;
    const bool OriginalCinema=bTacticCinema;bTacticCinema=true;
    FVector Presented=Focus;float PresentedRange=ViewDistance,PresentedTilt=CameraElevation;
    ApplyTacticCamera(Presented,PresentedRange,PresentedTilt);
    if(!Check(!Presented.Equals(Focus) && PresentedRange<ViewDistance && Focus.Equals(OriginalFocus) && Distance==OriginalRange && CameraElevation==OriginalTilt,TEXT("tactic camera never overwrites player orbit or focus"))) return false;
    UpdateTacticLens();
    if(!Check(BattleCamera->GetCameraComponent()->PostProcessSettings.VignetteIntensity>.22f,TEXT("tactic lens follows the current shot"))) return false;
    bTacticCinema=false;UpdateTacticLens();
    const auto& NeutralLens=BattleCamera->GetCameraComponent()->PostProcessSettings;
    if(!Check(FMath::IsNearlyEqual(NeutralLens.BloomIntensity,.35f) && FMath::IsNearlyEqual(NeutralLens.VignetteIntensity,.22f) && FMath::IsNearlyEqual(NeutralLens.AutoExposureBias,.25f),TEXT("cinema off immediately restores neutral lens settings"))) return false;
    bTacticCinema=false;Presented=Focus;PresentedRange=ViewDistance;PresentedTilt=CameraElevation;
    ApplyTacticCamera(Presented,PresentedRange,PresentedTilt);
    if(!Check(Presented.Equals(Focus) && PresentedRange==ViewDistance && PresentedTilt==CameraElevation,TEXT("cinema off retains the tactical view"))) return false;
    bTacticCinema=OriginalCinema;Field->Issue(ETacticalCommand::Skip);Field->bRealtimeCommand=OriginalMode;
    FVisualUnit Unit;Unit.Id=0;Unit.Troops=1400;
    using namespace gunyuformation;
    Unit.bGuarded=true;if(!Check(Shape(Unit,false)==EShape::ShieldWall,TEXT("visible guard creates shield wall"))) return false;
    Unit.Motion=1;if(!Check(Shape(Unit,false)==EShape::Column,TEXT("actual travel creates a column"))) return false;
    Unit.bDisordered=true;if(!Check(Shape(Unit,false)==EShape::Disordered,TEXT("disorder overrides a neat formation"))) return false;
    for(auto Formation:{EShape::Line,EShape::Column,EShape::ShieldWall,EShape::Hidden,EShape::Disordered,EShape::Wedge})
        for(int I=0;I<48;++I) if(!Check(Offset(Formation,I,48).Size2D()<115,TEXT("representative soldiers remain within their unit footprint"))) return false;
    CameraYaw=SavedYaw;CameraElevation=SavedTilt;Distance=SavedDistance;Focus=SavedFocus;CameraPreset=SavedPreset;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_BATTLE_VIEW: PASS (orbit/picking, displayed focus, no simulation mutation, visible formations and bounded footprint)"));
    return true;
}
