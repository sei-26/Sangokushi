#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
using namespace frontline;
void ATacticalField::PrepareBattlefieldPreview(int Variant)
{
    auto Save=std::make_unique<JourneySaveData>();Save->journey.ResetCampaign();Save->journey.Meet();Save->beforeBattle=Save->journey;
    Save->journey.Prepare(Variant==2 ? 0:1,0);auto& J=Save->journey;
    if(Variant!=2)
    {
        const int target=Variant==0 ? Campaign::At(9,9):Campaign::At(10,10);
        J.world.armies[0].tile=J.world.armies[0].target=Variant==0 ? Campaign::At(8,9):Campaign::At(9,10);
        J.world.armies[1].tile=J.world.armies[1].target=Variant==0 ? Campaign::At(7,11):Campaign::At(11,10);
        J.world.armies[2].tile=J.world.armies[2].target=Campaign::At(8,12);
        J.world.armies[3].tile=J.world.armies[3].target=target;J.world.armies[3].troops=6000;J.world.armies[3].morale=100;
        J.world.armies[3].stance=battle::Stance::Guard;J.chain.concealed[1]=false;
        J.RefreshIntelligence();Save->tactics[0]=ChainTactic::Feint;
    }
    std::vector<std::uint8_t> Bytes;if(EncodeJourneySave(*Save,Bytes)) ImportSession(Bytes);
}
bool ATacticalField::BattlefieldTacticsSmokeTest()
{
    const bool Legacy=bLegacyJourney,Playback=State->bNoPlayback;bLegacyJourney=false;State->bNoPlayback=false;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_BATTLEFIELD_TACTICS: FAIL %s"),Why);return V; };
    PrepareBattlefieldPreview(0);
    if(!Check(State->HUD.BattlefieldHint.Contains(TEXT("固守")),TEXT("simple command face explains the observed guard"))) return false;
    const int Predicted=State->Journey.Preview({{0,3,ChainTactic::Feint,-1}}).events[0].damage;
    Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);
    if(!Check(IsPlaying() && State->Journey.lastChain.events[0].damage==Predicted && CurrentBeat()->Reason.Contains(TEXT("森林")) && CurrentBeat()->Reason.Contains(TEXT("固守")),TEXT("actual covered/guarded damage matches the sanitized preview and reaches playback"))) return false;
    Issue(ETacticalCommand::Skip);PrepareBattlefieldPreview(1);
    if(!Check(State->HUD.BattlefieldHint.Contains(TEXT("挟撃")),TEXT("separated real fronts are discoverable without another control"))) return false;
    Issue(ETacticalCommand::Resupply);AdvancePresentation(6.f);
    Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);
    if(!Check(IsPlaying() && ReadTacticCinema().Voice.Contains(TEXT("糧")),TEXT("a real post-supply tactic answers the council in the cinematic"))) return false;
    const int Revision=State->Journey.world.revision,Steps=State->Journey.battleReview.postSupplySteps;
    ReadTacticCinema();ReadTacticCinema();
    if(!Check(State->Journey.world.revision==Revision && Steps==1 && State->Journey.battleReview.postSupplySteps==Steps,TEXT("reading the payoff grants no extra action, effect or history"))) return false;
    Issue(ETacticalCommand::Skip);PrepareBattlefieldPreview(2);
    SelectOfficer(0);SelectTactic(int(ChainTactic::Lure));SelectAt(Campaign::At(10,10));Issue(ETacticalCommand::QueueTactic);CommandAt(Campaign::At(8,11));
    SelectOfficer(1);SelectTactic(int(ChainTactic::Ambush));SelectAt(Campaign::At(10,10));Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);AdvancePresentation(3.1f);
    if(!Check(IsPlaying() && CurrentBeat()->Kind==EBattleCue::Ambush && ReadTacticCinema().Voice.Contains(TEXT("仕込み")),TEXT("actual prepared opening ambush responds to the chosen council plan"))) return false;
    Issue(ETacticalCommand::Skip);bLegacyJourney=Legacy;State->bNoPlayback=Playback;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_BATTLEFIELD_TACTICS: PASS (guard/cover/angles, observed preview to actual playback, simple hints, real ambush/supply council payoffs, immutable reads)"));return true;
}
