#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
#include <memory>
bool ATacticalField::MissionMomentsSmokeTest()
{
    if(!gunyumission::SmokeTest()) return false;
    const bool Council=bStartWithCouncil,Mode=bRealtimeCommand,Legacy=bLegacyJourney,Playback=State->bNoPlayback;
    const int Mission=FirstMission,Policy=EnemyPolicy;const float Speed=PlaybackSpeed,Volume=EffectsVolume;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_MISSION_MOMENTS: FAIL %s"),Why);return V; };
    bStartWithCouncil=true;bRealtimeCommand=false;bLegacyJourney=false;FirstMission=0;EnemyPolicy=0;State->bNoPlayback=false;PlaybackSpeed=1;EffectsVolume=0;
    Issue(ETacticalCommand::Restart);for(int I=0;I<4;++I) Action(EKeys::Enter);
    auto Save=std::make_unique<frontline::JourneySaveData>();std::vector<std::uint8_t> Initial,Natural,ReadBefore,ReadAfter;
    if(!Check(ExportSession(Initial) && frontline::DecodeJourneySave(Initial,*Save),TEXT("ordinary public escort deployment"))) return false;
    // A quiet route isolates progress feedback; no claimed combat victory.
    for(int E:{3,4}) Save->journey.world.armies[E].troops=0;Save->journey.RefreshIntelligence();
    if(!Check(frontline::EncodeJourneySave(*Save,Initial) && ImportSession(Initial),TEXT("isolated quiet escort route"))) return false;
    Issue(ETacticalCommand::EndTurn);
    if(!Check(IsPlaying() && ReadMissionResponse().Line.IsEmpty() && CurrentBeat()->MissionBefore.Convoy==0 && CurrentBeat()->MissionAfter.Convoy==1,TEXT("resolved progress remains private during playback"))) return false;
    AdvancePresentation(10);
    if(!Check(!IsPlaying() && ReadMissionResponse().Caption.Contains(TEXT("1/6")) && ExportSession(Natural),TEXT("natural movement completion publishes actual first progress"))) return false;
    ReadMissionResponse();ReadMissionResponse();AdvancePresentation(1);
    if(!Check(ExportSession(ReadAfter) && Natural==ReadAfter,TEXT("reading and the display timer change no saved progress, costs or trust"))) return false;
    AdvancePresentation(4);
    if(!Check(ReadMissionResponse().Line.IsEmpty(),TEXT("the short notice expires without input"))) return false;
    Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);
    if(!Check(ReadMissionResponse().Line.IsEmpty(),TEXT("second ordinary segment does not repeat the first speech"))) return false;
    Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);
    if(!Check(ReadMissionResponse().Caption.Contains(TEXT("道半ば")),TEXT("real third segment publishes the halfway response"))) return false;
    if(!Check(ImportSession(Initial) && ReadMissionResponse().Line.IsEmpty(),TEXT("loaded state never replays a transient speech"))) return false;
    Issue(ETacticalCommand::EndTurn);Issue(ETacticalCommand::Skip);
    if(!Check(ReadMissionResponse().Line.IsEmpty() && ExportSession(ReadAfter) && ReadAfter==Natural,TEXT("skip clears the response and preserves the exact actual outcome"))) return false;
    ImportSession(Initial);Issue(ETacticalCommand::RealtimeSpeed);Issue(ETacticalCommand::RealtimeSpeed);
    Issue(ETacticalCommand::EndTurn);AdvancePresentation(1);
    if(!Check(IsPlaying() && ReadMissionResponse().Line.IsEmpty(),TEXT("three-times playback still waits for the shown movement"))) return false;
    AdvancePresentation(.2f);ExportSession(ReadBefore);AdvancePresentation(1);
    if(!Check(ReadMissionResponse().Weight>.9f && ExportSession(ReadAfter) && ReadBefore==ReadAfter,TEXT("three-times playback keeps the notice readable in real seconds"))) return false;
    Issue(ETacticalCommand::RealtimeMode);Issue(ETacticalCommand::RealtimeStep);AdvancePresentation(10);
    if(!Check(RealtimePaused() && State->Journey.convoyStep==2 && ReadMissionResponse().Line.IsEmpty(),TEXT("real-time single-step shares the same quiet milestone rules"))) return false;
    bRealtimeCommand=false;bStartWithCouncil=false;FirstMission=2;Issue(ETacticalCommand::Restart);
    ExportSession(ReadBefore);frontline::DecodeJourneySave(ReadBefore,*Save);auto& J=Save->journey;
    const int Original=J.world.armies[0].tile,Other=J.world.armies[1].tile;
    J.world.armies[0].tile=J.world.armies[0].target=J.goal;J.world.armies[1].tile=J.world.armies[1].target=J.Definition().secondGoal;J.world.armies[2].tile=J.world.armies[2].target=Other;
    for(int E:{3,4}) J.world.armies[E].troops=0;J.RefreshIntelligence();
    if(!Check(frontline::EncodeJourneySave(*Save,ReadBefore) && ImportSession(ReadBefore),TEXT("display fixture with both guard locations occupied"))) return false;
    Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);
    if(!Check(State->Journey.heldTurns==1 && ReadMissionResponse().Caption.Contains(TEXT("烽火 1/")),TEXT("actual first maintained guard publishes the matching count"))) return false;
    ExportSession(ReadBefore);frontline::DecodeJourneySave(ReadBefore,*Save);Save->journey.world.armies[0].tile=Save->journey.world.armies[0].target=Original;Save->journey.RefreshIntelligence();
    if(!Check(frontline::EncodeJourneySave(*Save,ReadBefore) && ImportSession(ReadBefore),TEXT("guard leaves one wing"))) return false;
    Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);
    if(!Check(State->Journey.heldTurns==0 && ReadMissionResponse().Caption.Contains(TEXT("途切れ")),TEXT("actual lost guard progress asks for recovery"))) return false;
    bStartWithCouncil=Council;bRealtimeCommand=Mode;bLegacyJourney=Legacy;FirstMission=Mission;EnemyPolicy=Policy;State->bNoPlayback=Playback;PlaybackSpeed=Speed;EffectsVolume=Volume;Issue(ETacticalCommand::Restart);
    if(!Check(State->MissionResponseRemaining==0,TEXT("restart clears all transient response state"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_MISSION_MOMENTS: PASS (actual manual/RT convoy and guard progress, no future reveal, immutable reading, skip equality, real-time 3x readability, load/reset)"));return true;
}
