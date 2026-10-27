#include "TacticalTrial.h"
#include "TacticalState.h"

bool ATacticalField::SmokeTest()
{
    using namespace frontline;
    const bool SavedRealtimeMode=bRealtimeCommand; bRealtimeCommand=false;
    State->bNoPlayback = true;
    // Test the public UI commands; preview/reordering must not consume an action.
    const int SavedMission = FirstMission, SavedPolicy = EnemyPolicy;
    const bool SavedCouncil = bStartWithCouncil; bStartWithCouncil = false;
    FirstMission = 0; EnemyPolicy = 0;
    Issue(ETacticalCommand::Restart);
    SelectOfficer(0); Issue(ETacticalCommand::QueueTactic);
    if (!State->Reachable.Contains(Campaign::At(8, 11))) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL at line %d; predicted=%d; orders=%d; report=%s"), __LINE__, Presentation().Predicted, static_cast<int>(State->Orders.size()), *State->Report); return false; }
    CommandAt(Campaign::At(8, 11));
    SelectOfficer(1); Issue(ETacticalCommand::QueueTactic);
    SelectOfficer(2); Issue(ETacticalCommand::QueueTactic);
    if (Presentation().Predicted != 3 || State->Orders.size() != 3) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL at line %d; predicted=%d; orders=%d; report=%s"), __LINE__, Presentation().Predicted, static_cast<int>(State->Orders.size()), *State->Report); return false; }
    const int EnemyTroops = State->Journey.world.armies[3].troops;
    State->Journey.world.armies[2].morale=5;
    EditOrder(2, -1); // Insufficient morale stops the second hand, preserving later officers.
    if (Presentation().Predicted != 1 || State->Journey.world.armies[3].troops != EnemyTroops || State->Journey.chain.spent[0]) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL at line %d; predicted=%d; orders=%d; report=%s"), __LINE__, Presentation().Predicted, static_cast<int>(State->Orders.size()), *State->Report); return false; }
    Issue(ETacticalCommand::Execute);
    if (State->Journey.lastChain.completed != 1 || State->Orders.size() != 2 || State->Journey.chain.spent[1]) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL at line %d; predicted=%d; orders=%d; report=%s"), __LINE__, Presentation().Predicted, static_cast<int>(State->Orders.size()), *State->Report); return false; }
    EditOrder(1, -1);
    // An exposed ambush still works as a weaker solo attack; the low-morale successor waits.
    if (Presentation().Predicted != 1 || State->Journey.chain.concealed[1]) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL: stopped ambush was not re-evaluated")); return false; }
    EditOrder(0, 0);
    SelectOfficer(1); Issue(ETacticalCommand::Strike);
    if (!State->Journey.chain.spent[1] || State->Journey.world.armies[3].troops >= EnemyTroops) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL: unused officer could not attack after partial chain")); return false; }
    Issue(ETacticalCommand::Restart);
    Issue(ETacticalCommand::Sample);
    Issue(ETacticalCommand::Execute);
    if (State->Journey.lastChain.completed != 3 || !State->Orders.empty()) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL: complete chain")); return false; }
    Issue(ETacticalCommand::EndTurn);
    SelectOfficer(2); Issue(ETacticalCommand::QueueTactic); Issue(ETacticalCommand::Execute);
    if (!State->Journey.won) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL at line %d; predicted=%d; orders=%d; report=%s"), __LINE__, Presentation().Predicted, static_cast<int>(State->Orders.size()), *State->Report); return false; }
    Issue(ETacticalCommand::Continue);
    if (State->Journey.mission != 1 || State->Journey.world.armies.size() != 6) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL at line %d; predicted=%d; orders=%d; report=%s"), __LINE__, Presentation().Predicted, static_cast<int>(State->Orders.size()), *State->Report); return false; }
    Issue(ETacticalCommand::Restart);
    Issue(ETacticalCommand::Sample);
    Issue(ETacticalCommand::EndTurn);
    if (State->Journey.turn != 1 || !State->bConfirmEndTurn) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL at line %d; predicted=%d; orders=%d; report=%s"), __LINE__, Presentation().Predicted, static_cast<int>(State->Orders.size()), *State->Report); return false; }
    Issue(ETacticalCommand::Cancel);
    if (State->bConfirmEndTurn || State->Orders.size() != 3) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL at line %d; predicted=%d; orders=%d; report=%s"), __LINE__, Presentation().Predicted, static_cast<int>(State->Orders.size()), *State->Report); return false; }
    Issue(ETacticalCommand::EndTurn); Issue(ETacticalCommand::EndTurn);
    if (State->Journey.turn != 2 || !State->Orders.empty()) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL at line %d; predicted=%d; orders=%d; report=%s"), __LINE__, Presentation().Predicted, static_cast<int>(State->Orders.size()), *State->Report); return false; }
    Issue(ETacticalCommand::Restart);
    SelectOfficer(0); Issue(ETacticalCommand::Move);
    if (State->Reachable.IsEmpty()) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL at line %d; predicted=%d; orders=%d; report=%s"), __LINE__, Presentation().Predicted, static_cast<int>(State->Orders.size()), *State->Report); return false; }
    const int Destination = State->Reachable[0];
    ClickTile(Destination);
    if (State->Journey.world.armies[0].tile != Destination || CanIssue(ETacticalCommand::Move)) { UE_LOG(LogTemp, Error, TEXT("GUNYU_RULES_SMOKE: FAIL at line %d; predicted=%d; orders=%d; report=%s"), __LINE__, Presentation().Predicted, static_cast<int>(State->Orders.size()), *State->Report); return false; }
    FirstMission = SavedMission; EnemyPolicy = SavedPolicy;
    Issue(ETacticalCommand::Restart);
    State->bNoPlayback = false;
    if (!PlaybackSmokeTest()) return false;
    bStartWithCouncil = SavedCouncil;
    if (!JourneySmokeTest()) return false;
    if (!StoryPromiseSmokeTest()) return false;
    if (!TacticRepertoireSmokeTest()) return false;
    if (!ComboSmokeTest()) return false;
    if (!TacticAvailabilitySmokeTest()) return false;
    if (!TacticCinemaSmokeTest()) return false;
    if (!SpectacleSmokeTest()) return false;
    if (!ChainReviewSmokeTest()) return false;
    if (!ChainPresentationSmokeTest()) return false;
    if (!EnemyReadSmokeTest()) return false;
    if (!VisualGeometrySmokeTest()) return false;
    if (!BackdropSmokeTest()) return false;
    if (!InfantryMotionSmokeTest()) return false;
    if (!ActivityReadSmokeTest()) return false;
    if (!RealtimeSmokeTest()) return false;
    if (!ChainSignalSmokeTest()) return false;
    if (!BattleWatchSmokeTest()) return false;
    if (!MarchPlanSmokeTest()) return false;
    if (!OpeningUpgradeSmokeTest()) return false;
    if (!SaveSessionSmokeTest()) return false;
    bRealtimeCommand=SavedRealtimeMode;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp, Display, TEXT("GUNYU_RULES_SMOKE: PASS (individual orders, immutable prediction, reorder, partial chain recovery, queued-turn guard, move, next mission)"));
    return true;
}
