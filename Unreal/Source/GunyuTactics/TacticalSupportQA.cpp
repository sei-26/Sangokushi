#include "TacticalSupportQA.h"
#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include <memory>
bool PrepareSupportContactPreview(ATacticalField* Field)
{
    using namespace frontline;
    auto Save=std::make_unique<JourneySaveData>();std::vector<std::uint8_t> Bytes;
    if(!Field || !Field->ExportSession(Bytes) || !DecodeJourneySave(Bytes,*Save)) return false;
    auto& J=Save->journey;int EnemyTile=-1;
    for(int Tile:J.world.MapNeighbors(J.world.armies[0].tile))
    {
        if(J.world.tiles[Tile].terrain==Terrain::River || J.world.tiles[Tile].terrain==Terrain::Sea || !TacticChain::Free(J.world,Tile)) continue;
        for(int A=1;A<3;++A) if(J.world.armies[A].tile!=J.goal && J.world.armies[A].tile!=J.Definition().secondGoal && J.world.MapDistance(Tile,J.world.armies[A].tile)==1) EnemyTile=Tile;
        if(EnemyTile>=0) break;
    }
    if(EnemyTile<0) return false;
    J.world.armies[3].tile=J.world.armies[3].target=EnemyTile;J.world.armies[3].troops=1600;J.world.armies[3].morale=85;
    ++J.world.revision;J.RefreshIntelligence();
    return EncodeJourneySave(*Save,Bytes) && Field->ImportSession(Bytes) && Field->Presentation().BattleChoices.Num()==2 && Field->Presentation().SupportChoices.Num()==2;
}
