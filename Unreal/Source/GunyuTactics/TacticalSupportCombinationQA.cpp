#include "TacticalTrial.h"
#include "TacticalSupportQA.h"
#include "JourneySave.hpp"
#include "JourneySupportActions.hpp"
#include <memory>
bool PrepareSupportCombinationPreview(ATacticalField* Field,int Variant)
{
    using namespace frontline;
    auto Save=std::make_unique<JourneySaveData>();std::vector<std::uint8_t> Bytes;
    if(!Field || Variant<0 || Variant>2 || !Field->ExportSession(Bytes) || !DecodeJourneySave(Bytes,*Save)) return false;
    auto& J=Save->journey;if(J.stage!=JourneyStage::Battle || J.Definition().kind!=JourneyObjective::HoldPositions) return false;
    // Explicit tactical test positions after an ordinary deployment, never a fabricated settled battle.
    for(int Y=4;Y<=7;++Y) for(int X=4;X<=11;++X) J.world.tiles[Campaign::At(X,Y)].terrain=Terrain::Plain;
    J.world.armies[0].tile=J.world.armies[0].target=Campaign::At(6,6);
    J.world.armies[1].tile=J.world.armies[1].target=J.goal;
    J.world.armies[2].tile=J.world.armies[2].target=J.Definition().secondGoal;
    J.world.armies[3].tile=J.world.armies[3].target=Campaign::At(6,5);J.world.armies[3].troops=Variant==1 ? 350:3000;
    J.world.armies[3].morale=90;J.world.armies[3].stance=battle::Stance::Balanced;
    J.assignments[0]={};J.assignments[1]={JourneyDuty::Hold,J.goal,true};J.assignments[2]={JourneyDuty::Hold,J.Definition().secondGoal,true};
    J.chain.BeginTurn(J.world);J.guarding={};for(int A=0;A<3;++A) J.chain.concealed[A]=false;
    Save->selected=0;Save->target=3;Save->orders.clear();Save->signal.Cancel();Save->watch.Reset();Save->realtime.Reset();Save->realtimeMode=true;
    for(int A=0;A<3;++A) Save->realtime.Queue(J,A,JourneyDirective::Hold);
    J.RefreshIntelligence();++J.world.revision;
    if(!EncodeJourneySave(*Save,Bytes) || !Field->ImportSession(Bytes)) return false;
    if(Variant==2)
    {
        Field->SelectTactic(int(ChainTactic::Feint));Field->Issue(ETacticalCommand::QueueTactic);Field->Issue(ETacticalCommand::Execute);
        Field->Issue(ETacticalCommand::RealtimePause);Field->AdvanceRealtime(0);Field->AdvancePresentation(20);
        if(!Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
        Field->SelectOfficer(0);
    }
    if(!Field->ExportSession(Bytes) || !DecodeJourneySave(Bytes,*Save)) return false;
    const auto Choices=ReadJourneySupportActions(Save->journey,Save->realtime,0);
    for(const auto& C:Choices) if(C.guardActs && C.followup==(Variant==2) && C.plan.orders.size()==std::size_t(Variant==2 ? 1:2)) return true;
    return false;
}
