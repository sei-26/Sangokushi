#include "TacticalTrial.h"
#include "TacticalState.h"

using namespace frontline;

bool ATacticalField::EnemyReadSmokeTest()
{
    const int SavedMission = FirstMission, SavedPolicy = EnemyPolicy;
    const bool SavedCouncil = bStartWithCouncil, SavedNoPlayback = State->bNoPlayback;
    const bool SavedOverlay = State->bThreatOverlay;
    const float SavedVolume = EffectsVolume;
    const bool SavedReadable=bReadableFoliage; bReadableFoliage=true;
    FirstMission = 0; EnemyPolicy = 1; bStartWithCouncil = false;
    State->bNoPlayback = false; State->bThreatOverlay = true; EffectsVolume = 0;
    auto Check = [](bool Value, const TCHAR* Reason)
    {
        if (!Value) UE_LOG(LogTemp, Error, TEXT("GUNYU_ENEMY_READ: FAIL: %s"), Reason);
        return Value;
    };
    Issue(ETacticalCommand::Restart);
    if (!Check(Presentation().EnemyReads.Num() == 1 && Presentation().EnemyReads[0].Intent == TEXT("陣を維持") && !Presentation().ThreatCells.IsEmpty(), TEXT("visible guard read and threat cells; hidden reserve absent"))) return false;
    if (!Check(State->FoliageClearings.Num()==6 && State->FoliageClearings[4].A==0 && State->FoliageClearings[4].R<0,
        TEXT("hidden reserve never reaches canopy shader parameters"))) return false;
    const int Revision = State->Journey.world.revision, Morale = State->Journey.world.armies[0].morale;
    bReadableFoliage=false;RenderUnits();
    for (const auto& Clearing : State->FoliageClearings)
        if (!Check(Clearing.A==0 && Clearing.R<0,TEXT("canopy assistance can be disabled completely"))) return false;
    if (!Check(State->Journey.world.revision==Revision,TEXT("canopy assistance does not consume a game action"))) return false;
    bReadableFoliage=true;RenderUnits();
    ToggleThreatOverlay();
    if (!Check(!Presentation().bThreatOverlay && State->Journey.world.revision == Revision && State->Journey.world.armies[0].morale == Morale, TEXT("overlay toggle consumes no action"))) return false;
    ToggleThreatOverlay();
    SelectOfficer(0); Issue(ETacticalCommand::Move); CommandAt(Campaign::At(9,11)); Issue(ETacticalCommand::Skip);
    if (!Check(State->Journey.world.armies[0].tile == Campaign::At(9,11), TEXT("clear Liu Bei's occupied tile before preparing roar"))) return false;
    SelectOfficer(2); Issue(ETacticalCommand::Move);
    SelectAt(Presentation().EnemyReads[0].Tile);
    if (!Check(State->Target == 3 && State->Input == ETacticalInput::Select, TEXT("enemy card selects target and closes map input"))) return false;
    Issue(ETacticalCommand::Move); CommandAt(Campaign::At(9,10)); Issue(ETacticalCommand::Skip);
    if (!Check(State->Journey.world.armies[2].tile == Campaign::At(9,10), TEXT("Zhang Fei reaches the guard"))) return false;
    SelectTactic(static_cast<int>(ChainTactic::SerpentRoar)); Issue(ETacticalCommand::QueueTactic);
    const FString Before = Presentation().EnemyReads[0].Intent;
    Issue(ETacticalCommand::Execute);
    if (!Check(IsPlaying() && Presentation().EnemyReads[0].Intent == Before && VisualUnits()[3].bGuarded, TEXT("read never reveals final state during playback"))) return false;
    Issue(ETacticalCommand::Skip);
    if (!Check(Presentation().EnemyReads[0].Intent == TEXT("混乱・移動停止") && Presentation().ThreatFrom == Presentation().ThreatNext && !VisualUnits()[3].bGuarded, TEXT("successful signature updates read and stops advance"))) return false;
    Issue(ETacticalCommand::EndTurn); Issue(ETacticalCommand::Skip);
    if (!Check(Presentation().EnemyReads[0].Intent != TEXT("陣を維持") && Presentation().EnemyReads[0].Intent != TEXT("混乱・移動停止"), TEXT("guard break persists; disorder expires next turn"))) return false;
    EnemyPolicy = 2; Issue(ETacticalCommand::Restart);
    if (!Check(Presentation().EnemyReads.Num() == 2, TEXT("both visible enemies have selectable cards"))) return false;
    SelectAt(Campaign::At(7,11));
    if (!Check(State->Target == 4 && Presentation().EnemyReads[1].bSelected && Presentation().EnemyReads[1].Intent == TEXT("索敵しつつ接近"), TEXT("scout card uses observed scouting and updates selected threat"))) return false;
    if (!Check(State->FoliageClearings[4].A>0,TEXT("scout clears leaves only after becoming observed"))) return false;
    bReadableFoliage=SavedReadable;
    FirstMission = SavedMission; EnemyPolicy = SavedPolicy; bStartWithCouncil = SavedCouncil;
    State->bNoPlayback = SavedNoPlayback; State->bThreatOverlay = SavedOverlay; EffectsVolume = SavedVolume;
    Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp, Display, TEXT("GUNYU_ENEMY_READ: PASS (observed guard, hidden reserve, non-consuming toggle, target selection, playback privacy, signature counter and recovery)"));
    return true;
}
