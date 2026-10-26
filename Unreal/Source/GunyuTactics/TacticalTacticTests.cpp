#include "TacticalTrial.h"
#include "TacticalState.h"

using namespace frontline;

bool ATacticalField::TacticRepertoireSmokeTest()
{
    const int SavedMission = FirstMission, SavedPolicy = EnemyPolicy;
    const bool SavedCouncil = bStartWithCouncil, SavedNoPlayback = State->bNoPlayback;
    const float SavedVolume = EffectsVolume, SavedSpeed = PlaybackSpeed;
    FirstMission = 0; EnemyPolicy = 0; bStartWithCouncil = true;
    State->bNoPlayback = false; EffectsVolume = 0; PlaybackSpeed = 1;
    auto Check = [](bool Value, const TCHAR* Reason)
    {
        if (!Value) UE_LOG(LogTemp, Error, TEXT("GUNYU_REPERTOIRE: FAIL: %s"), Reason);
        return Value;
    };
    auto PrepareFormation = [this]
    {
        Issue(ETacticalCommand::Restart);
        for (int I = 0; I < 3; ++I) Issue(ETacticalCommand::Dialogue);
        Issue(ETacticalCommand::PlanSupplies); Issue(ETacticalCommand::Deploy);
        const int Destinations[] = {Campaign::At(9,11),Campaign::At(9,9),Campaign::At(9,10)};
        for (int I = 0; I < 3; ++I)
        { SelectOfficer(I); Issue(ETacticalCommand::Move); CommandAt(Destinations[I]); Issue(ETacticalCommand::Skip); }
    };
    PrepareFormation(); SelectOfficer(0);
    SelectTactic(static_cast<int>(ChainTactic::Oath));
    const int Morale = State->Journey.world.armies[0].morale;
    if (!Check(Presentation().Tactics.Num() == 4 && !State->Journey.chain.signatureUsed[0] && !State->Journey.chain.spent[0], TEXT("selection exposes four choices and consumes nothing"))) return false;
    State->Target = -1; Refresh();
    if (!Check(CanIssue(ETacticalCommand::QueueTactic), TEXT("support needs no hostile target"))) return false;
    Issue(ETacticalCommand::QueueTactic);
    SelectAt(Campaign::At(10,10)); SelectOfficer(2); SelectTactic(static_cast<int>(ChainTactic::SerpentRoar)); Issue(ETacticalCommand::QueueTactic);
    SelectOfficer(1); SelectTactic(static_cast<int>(ChainTactic::AzureDragon)); Issue(ETacticalCommand::QueueTactic);
    if (!Check(Presentation().Predicted == 3 && State->Journey.world.armies[0].morale == Morale && !State->Journey.chain.signatureUsed[0], TEXT("whole signature chain previews without spending costs"))) return false;
    Issue(ETacticalCommand::Execute);
    if (!Check(State->Beats.Num() == 3 && State->Beats[0].Kind == EBattleCue::Rally && State->Beats[2].bSignature, TEXT("support and unique attacks receive distinct real-event beats"))) return false;
    const int FinalTroops = State->Journey.world.armies[3].troops;
    AdvancePresentation(1.9f);
    if (!Check(State->Units[1].bInspired && State->Units[3].Troops == 1800, TEXT("support is shown before future enemy damage"))) return false;
    AdvancePresentation(3.8f);
    if (!Check(CurrentBeat()->Kind == EBattleCue::SerpentRoar && State->Units[3].Troops == 1522 && State->Units[3].bDisordered, TEXT("roar shows only its own damage and disorder"))) return false;
    AdvancePresentation(3.8f);
    if (!Check(CurrentBeat()->Kind == EBattleCue::AzureDragon && State->Units[3].Loss == 744 && State->Units[3].Troops == FinalTroops, TEXT("hero attack uses the exact inspired damage"))) return false;
    Issue(ETacticalCommand::Skip); SelectOfficer(0);
    if (!Check(!CanIssue(ETacticalCommand::QueueTactic) && State->Journey.chain.signatureUsed[0], TEXT("used signature cannot be queued again"))) return false;
    Issue(ETacticalCommand::EndTurn); Issue(ETacticalCommand::Skip);
    if (!Check(State->Journey.chain.signatureUsed[0] && !CanIssue(ETacticalCommand::QueueTactic), TEXT("next turn does not replenish a signature"))) return false;
    EnemyPolicy = 1; PrepareFormation();
    const ChainTactic Tactics[] = {ChainTactic::Feint,ChainTactic::Disrupt,ChainTactic::Breakthrough};
    for (int I = 0; I < 3; ++I)
    { SelectOfficer(I); SelectTactic(static_cast<int>(Tactics[I])); Issue(ETacticalCommand::QueueTactic); }
    if (!Check(Presentation().Predicted == 3, TEXT("alternative common chain counters guarding enemy"))) return false;
    EditOrder(1,-1);
    if (!Check(Presentation().Predicted == 0 && !State->Journey.chain.pressured[3], TEXT("wrong order fails without leaking preview state"))) return false;
    EditOrder(0,1); Issue(ETacticalCommand::Execute); Issue(ETacticalCommand::Skip);
    if (!Check(State->Journey.lastChain.completed == 3 && !State->Journey.chain.signatureUsed[1], TEXT("common chain works without consuming signature uses"))) return false;
    FirstMission = SavedMission; EnemyPolicy = SavedPolicy; bStartWithCouncil = SavedCouncil;
    State->bNoPlayback = SavedNoPlayback; EffectsVolume = SavedVolume; PlaybackSpeed = SavedSpeed;
    Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp, Display, TEXT("GUNYU_REPERTOIRE: PASS (four choices, support, exact step effects, unique limits, alternative chain, non-destructive preview)"));
    return true;
}
