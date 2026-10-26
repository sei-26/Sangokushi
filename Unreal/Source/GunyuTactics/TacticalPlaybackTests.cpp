#include "TacticalTrial.h"
#include "TacticalState.h"

bool ATacticalField::PlaybackSmokeTest()
{
    const int SavedMission = FirstMission, SavedPolicy = EnemyPolicy;
    const float SavedVolume = EffectsVolume, SavedSpeed = PlaybackSpeed;
    FirstMission = 0; EnemyPolicy = 0; EffectsVolume = 0; PlaybackSpeed = 1;
    auto Check = [](bool Condition, const TCHAR* Name)
    {
        if (!Condition) UE_LOG(LogTemp, Error, TEXT("GUNYU_PLAYBACK: FAIL: %s"), Name);
        return Condition;
    };
    Issue(ETacticalCommand::Restart); Issue(ETacticalCommand::Sample);
    const FVector Initial = State->Units[0].Position;
    Issue(ETacticalCommand::Execute);
    const int ResolvedTroops = State->Journey.world.armies[3].troops;
    const int ResolvedTurn = State->Journey.turn;
    if (!Check(IsPlaying() && State->Beats.Num() == 3 && State->Units[3].Troops == 1800, TEXT("three real-event beats start at pre-action soldiers"))) return false;
    if (!Check(!CanIssue(ETacticalCommand::Strike) && !CanIssue(ETacticalCommand::EndTurn), TEXT("commands blocked during replay"))) return false;
    Issue(ETacticalCommand::EndTurn); SelectOfficer(2);
    if (!Check(State->Journey.turn == ResolvedTurn && State->Selected == 0, TEXT("UI cannot mutate battle mid-replay"))) return false;
    AdvancePresentation(.8f);
    if (!Check(State->Units[0].Position != Initial && State->Units[0].Position != SurfaceLocation(State->Journey.world.armies[0].tile), TEXT("lure is interpolated"))) return false;
    AdvancePresentation(4.2f);
    if (!Check(CurrentBeat() && CurrentBeat()->Kind == EBattleCue::Ambush && State->Units[3].Loss == 413 && State->Units[3].Troops == 1387, TEXT("ambush reveals only its own step damage"))) return false;
    if (!Check(State->Journey.world.armies[3].troops == ResolvedTroops, TEXT("animation never reapplies damage"))) return false;
    Issue(ETacticalCommand::Skip);
    if (!Check(!IsPlaying() && State->Units[3].Troops == ResolvedTroops && State->Journey.turn == ResolvedTurn, TEXT("skip changes presentation only"))) return false;
    Issue(ETacticalCommand::Restart); Issue(ETacticalCommand::Sample); Issue(ETacticalCommand::Execute);
    AdvancePresentation(100.f);
    if (!Check(!IsPlaying() && State->Journey.world.armies[3].troops == ResolvedTroops, TEXT("natural completion equals skip"))) return false;
    Issue(ETacticalCommand::Restart); Issue(ETacticalCommand::Sample); EditOrder(2, -1); Issue(ETacticalCommand::Execute);
    if (!Check(State->Beats.Num() == 2 && State->Beats[1].Kind == EBattleCue::Stop, TEXT("failed step stops playback without inventing later attacks"))) return false;
    Issue(ETacticalCommand::Skip);
    if (!Check(State->Orders.size() == 2, TEXT("failed and unexecuted reservations retained"))) return false;
    Issue(ETacticalCommand::Restart); Issue(ETacticalCommand::Move);
    const int Destination = State->Reachable[0]; ClickTile(Destination);
    if (!Check(IsPlaying() && State->Units[0].Position != SurfaceLocation(Destination), TEXT("ordinary move animates"))) return false;
    AdvancePresentation(3.f);
    if (!Check(!IsPlaying() && State->Units[0].Position == SurfaceLocation(Destination), TEXT("move reaches canonical hex"))) return false;
    Issue(ETacticalCommand::EndTurn);
    for (const auto& Beat : State->Beats)
        for (int Id = 3; Id < 5; ++Id)
            if (Beat.After[Id].Id >= 0)
            {
                const auto* Contact = State->Journey.intelligence[0].Find(Id);
                if (!Check(Contact && Contact->current && Beat.After[Id].Position == SurfaceLocation(Contact->observed.tile), TEXT("enemy visuals use observations only"))) return false;
            }
    Issue(ETacticalCommand::Skip);
    FirstMission = SavedMission; EnemyPolicy = SavedPolicy; EffectsVolume = SavedVolume; PlaybackSpeed = SavedSpeed;
    Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp, Display, TEXT("GUNYU_PLAYBACK: PASS (step damage, interpolation, input lock, skip equivalence, stopped chain, observed enemies)"));
    return true;
}