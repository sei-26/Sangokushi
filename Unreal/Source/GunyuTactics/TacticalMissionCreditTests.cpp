#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
#include <memory>
using namespace frontline;
bool ATacticalField::MissionCreditSmokeTest()
{
    if(!gunyumission::CreditSmokeTest()) return false;
    const bool Council=bStartWithCouncil,Mode=bRealtimeCommand,Legacy=bLegacyJourney,Playback=State->bNoPlayback;
    const int Mission=FirstMission,Policy=EnemyPolicy;const float Speed=PlaybackSpeed,Volume=EffectsVolume;
    auto Check=[this](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_MISSION_CREDIT: FAIL %s (turn=%d held=%d stage=%d caption=%s report=%s)"),Why,State->Journey.turn,State->Journey.heldTurns,int(State->Journey.stage),*ReadMissionResponse().Caption,*State->Report);return V; };
    bStartWithCouncil=true;bRealtimeCommand=false;bLegacyJourney=false;FirstMission=0;EnemyPolicy=0;State->bNoPlayback=false;PlaybackSpeed=1;EffectsVolume=0;
    Issue(ETacticalCommand::Restart);for(int I=0;I<4;++I) Action(EKeys::Enter);
    auto Save=std::make_unique<JourneySaveData>();std::vector<std::uint8_t> Initial,Before,After;
    ExportSession(Before);if(!Check(DecodeJourneySave(Before,*Save),TEXT("public escort deployment"))) return false;
    auto& J=Save->journey;const int Next=J.ConvoyPath()[1],Out=J.world.armies[2].tile,Other=Campaign::At(7,10);
    // Isolated route fixture: Zhang blocks the car, Guan can reoccupy the road.
    J.world.armies[2].tile=J.world.armies[2].target=Next;J.world.armies[1].tile=J.world.armies[1].target=Other;J.chain.concealed[1]=false;
    for(int E:{3,4}) J.world.armies[E].troops=0;J.RefreshIntelligence();
    if(!Check(EncodeJourneySave(*Save,Initial) && ImportSession(Initial),TEXT("valid occupied-road fixture"))) return false;
    auto Move=[&](int Id,int Tile) { SelectOfficer(Id);Issue(ETacticalCommand::Move);CommandAt(Tile);Issue(ETacticalCommand::Skip); };
    Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);
    if(!Check(State->Journey.convoyStep==0 && ReadMissionResponse().Line.IsEmpty(),TEXT("a blocked cart never praises an unperformed role"))) return false;
    Move(2,Out);
    if(!Check(State->RoadClearArmy==2 && ReadMissionResponse().Line.IsEmpty(),TEXT("actual movement remembers Zhang without a premature reward"))) return false;
    Issue(ETacticalCommand::EndTurn);
    if(!Check(IsPlaying() && CurrentBeat()->RoadClearedBy==2 && ReadMissionResponse().Line.IsEmpty(),TEXT("real cart advance carries Zhang's evidence but waits for playback"))) return false;
    AdvancePresentation(10);ExportSession(Before);ReadMissionResponse();AdvancePresentation(1);ExportSession(After);
    if(!Check(ReadMissionResponse().Actor==2 && ReadMissionResponse().Reply.Contains(TEXT("翼徳")) && Before==After,TEXT("specific thanks neither farm trust nor alter costs or progress"))) return false;
    Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);
    if(!Check(ReadMissionResponse().Line.IsEmpty() && State->RoadClearArmy==-1,TEXT("a consumed clearance does not repeat on the next segment"))) return false;
    ImportSession(Initial);Move(2,Out);
    if(!Check(State->RoadClearArmy==2,TEXT("a new real clearance is pending before withdrawal"))) return false;
    Issue(ETacticalCommand::Withdraw);Issue(ETacticalCommand::Withdraw);Issue(ETacticalCommand::Skip);Issue(ETacticalCommand::Retry);
    if(!Check(State->Journey.stage==JourneyStage::Preparation && State->RoadClearArmy==-1,TEXT("withdrawal and retry cannot carry evidence into another battle"))) return false;
    ImportSession(Initial);Move(2,Out);Move(1,Next);Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);
    if(!Check(State->Journey.convoyStep==0 && State->RoadClearArmy==-1 && ReadMissionResponse().Line.IsEmpty(),TEXT("reoccupying the road invalidates the earlier clearer"))) return false;
    Move(1,Other);Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);
    if(!Check(ReadMissionResponse().Actor==1 && ReadMissionResponse().Caption.Contains(TEXT("関羽隊")),TEXT("the later actual clearer receives the credit"))) return false;
    ImportSession(Initial);Move(2,Out);ExportSession(Before);ImportSession(Before);Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);
    if(!Check(!ReadMissionResponse().Caption.Contains(TEXT("隊が車道")),TEXT("loading does not invent a pre-load personal contribution"))) return false;
    ImportSession(Initial);Issue(ETacticalCommand::RealtimeMode);SelectOfficer(2);CommandAt(Out);Issue(ETacticalCommand::RealtimeStep);
    if(!Check(CurrentBeat() && CurrentBeat()->RoadClearedBy==2,TEXT("same-cycle real-time movement and cart progress preserve the correct actor"))) return false;
    AdvancePresentation(10);
    if(!Check(RealtimePaused() && ReadMissionResponse().Actor==2,TEXT("real-time single-step publishes the same contribution paused"))) return false;
    PrepareReliefPreview(0);
    if(!Check(CommitRelief(),TEXT("public staged handover is legal"))) return false;
    AdvancePresentation(10);
    if(!Check(ReadMissionResponse().Line.IsEmpty(),TEXT("arrival alone does not certify a successful guard"))) return false;
    Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);ExportSession(Before);ReadMissionResponse();ExportSession(After);
    if(!Check(State->Journey.heldTurns==2 && ReadMissionResponse().Actor==0 && ReadMissionResponse().ReplyActor==2 && ReadMissionResponse().Caption.Contains(TEXT("張飛 → 劉備")) && Before==After,TEXT("the actual next guard check credits Liu and the outgoing Zhang exactly once"))) return false;
    Move(0,Campaign::At(8,8));Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);Move(0,Campaign::At(8,9));
    for(int Id:{0,1}) { SelectOfficer(Id);Issue(ETacticalCommand::Hold);Issue(ETacticalCommand::Skip); }
    Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);
    if(!Check(State->Journey.heldTurns==1 && !ReadMissionResponse().Caption.Contains(TEXT("→")),TEXT("old settled handovers do not replay after later lost and regained guard"))) return false;
    PrepareReliefPreview(2);CommitRelief();Issue(ETacticalCommand::Skip);Issue(ETacticalCommand::EndTurn);AdvancePresentation(10);
    if(!Check(State->Journey.heldTurns==0 && !ReadMissionResponse().Caption.Contains(TEXT("→")),TEXT("an exposed failed handover cannot claim preserved guard"))) return false;
    bStartWithCouncil=Council;bRealtimeCommand=Mode;bLegacyJourney=Legacy;FirstMission=Mission;EnemyPolicy=Policy;State->bNoPlayback=Playback;PlaybackSpeed=Speed;EffectsVolume=Volume;Issue(ETacticalCommand::Restart);
    if(!Check(State->RoadClearArmy==-1,TEXT("restart clears transient role evidence"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_MISSION_CREDIT: PASS (actual named clearing, reoccupation and single-use evidence, immutable thanks, load/reset, real-time progress, confirmed/failed/stale handovers)"));return true;
}
