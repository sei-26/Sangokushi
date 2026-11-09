#include "TacticalTrial.h"
#include "TacticalBattleResponse.h"
#include "TacticalContributionQA.h"
#include "TacticalSupportQA.h"
#include "JourneySave.hpp"
#include <memory>

using namespace frontline;
namespace
{
    using FBytes=std::vector<std::uint8_t>;
    constexpr int AidOfficer=0,NorthGuard=1,SouthGuard=2,DrawnEnemy=3,OtherEnemy=4;

    bool ReadSession(ATacticalField* Trial,FBytes& Bytes,JourneySaveData& Save)
    { return Trial->ExportSession(Bytes) && DecodeJourneySave(Bytes,Save); }

    bool PrepareContact(ATacticalField* Trial,FBytes& Initial)
    {
        if(!PrepareCooperationCouncilPreview(Trial,2)) return false;
        Trial->Issue(ETacticalCommand::PlanExample);
        Trial->Issue(ETacticalCommand::PlanAmbush);
        Trial->Issue(ETacticalCommand::Deploy);
        if(!PrepareDefenseRetryPreview(Trial,2)) return false;
        Trial->Issue(ETacticalCommand::PlanExample);
        Trial->Issue(ETacticalCommand::Deploy);

        auto Save=std::make_unique<JourneySaveData>();
        if(!ReadSession(Trial,Initial,*Save)) return false;
        auto& J=Save->journey;
        if(J.stage!=JourneyStage::Battle || J.mission!=2 || J.heldTurns!=0 || !Save->beforeBattle ||
           !Save->beforeBattle->retryDefense || Save->beforeBattle->retryDefense->failedGuards!=(1<<AidOfficer) ||
           J.assignments[AidOfficer].duty!=JourneyDuty::None || Save->realtime.orders[AidOfficer].kind!=JourneyDirective::None ||
           J.assignments[NorthGuard].duty!=JourneyDuty::Hold || J.assignments[NorthGuard].tile!=J.goal ||
           J.assignments[SouthGuard].duty!=JourneyDuty::Hold || J.assignments[SouthGuard].tile!=J.Definition().secondGoal ||
           Save->selected!=AidOfficer || !Save->realtimeMode || !Save->realtime.paused) return false;

        // Contact boundary only: place the already-paid forces, preserving strength,
        // terrain, assignments and orders. A lure can draw one of two northern enemies
        // into contact with Liu, who remains outside the beacon's adjacency.
        const int Tiles[]={Campaign::At(9,6),J.goal,J.Definition().secondGoal,Campaign::At(8,6),Campaign::At(7,5)};
        for(int Id=0;Id<5;++Id)
        {
            J.world.armies[Id].tile=J.world.armies[Id].target=Tiles[Id];
            J.world.armies[Id].path.clear();
        }
        // Policy 1 deploys braced enemies. The lure test needs an unbraced
        // target, while the other northern enemy keeps its normal stance.
        J.world.armies[DrawnEnemy].stance=battle::Stance::Balanced;
        ++J.world.revision;
        J.RefreshIntelligence();
        return EncodeJourneySave(*Save,Initial) && Trial->ImportSession(Initial);
    }

    bool UseLureCard(ATacticalField* Trial)
    {
        const auto& Hud=Trial->Presentation();
        const int Index=Hud.BattleChoices.IndexOfByPredicate([](const FTacticalPresentation::FBattleChoice& Card)
        {
            return Card.Actor==AidOfficer && Card.Title.Contains(TEXT("を囮に回す")) && Card.Outcome.Contains(TEXT("隣接敵2→1"));
        });
        if(Index!=INDEX_NONE) return Trial->UseBattleChoice(Index,Hud.BattleChoiceVersion);
        UE_LOG(LogTemp,Warning,TEXT("GUNYU_DEFENSE_RETRY_PAYOFF_DIAGNOSTIC: selected=%d choices=%d reason=%s"),Hud.Selected,Hud.BattleChoices.Num(),*Hud.BattleChoiceReason);
        for(const auto& Card:Hud.BattleChoices)
            UE_LOG(LogTemp,Warning,TEXT("GUNYU_DEFENSE_RETRY_PAYOFF_DIAGNOSTIC: actor=%d title=%s outcome=%s"),Card.Actor,*Card.Title,*Card.Outcome);
        return false;
    }

    bool HasPayoff(const ATacticalField* Trial)
    {
        const auto Speech=Trial->ReadMissionResponse();
        return Speech.Caption==TEXT("北の烽火 / 援護が届いた") && Speech.Actor==NorthGuard && Speech.ReplyActor==AidOfficer &&
            !Speech.Line.IsEmpty() && !Speech.Reply.IsEmpty();
    }

    bool CheckedBoth(const FBattleBeat* Beat,int Turn,bool Assisted)
    {
        if(!Beat || Beat->Kind!=EBattleCue::Enemy || Beat->MissionAfter.DefenseChecks.Num()!=2) return false;
        const auto& North=Beat->MissionAfter.DefenseChecks[0];
        const auto& South=Beat->MissionAfter.DefenseChecks[1];
        return North.Goal==Campaign::At(7,6) && North.Guard==NorthGuard && North.Turn==Turn && North.bGuarded &&
            North.bSecured==Assisted && North.Reason==int(Assisted ? JourneyDefenseReason::Secured:JourneyDefenseReason::Overwhelmed) &&
            South.Goal==Campaign::At(8,9) && South.Guard==SouthGuard && South.Turn==Turn && South.bSecured &&
            Beat->MissionAfter.Held==(Assisted ? 1:0);
    }
}

bool ATacticalController::DefenseRetryPayoffSmokeTest()
{
    auto Check=[](bool Passed,const TCHAR* Why)
    {
        if(!Passed) UE_LOG(LogTemp,Error,TEXT("GUNYU_DEFENSE_RETRY_PAYOFF: FAIL %s"),Why);
        return Passed;
    };
    CloseEpisode();Field->Issue(ETacticalCommand::Skip);
    const bool Legacy=Field->bLegacyJourney;
    FBytes Original,Initial,AfterAid,Natural,Bytes;
    auto Save=std::make_unique<JourneySaveData>();
    if(!Check(Field->ExportSession(Original) && PrepareContact(Field,Initial) && ReadSession(Field,Bytes,*Save),
        TEXT("ordinary retry deployment selects and frees Liu before the shared contact boundary"))) return false;
    const int Turn=Save->journey.turn;
    if(!Check(UseLureCard(Field) && Field->IsPlaying() && !HasPayoff(Field),
        TEXT("visible solo lure executes through the regular card without anticipating a defense result"))) return false;
    Field->AdvancePresentation(20);
    if(!Check(!Field->IsPlaying() && Field->RealtimePaused() && !HasPayoff(Field) && ReadSession(Field,AfterAid,*Save) &&
        Save->journey.lastChain.completed==1 && Save->journey.lastChain.events.front().tactic==ChainTactic::Lure &&
        Save->journey.lastChain.events.front().army==AidOfficer && Save->journey.lastChain.events.front().enemy==DrawnEnemy &&
        Save->journey.world.armies[DrawnEnemy].tile==Campaign::At(9,6) && Save->journey.heldTurns==0 &&
        Save->journey.turn==Turn && Save->journey.chain.spent[AidOfficer] && !Save->journey.chain.spent[NorthGuard] &&
        !Save->journey.chain.spent[SouthGuard],TEXT("actual displacement preserves both guards' actions and waits for a command update"))) return false;
    Field->Issue(ETacticalCommand::RealtimeStep);
    if(!Check(CheckedBoth(Field->CurrentBeat(),Turn,true) && !HasPayoff(Field),
        TEXT("new command resolves both real guards but withholds acknowledgment during playback"))) return false;
    Field->AdvancePresentation(20);
    if(!Check(HasPayoff(Field) && ReadSession(Field,Natural,*Save) && Save->journey.turn==Turn+1 && Save->journey.heldTurns==1,
        TEXT("guard and supporter acknowledge the newly confirmed defense only after playback"))) return false;
    const auto& J=Save->journey;
    const auto* Seen=J.intelligence[0].Find(DrawnEnemy);
    if(!Check(Seen && Seen->current && Seen->observed.troops>0 && J.world.MapDistance(Seen->observed.tile,J.goal)>1 &&
        J.world.armies[NorthGuard].tile==J.goal,TEXT("the drawn enemy remains observed away from the same living guard"))) return false;

    if(!Check(Field->ImportSession(Initial),TEXT("restore the identical no-aid control"))) return false;
    Field->Issue(ETacticalCommand::RealtimeStep);
    if(!Check(CheckedBoth(Field->CurrentBeat(),Turn,false),TEXT("without the lure two enemies overwhelm the northern guard"))) return false;
    Field->AdvancePresentation(20);
    if(!Check(!HasPayoff(Field),TEXT("same setup without aid never invents an acknowledgment"))) return false;

    if(!Check(Field->ImportSession(AfterAid),TEXT("reload settled solo aid before the next check"))) return false;
    Field->Issue(ETacticalCommand::RealtimeStep);
    Field->AdvancePresentation(20);
    if(!Check(!HasPayoff(Field) && Field->ExportSession(Bytes) && Bytes==Natural,
        TEXT("loading clears pending speech evidence and preserves the real next-check outcome"))) return false;

    if(!Check(Field->ImportSession(Initial) && UseLureCard(Field),TEXT("repeat the actual aid for skip comparison"))) return false;
    Field->AdvancePresentation(20);
    Field->Issue(ETacticalCommand::RealtimeStep);
    Field->Issue(ETacticalCommand::Skip);
    if(!Check(!HasPayoff(Field) && Field->ExportSession(Bytes) && Bytes==Natural,
        TEXT("skipping the confirmation removes speech while keeping the same battle result"))) return false;

    if(!Check(DecodeJourneySave(Initial,*Save) && Save->beforeBattle.has_value(),TEXT("read the same board for the ordinary-support control"))) return false;
    Save->beforeBattle->retryDefense.reset();
    if(!Check(EncodeJourneySave(*Save,Bytes) && Field->ImportSession(Bytes) && UseLureCard(Field),
        TEXT("the same ordinary aid can run without the retry lesson"))) return false;
    Field->AdvancePresentation(20);
    Field->Issue(ETacticalCommand::RealtimeStep);
    if(!Check(CheckedBoth(Field->CurrentBeat(),Turn,true),TEXT("ordinary aid still secures both wings by the same rules"))) return false;
    Field->AdvancePresentation(20);
    if(!Check(!HasPayoff(Field),TEXT("ordinary aid cannot claim the retry-specific exchange"))) return false;

    Field->bLegacyJourney=Legacy;
    if(!Check(Field->ImportSession(Original),TEXT("restore the caller's original session"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_DEFENSE_RETRY_PAYOFF: PASS (actual retry and solo lure, fresh same-wing confirmation, two-enemy no-aid control, ordinary aid, load and skip)"));
    return true;
}

bool ATacticalController::PrepareDefenseRetryPayoffCapture(int Variant)
{
    if(Variant<0 || Variant>1) return false;
    CloseEpisode();Field->Issue(ETacticalCommand::Skip);
    FBytes Initial;
    if(!PrepareContact(Field,Initial)) return false;
    auto Save=std::make_unique<JourneySaveData>();
    if(!DecodeJourneySave(Initial,*Save)) return false;
    const bool Assisted=Variant==0;
    if(Assisted)
    {
        if(!UseLureCard(Field)) return false;
        Field->AdvancePresentation(20);
    }
    Field->Issue(ETacticalCommand::RealtimeStep);
    if(!CheckedBoth(Field->CurrentBeat(),Save->journey.turn,Assisted)) return false;
    Field->AdvancePresentation(20);
    Focus=ATacticalField::TileLocation(Campaign::At(8,6));Distance=2750;LabelLayoutFrame=MAX_uint64;
    return HasPayoff(Field)==Assisted;
}
