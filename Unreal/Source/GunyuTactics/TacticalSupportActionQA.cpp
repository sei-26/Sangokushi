#include "TacticalTrial.h"
#include "TacticalSupportQA.h"
#include "JourneySave.hpp"
#include <memory>
bool PrepareSupportActionPreview(ATacticalField* Field,int Variant)
{
    using namespace frontline;
    auto Save=std::make_unique<JourneySaveData>();std::vector<std::uint8_t> Bytes;
    if(!Field || Variant<0 || Variant>2 || !Field->ExportSession(Bytes) || !DecodeJourneySave(Bytes,*Save)) return false;
    auto& J=Save->journey;if(J.stage!=JourneyStage::Battle || J.Definition().kind!=JourneyObjective::HoldPositions) return false;
    const int Actor=Variant==0 ? 0:2;Save->selected=Actor;Save->target=3;Save->orders.clear();Save->signal.Cancel();Save->watch.Reset();Save->realtime.Reset();
    for(int A=0;A<3;++A) { J.assignments[A]={};J.world.armies[A].tile=J.world.armies[A].target=Campaign::At(11+A,10); }
    J.world.armies[1].tile=J.world.armies[1].target=J.goal;int Enemy=Campaign::At(8,6),At=Campaign::At(9,6);
    if(Variant>0)
    {
        bool Found=false;
        for(int E:J.world.MapNeighbors(J.goal))
        {
            for(int A:J.world.MapNeighbors(E))
            {
                if(A==J.goal || A==J.Definition().secondGoal || !StrategistJourney::OnMap(A)) continue;
                J.world.armies[Actor].tile=A;J.world.armies[3].tile=E;
                const int Push=TacticChain::PushTile(J.world,Actor,3);
                if(!StrategistJourney::OnMap(Push) || J.world.MapDistance(Push,J.goal)<=1 || Push==J.Definition().secondGoal || Push==A) continue;
                Enemy=E;At=A;J.world.tiles[Push].terrain=Terrain::Plain;Found=true;break;
            }
            if(Found) break;
        }
        if(!Found) return false;
    }
    J.world.armies[Actor].tile=J.world.armies[Actor].target=At;
    J.world.armies[3].tile=J.world.armies[3].target=Enemy;J.world.armies[3].troops=1600;J.world.armies[3].morale=Variant==1 ? 55:85;
    J.world.armies[3].stance=battle::Stance::Balanced;
    for(int Tile:J.world.MapNeighbors(At)) if(StrategistJourney::OnMap(Tile)) J.world.tiles[Tile].terrain=Terrain::Plain;
    for(int Tile:{J.goal,At,Enemy}) J.world.tiles[Tile].terrain=Terrain::Plain;
    J.chain.BeginTurn(J.world);J.guarding={};J.RefreshIntelligence();++J.world.revision;
    for(int A=0;A<3;++A) Save->realtime.Queue(J,A,JourneyDirective::Hold);
    return EncodeJourneySave(*Save,Bytes) && Field->ImportSession(Bytes) && !Field->Presentation().BattleChoices.IsEmpty();
}
