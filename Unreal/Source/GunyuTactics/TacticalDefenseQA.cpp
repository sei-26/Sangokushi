#include "TacticalTrial.h"
#include "TacticalSupportQA.h"
#include "TacticalMissionResponse.h"
#include "JourneySave.hpp"
#include <memory>
bool PrepareDefensePreview(ATacticalField* Field,int Variant)
{
    using namespace frontline;
    if(!Field || Variant<0 || Variant>2 || !PrepareSupportCombinationPreview(Field,0)) return false;
    // Authored contact boundary after paid deployment; every following tactic and cycle is real.
    auto Save=std::make_unique<JourneySaveData>();std::vector<std::uint8_t> Bytes;
    if(!Field->ExportSession(Bytes) || !DecodeJourneySave(Bytes,*Save)) return false;
    auto& J=Save->journey;J.world.armies[4].troops=0;
    for(int A=0;A<3;++A) { J.world.armies[A].troops=2000;J.world.armies[A].morale=95; }
    J.RefreshIntelligence();if(!EncodeJourneySave(*Save,Bytes) || !Field->ImportSession(Bytes)) return false;
    Field->SelectOfficer(0);Field->SelectTactic(int(ChainTactic::Feint));Field->Issue(ETacticalCommand::QueueTactic);
    Field->SelectOfficer(1);Field->SelectTactic(int(ChainTactic::Disrupt));Field->Issue(ETacticalCommand::QueueTactic);
    Field->Issue(ETacticalCommand::Execute);if(Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
    Field->AdvanceRealtime(0);Field->AdvancePresentation(20);
    if(!Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
    Field->SelectOfficer(0);
    if(Variant>0) { Field->Issue(ETacticalCommand::RealtimeStep);Field->AdvancePresentation(20); }
    if(Variant>1) { Field->Issue(ETacticalCommand::RealtimeStep);Field->AdvancePresentation(20); }
    const auto& D=Field->Presentation().Defenses;
    if(Field->IsPlaying() || D.Num()!=2) return false;
    if(Variant==0) return D[0].Current.Contains(TEXT("固守待ち")) && D[0].Last.Contains(TEXT("報告なし"));
    if(Variant==1) return D[0].Last.Contains(TEXT("固守なし")) && Field->ReadMissionResponse().Actor==1;
    return D[0].bLastSecured && D[1].bLastSecured;
}
