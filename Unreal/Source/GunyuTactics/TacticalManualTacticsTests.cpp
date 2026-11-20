#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include <memory>
using namespace frontline;

bool ATacticalController::ManualTacticsSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_MANUAL_TACTICS: FAIL %s"),Why);return V; };
    const bool Detailed=bDetailedBattle;Field->Issue(ETacticalCommand::Skip);Field->PrepareTacticTargetPreview(0);bDetailedBattle=false;
    if(!Check(ShowManualTactics() && Field->Presentation().Tactics.Num()>=4 && Field->CanIssue(ETacticalCommand::QueueTactic),
        TEXT("simple manual control exposes the real loadout and a usable tactic"))) return false;
    std::vector<std::uint8_t> Before,After;Field->ExportSession(Before);
    ToggleTacticPicker();ToggleTacticPicker();Field->ExportSession(After);
    if(!Check(Before==After && ShowManualTactics(),TEXT("opening and closing the picker spends no action or resources"))) return false;
    Field->Issue(ETacticalCommand::QueueTactic);
    if(!Check(Field->CanFireManualTactics(),TEXT("a solo order is available for immediate manual fire"))) return false;
    Field->Action(EKeys::C);
    if(!Check(Field->IsPlaying() && !Field->CanFireManualTactics(),
        TEXT("a solo order fires through the actual tactic pipeline, not just a proposal"))) return false;
    Field->Issue(ETacticalCommand::Skip);
    if(!Check(ShowManualTactics() && Field->Presentation().TacticAvailability.Contains(TEXT("行動済み")),TEXT("spent tactics stay visible with their reason"))) return false;
    Field->PrepareTacticTargetPreview(1);Field->Issue(ETacticalCommand::RealtimeMode);
    Field->Issue(ETacticalCommand::QueueTactic);
    auto Saved=std::make_unique<JourneySaveData>();Field->ExportSession(Before);if(!DecodeJourneySave(Before,*Saved)) return false;
    const int Turn=Saved->journey.turn,Food=Saved->journey.world.cities[0].food;
    Field->Issue(ETacticalCommand::RealtimeChainSignal);Field->ExportSession(Before);
    if(!Check(!Field->CanFireManualTactics() && !Field->FireManualTactics() && Field->Presentation().BattleChoiceReason.Contains(TEXT("合図待ち")),TEXT("immediate fire cannot bypass an armed chain signal and explains the wait"))) return false;
    Field->ExportSession(After);if(!Check(Before==After,TEXT("refusing a signaled plan changes no orders or budgets"))) return false;
    Field->Issue(ETacticalCommand::RealtimeChainSignal);
    if(!Check(Field->RealtimePaused() && Field->FireManualTactics() && Field->IsPlaying(),TEXT("paused manual control plays the queued combo immediately"))) return false;
    Field->AdvancePresentation(60);Field->ExportSession(After);if(!DecodeJourneySave(After,*Saved)) return false;
    if(!Check(Field->RealtimePaused() && !Field->IsPlaying() && Saved->journey.turn==Turn && Saved->journey.world.cities[0].food==Food &&
        Saved->journey.lastChain.completed==2 && Saved->journey.lastChain.events[1].comboPercent==125 && Saved->journey.chain.spent[0] && Saved->journey.chain.spent[1],
        TEXT("the real 125-percent follow-up spends both actions and returns paused without a free cycle"))) return false;
    Field->PrepareTacticTargetPreview(0);Field->Issue(ETacticalCommand::AutoBattle);
    if(!Check(!ShowManualTactics(),TEXT("delegation owns its own orders"))) return false;
    Field->Issue(ETacticalCommand::AutoBattle);
    if(!Check(ShowManualTactics() && Field->RealtimePaused(),TEXT("manual takeover immediately brings back the tactic controls"))) return false;
    ToggleTacticPicker();
    if(!Check(bTacticPickerOpen && Field->RealtimePaused(),TEXT("the picker holds the tactical clock paused"))) return false;
    ToggleTacticPicker();Field->PrepareTacticAvailabilityPreview(true);
    if(!Check(ShowManualTactics() && Field->Presentation().TacticAvailability.Contains(TEXT("固有戦法は使用済み")) && Field->Presentation().Tactics.Num()>=4,
        TEXT("a used unique tactic never hides the ordinary loadout"))) return false;
    bDetailedBattle=Detailed;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_MANUAL_TACTICS: PASS (simple visibility, free selection, solo, real 125-percent combo, paused fire, exact budgets, spent reason, takeover and picker pause)"));return true;
}
