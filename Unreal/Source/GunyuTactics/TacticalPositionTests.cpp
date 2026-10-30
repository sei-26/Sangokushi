#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
using namespace frontline;
void ATacticalField::PrepareTacticPositionPreview(int Variant)
{
    bLegacyJourney=false;bStartWithCouncil=false;FirstMission=Variant==2 ? 2:0;EnemyPolicy=0;bRealtimeCommand=false;Issue(ETacticalCommand::Restart);
    if(Variant!=2)
    { SelectOfficer(0);Issue(ETacticalCommand::Move);CommandAt(Campaign::At(9,11));Issue(ETacticalCommand::Skip); }
    SelectOfficer(Variant==1 ? 1:2);
    if(Variant==2) { bRealtimeCommand=true;Refresh();State->Realtime.Queue(State->Journey,2,JourneyDirective::Hold); }
    BeginTacticPositionComparison();
    int Chosen=-1;
    for(const auto& C:Presentation().Position.Cells) if(C.bReady) { Chosen=C.Tile;break; }
    if(Chosen<0) for(const auto& C:Presentation().Position.Cells) if(C.bMoveAllowed) { Chosen=C.Tile;break; }
    if(Chosen>=0) SelectTacticPosition(Chosen);
}
bool ATacticalField::TacticPositionSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_TACTIC_POSITIONS: FAIL %s"),Why);return V; };
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedRealtime=bRealtimeCommand,SavedLegacy=bLegacyJourney,SavedNoPlayback=State->bNoPlayback;
    State->bNoPlayback=false;
    auto Bytes=[this]() { std::vector<std::uint8_t> B;ExportSession(B);return B; };
    const int Destination=Campaign::At(9,10);
    PrepareTacticPositionPreview(0);Issue(ETacticalCommand::Cancel);const auto Before=Bytes();
    if(!Check(BeginTacticPositionComparison() && Presentation().Position.bVisible && !Presentation().Position.bCanCommit,TEXT("opening requires explicit tile selection before commit"))) return false;
    const auto Solo=ReadTacticPosition(Destination);ClickTile(Destination);RenderEffects();
    if(!Check(Solo.bMoveAllowed && Solo.bReady && Solo.Step==1 && Solo.ComboPercent==100 && Presentation().Position.bCanCommit && Bytes()==Before,TEXT("reading, selecting and drawing a real adjacent attack position leave all serialized gameplay unchanged"))) return false;
    auto Expected=std::make_unique<JourneySaveData>();DecodeJourneySave(Before,*Expected);
    if(!Check(Expected->journey.Move(2,Destination),TEXT("reference normal move is legal"))) return false;
    std::vector<std::uint8_t> ExpectedBytes;EncodeJourneySave(*Expected,ExpectedBytes);
    if(!Check(CommitTacticPosition() && IsPlaying() && !CommitTacticPosition(),TEXT("explicit commit performs exactly one normal move and respects replay lock"))) return false;
    Issue(ETacticalCommand::Skip);
    if(!Check(Bytes()==ExpectedBytes && State->Journey.chain.moved[2] && !State->Journey.chain.spent[2] && !Presentation().Position.bVisible,TEXT("manual commit matches normal move byte for byte, preserves action and closes preview"))) return false;
    Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.lastChain.completed==1 && State->Journey.lastChain.events[0].damage==Solo.Damage,TEXT("subsequent actual solo tactic matches hypothetical damage"))) return false;
    PrepareTacticPositionPreview(1);const int AmbushTile=Presentation().Position.SelectedTile;
    const auto Ambush=ReadTacticPosition(AmbushTile);
    if(!Check(State->Journey.chain.concealed[1] && Ambush.bReady && Ambush.Risk.Contains(TEXT("潜伏解除")),TEXT("moving prepared Guan displays the concealment cost"))) return false;
    CommitTacticPosition();Issue(ETacticalCommand::Skip);Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.lastChain.completed==1 && State->Journey.lastChain.events[0].setupPercent==100 && State->Journey.lastChain.events[0].damage==Ambush.Damage,TEXT("actual moved ambush loses setup bonus exactly as forecast"))) return false;
    PrepareTacticPositionPreview(0);Issue(ETacticalCommand::Cancel);
    State->Orders={{0,-1,ChainTactic::Oath,-1}};Refresh();BeginTacticPositionComparison();
    const auto Follow=ReadTacticPosition(Destination);
    if(!Check(Follow.bReady && Follow.Step==2 && Follow.ComboPercent==125,TEXT("movement comparison preserves preceding support and combo effects"))) return false;
    SelectTacticPosition(Destination);CommitTacticPosition();Issue(ETacticalCommand::Skip);Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.lastChain.completed==2 && State->Journey.lastChain.events[1].damage==Follow.Damage,TEXT("actual move then two-step chain matches forecast including inspiration"))) return false;
    PrepareTacticPositionPreview(0);State->Journey.world.armies[2].morale=0;Refresh();
    if(!Check(ReadTacticPosition(Destination).bMoveAllowed && !ReadTacticPosition(Destination).bReady,TEXT("legal move never invents missing tactic morale"))) return false;
    State->Journey.world.armies[2].morale=90;State->Journey.chain.signatureUsed[2]=true;Issue(ETacticalCommand::Cancel);SelectTactic(int(ChainTactic::SerpentRoar));BeginTacticPositionComparison();
    if(!Check(!ReadTacticPosition(Destination).bReady && ReadTacticPosition(Destination).Result.Contains(TEXT("使用済み")),TEXT("moving does not restore a used signature"))) return false;
    Issue(ETacticalCommand::Cancel);SelectTactic(int(ChainTactic::Feint));State->Journey.chain.spent[2]=true;BeginTacticPositionComparison();
    if(!Check(ReadTacticPosition(Destination).bMoveAllowed && !ReadTacticPosition(Destination).bReady,TEXT("unmoved officer may reposition after acting but cannot attack twice"))) return false;
    State->Journey.chain.moved[2]=true;Refresh();
    if(!Check(!ReadTacticPosition(Destination).bMoveAllowed && !Presentation().Position.bCanCommit && !CommitTacticPosition(),TEXT("spent movement cannot be refunded through comparison"))) return false;
    // Existing legal terrain and observed occupation remain authoritative for hints.
    PrepareTacticPositionPreview(0);State->Journey.world.tiles[Destination].terrain=Terrain::River;Refresh();
    if(!Check(!ReadTacticPosition(Destination).bMoveAllowed,TEXT("rivers remain impassable"))) return false;
    PrepareTacticPositionPreview(0);State->Journey.world.armies[4].tile=State->Journey.world.armies[4].target=Destination;State->Journey.RefreshIntelligence();Refresh();
    if(!Check(!ReadTacticPosition(Destination).bMoveAllowed,TEXT("observed enemy occupation blocks the hypothetical move"))) return false;
    PrepareTacticPositionPreview(0);const auto Known=ReadTacticPosition(Destination);
    State->Journey.world.armies[4].tile=State->Journey.world.armies[4].target=Destination;Refresh();
    const auto Hidden=ReadTacticPosition(Destination);
    if(!Check(Hidden.bReady && Hidden.Result==Known.Result && Hidden.Risk==Known.Risk && Hidden.Damage==Known.Damage,TEXT("unobserved occupation changes no read or warning"))) return false;
    SelectTacticPosition(Destination);
    if(!Check(!CommitTacticPosition() && !State->Journey.chain.moved[2] && !State->Journey.chain.spent[2] && (!State->Journey.intelligence[0].Find(4) || !State->Journey.intelligence[0].Find(4)->current),TEXT("actual hidden obstruction may stop movement without spending or free scouting"))) return false;
    PrepareTacticPositionPreview(0);State->Journey.intelligence[0].visible[Destination]=false;
    if(!Check(ReadTacticPosition(Destination).Tile<0 && !CommitTacticPosition(),TEXT("cached selection is revalidated when visibility is lost"))) return false;
    PrepareTacticPositionPreview(2);
    if(!Check(Presentation().Position.Risk.Contains(TEXT("烽火")) && Presentation().Position.Timing.Contains(TEXT("置き換え")),TEXT("beacon and standing-order consequences are visible before confirmation"))) return false;
    // Opening changes only pause; committing changes only the selected future order.
    PrepareTacticPositionPreview(0);Issue(ETacticalCommand::Cancel);bRealtimeCommand=true;Refresh();
    // Freeze the visible enemy for the paid-cycle assertion; normal RT may move it.
    State->Journey.enemyPolicy=1;
    for(int I=3;I<int(State->Journey.world.armies.size());++I) State->Journey.world.armies[I].stance=battle::Stance::Guard;
    State->Journey.RefreshIntelligence();
    State->Orders={{0,-1,ChainTactic::Oath,-1}};State->Realtime.Queue(State->Journey,1,JourneyDirective::Ambush);
    State->Realtime.Queue(State->Journey,2,JourneyDirective::March,Campaign::At(12,11));
    State->Realtime.AppendWaypoint(State->Journey,2,Campaign::At(13,11));State->Realtime.SetArrival(State->Journey,2,JourneyArrival::Hold);
    State->ChainSignal.Arm(State->Journey,State->Orders);State->Realtime.paused=false;State->Realtime.Advance(2);Refresh();
    Expected=std::make_unique<JourneySaveData>();DecodeJourneySave(Bytes(),*Expected);Expected->realtime.paused=true;ExpectedBytes.clear();EncodeJourneySave(*Expected,ExpectedBytes);
    if(!Check(BeginTacticPositionComparison() && Bytes()==ExpectedBytes,TEXT("RT opening pauses without clearing orders, waypoints, signal, resources or elapsed time"))) return false;
    SelectTacticPosition(Destination);
    Expected->realtime.Queue(Expected->journey,2,JourneyDirective::March,Destination);ExpectedBytes.clear();EncodeJourneySave(*Expected,ExpectedBytes);
    if(!Check(CommitTacticPosition() && Bytes()==ExpectedBytes && State->ChainSignal.Armed() && !State->Journey.chain.moved[2],TEXT("RT commit only replaces selected march and arrival/waypoints, without a free movement or clock tick"))) return false;
    const auto Ordered=Bytes();
    if(!Check(ImportSession(Ordered) && !Presentation().Position.bVisible && State->Realtime.orders[2].destination==Destination,TEXT("loading preserves actual order, never persists a hypothetical selection"))) return false;
    Issue(ETacticalCommand::RealtimeStep);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.world.armies[2].tile==Destination && State->Journey.turn==2 && State->Realtime.orders[2].kind==JourneyDirective::None,TEXT("RT movement requires a real full cycle and stops after the chosen single step"))) return false;
    // New mode must not convert lure's movement budget into a second withdrawal.
    bRealtimeCommand=false;FirstMission=0;bStartWithCouncil=false;Issue(ETacticalCommand::Restart);SelectOfficer(0);BeginTacticPositionComparison();
    bool MovedLureBlocked=false;
    for(const auto& C:Presentation().Position.Cells) if(C.bMoveAllowed && C.Result.Contains(TEXT("誘導役は移動済み"))) MovedLureBlocked=true;
    if(!Check(MovedLureBlocked,TEXT("a hypothetical advance cannot also pay for lure retreat"))) return false;
    Issue(ETacticalCommand::Cancel);SelectTactic(int(ChainTactic::Oath));
    if(!Check(!CanCompareTacticPositions(),TEXT("support does not advertise enemy attack positions"))) return false;
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedRealtime;bLegacyJourney=SavedLegacy;State->bNoPlayback=SavedNoPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_TACTIC_POSITIONS: PASS (immutable hypothetical move, actual solo/chain damage, lost ambush, budgets and signatures, public occupation, fog, objective warnings, exact RT order/pause conservation, saved orders and real cycle)"));return true;
}
