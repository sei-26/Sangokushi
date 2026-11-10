#include "TacticalTrial.h"
#include "TacticalChrome.h"
#include "TacticalMapLabelLayout.h"
#include "JourneySave.hpp"
#include <memory>
bool ATacticalController::MissionFocusSmokeTest()
{
    using namespace frontline;
    const bool Council=Field->bStartWithCouncil,Mode=Field->bRealtimeCommand,Legacy=Field->bLegacyJourney;
    const int Mission=Field->FirstMission,Policy=Field->EnemyPolicy,Preset=CameraPreset;const auto OldFocus=Focus;const float OldDistance=Distance;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_MISSION_FOCUS: FAIL %s"),Why);return V; };
    TArray<FSlateRect> Occupied{FSlateRect(260,120,620,230)};
    const FVector2D Size(180,70),Min=gunyulayout::LabelOrigin,Max(1256,712);
    const auto First=gunyumaplabel::Reserve(FVector2D(350,92),Size,Min,Max,Occupied);
    const auto Second=gunyumaplabel::Reserve(FVector2D(350,92),Size,Min,Max,Occupied);
    if(!Check(First.Y>=Min.Y && Second.Y>=Min.Y && Occupied.Num()==3 &&
        !gunyumaplabel::Intersects(Occupied[0],Occupied[1]) && !gunyumaplabel::Intersects(Occupied[0],Occupied[2]) &&
        !gunyumaplabel::Intersects(Occupied[1],Occupied[2]),
        TEXT("two overlapping projected beacons stay below the header and clear the response and each other"))) return false;
    CloseEpisode();Field->bStartWithCouncil=true;Field->bRealtimeCommand=false;Field->bLegacyJourney=false;Field->FirstMission=0;Field->EnemyPolicy=0;
    Field->Issue(ETacticalCommand::Restart);
    for(int I=0;I<4;++I) Field->Action(EKeys::Enter);
    std::vector<std::uint8_t> Before,After;auto Save=std::make_unique<JourneySaveData>();
    if(!Check(Field->ExportSession(Before) && DecodeJourneySave(Before,*Save) && Save->journey.HasConvoy(),TEXT("ordinary public council deploys the promised escort"))) return false;
    auto& J=Save->journey;const int Next=J.ConvoyPath()[J.convoyStep+1],Original=J.world.armies[0].tile;
    if(!Check(Field->Presentation().bConvoyMission && Field->Presentation().MissionFocusTile==Next && Field->Presentation().MissionStep.Contains(TEXT("護送")),TEXT("escort points to the actual next route cell"))) return false;
    FocusMissionObjective();Field->ExportSession(After);
    if(!Check(Focus.Equals(Field->SurfaceLocation(Next)) && Before==After,TEXT("goal focus changes the camera without selecting, ordering, spending or advancing"))) return false;
    // Isolated display fixture: a friendly blocks the observed, enemy-free route.
    J.world.armies[3].troops=J.world.armies[4].troops=0;
    J.world.armies[0].tile=J.world.armies[0].target=Next;J.RefreshIntelligence();
    if(!Check(EncodeJourneySave(*Save,Before) && Field->ImportSession(Before) && Field->Presentation().MissionStep.Contains(TEXT("隊が進路上")),TEXT("friendly traffic names its real blocker"))) return false;
    J.world.armies[0].tile=J.world.armies[0].target=Original;J.RefreshIntelligence();
    if(!Check(EncodeJourneySave(*Save,Before) && Field->ImportSession(Before) && Field->Presentation().MissionStep.Contains(TEXT("前路は空いた")),TEXT("cleared route switches from combat to advancing the actual escort"))) return false;
    const auto PublicStep=Field->Presentation().MissionStep;
    J.world.armies[4].troops=1300;J.world.armies[4].tile=J.world.armies[4].target=Next;
    if(!Check(EncodeJourneySave(*Save,Before) && Field->ImportSession(Before) && Field->Presentation().MissionStep==PublicStep && Field->Presentation().MissionFocusTile==Next,TEXT("unobserved actual enemy occupancy cannot change the hint or its focus"))) return false;
    J.world.armies[4].troops=0;J.RefreshIntelligence();
    if(!Check(EncodeJourneySave(*Save,Before) && Field->ImportSession(Before),TEXT("restore the public clear-route fixture"))) return false;
    FocusMissionObjective();const auto DisplayedFocus=Focus;
    Field->Issue(ETacticalCommand::EndTurn);FocusMissionObjective();
    if(!Check(Field->IsPlaying() && Focus==DisplayedFocus && Field->Presentation().MissionStep==PublicStep,TEXT("playback neither exposes the resolved future goal nor reframes it"))) return false;
    Field->Issue(ETacticalCommand::Skip);
    if(!Check(Field->ExportSession(After) && DecodeJourneySave(After,*Save) && Save->journey.convoyStep==1 && Field->Presentation().MissionFocusTile==Save->journey.ConvoyPath()[2] && Field->Presentation().MissionStep.Contains(TEXT("護送 1/")),TEXT("real command progression updates the displayed route and count after playback"))) return false;
    Field->bStartWithCouncil=false;
    for(int M:{1,2,0})
    {
        Field->FirstMission=M;Field->Issue(ETacticalCommand::Restart);
        if(!Check(Field->ExportSession(Before) && DecodeJourneySave(Before,*Save),TEXT("read the actual mission"))) return false;
        if(M>0)
        {
            // Settled-history display fixture; no claimed victory is manufactured by the UI.
            auto& Prior=Save->journey.history[M-1];Prior.completed=Prior.won=true;Prior.story.recorded=true;Prior.story.reply=JourneyPledge::Coordinate;
            if(M==2) Prior.review.coordinatedOfficers=3;
            if(!Check(EncodeJourneySave(*Save,Before) && Field->ImportSession(Before),TEXT("display the prior promise and earned hold reduction"))) return false;
        }
        const auto& H=Field->Presentation();const auto Kind=Save->journey.Definition().kind;
        if(Kind==JourneyObjective::CaptureCamp && !Check(H.MissionFocusTile==Save->journey.goal && H.MissionStep.Contains(TEXT("河港占領")),TEXT("river port focuses its occupation objective"))) return false;
        if(Kind==JourneyObjective::CaptureCamp && !Check(H.Objective.Contains(TEXT("指揮所")) && !H.Objective.Contains(TEXT("烽火")),TEXT("a prior promise does not replace port occupation with a beacon objective"))) return false;
        if(Kind==JourneyObjective::HoldPositions && !Check(H.MissionFocusTile==Save->journey.goal && H.MissionStep.Contains(TEXT("北の守備")),TEXT("dual beacons focus the wing that still needs its guard"))) return false;
        if(Kind==JourneyObjective::HoldPositions && !Check(H.MissionStep.Contains(TEXT("0/2")) && H.Objective.Contains(TEXT("0/2")),TEXT("actual reduced hold requirement is consistent with the original objective detail"))) return false;
        if(Kind==JourneyObjective::Breakout && !Check(H.MissionDetail.Contains(TEXT("先遣伏兵")) && H.MissionStep.Contains(TEXT("退路確保")),TEXT("the council preparation remains connected to the breakout"))) return false;
        FocusMissionObjective();Field->ExportSession(After);
        if(!Check(Before==After,TEXT("reading and focusing every objective preserve the saved session"))) return false;
    }
    Field->bStartWithCouncil=true;Field->bRealtimeCommand=true;Field->FirstMission=1;Field->Issue(ETacticalCommand::Restart);
    for(int I=0;I<3;++I) Field->Action(EKeys::Enter);
    Field->Issue(ETacticalCommand::PlanExample);Field->Issue(ETacticalCommand::Deploy);
    const auto BeforeFreeFocus=Focus;FocusSelectedDuty();
    if(!Check(SelectedDutyTile()<0 && Focus==BeforeFreeFocus,TEXT("free Zhang has no council destination to focus"))) return false;
    Field->SelectOfficer(0);
    if(!Check(Field->ExportSession(Before) && DecodeJourneySave(Before,*Save) && SelectedDutyTile()==Save->journey.assignments[0].tile &&
        SelectedDutyTile()!=Field->Presentation().MissionFocusTile,TEXT("the real council destination is distinct from the river-port objective"))) return false;
    const auto DutyFocus=Field->SurfaceLocation(SelectedDutyTile());FocusSelectedDuty();
    if(!Check(Focus.Equals(DutyFocus) && Field->ExportSession(After) && Before==After,
        TEXT("duty focus frames the public assignment without changing the officer or save"))) return false;
    Field->Issue(ETacticalCommand::RealtimeStep);FocusSelectedDuty();
    if(!Check(Field->IsPlaying() && SelectedDutyTile()<0 && Focus.Equals(DutyFocus),TEXT("duty focus is unavailable while the cycle plays"))) return false;
    Field->Issue(ETacticalCommand::Skip);
    Field->bStartWithCouncil=Council;Field->bRealtimeCommand=Mode;Field->bLegacyJourney=Legacy;Field->FirstMission=Mission;Field->EnemyPolicy=Policy;Field->Issue(ETacticalCommand::Restart);
    Focus=OldFocus;Distance=OldDistance;CameraPreset=Preset;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_MISSION_FOCUS: PASS (escort route, fog, public goals, collision-safe plates, actual council destination, immutable view controls and playback guards)"));return true;
}
