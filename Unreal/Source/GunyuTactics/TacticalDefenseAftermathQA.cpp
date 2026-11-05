#include "TacticalTrial.h"
#include "TacticalSupportQA.h"
#include "JourneySave.hpp"
#include <memory>
#include <utility>
bool PrepareDefenseAftermathPreview(ATacticalField* Field,int Variant)
{
    using namespace frontline;
    if(!Field || Variant<0 || Variant>5 || !PrepareSupportCombinationPreview(Field,0)) return false;
    // Explicit contact boundary after a paid deployment. Outcomes below use ordinary commands only.
    auto Save=std::make_unique<JourneySaveData>();std::vector<std::uint8_t> Bytes;
    if(!Field->ExportSession(Bytes) || !DecodeJourneySave(Bytes,*Save)) return false;
    auto& J=Save->journey;J.world.armies[4].troops=0;
    // Keep the paid army's real troop totals so settlement and resource return remain valid.
    for(int A=0;A<3;++A) J.world.armies[A].morale=95;
    const int Guard=Variant==5 ? 0:Variant==3 || Variant==4 ? 2:1,Source=Guard==0 ? 1:0;
    if(Guard!=1)
    {
        std::swap(J.world.armies[1].tile,J.world.armies[Guard].tile);
        for(int A:{1,Guard})
        {
            J.world.armies[A].target=J.world.armies[A].tile;
            J.assignments[A]=A==Source ? JourneyAssignment{}:JourneyAssignment{JourneyDuty::Hold,J.world.armies[A].tile,true};
        }
    }
    J.RefreshIntelligence();if(!EncodeJourneySave(*Save,Bytes) || !Field->ImportSession(Bytes)) return false;
    if(Variant!=2)
    {
        Field->SelectOfficer(Source);Field->SelectTactic(int(ChainTactic::Feint));Field->Issue(ETacticalCommand::QueueTactic);
        Field->SelectOfficer(Guard);Field->SelectTactic(int(ChainTactic::Disrupt));Field->Issue(ETacticalCommand::QueueTactic);
        Field->Issue(ETacticalCommand::Execute);if(Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
        Field->AdvanceRealtime(0);Field->Issue(ETacticalCommand::Skip);
        if(!Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
    }
    for(int Cycle=0;Cycle<6 && Field->Presentation().bBattle;++Cycle)
    {
        Field->Issue(ETacticalCommand::RealtimeStep);Field->Issue(ETacticalCommand::Skip);
        if(Variant==1 || Variant>=4) { Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);break; }
    }
    if(!Field->ExportSession(Bytes) || !DecodeJourneySave(Bytes,*Save)) return false;
    const auto& End=Save->journey;const auto& R=End.history[End.mission].review;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_DEFENSE_AFTERMATH_DIAGNOSTIC: variant=%d stage=%d won=%d withdrawn=%d turn=%d held=%d confirmed=%d failures=%d chain=%d activeConfirmed=%d activeFailures=%d"),
        Variant,int(End.stage),int(End.won),int(End.withdrawn),End.turn,End.heldTurns,R.confirmedDefenders,R.spentGuardFailures,R.coordinatedOfficers,End.battleReview.confirmedDefenders,End.battleReview.spentGuardFailures);
    return End.stage==JourneyStage::Debrief && R.confirmedDefenders!=0 &&
        (Variant==1 || Variant>=4 ? End.withdrawn && !End.won:End.won) &&
        (Variant==2 ? R.spentGuardFailures==0:R.spentGuardFailures==(1<<Guard));
}
