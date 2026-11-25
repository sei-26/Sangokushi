#include "TacticalDecisionMoment.h"
#include "StrategistJourney.hpp"
#include "JourneyDefense.hpp"

namespace gunyudecision
{
    using namespace frontline;

    bool CanSeePortOpening(const StrategistJourney& J)
    {
        const auto& Port=J.battleReview.port;
        if(Port.clearedBy<0 || J.turn-Port.clearedOn>StrategistJourney::PortOpeningCycles ||
            !J.intelligence[0].visible[J.goal]) return false;
        for(int Tile:J.world.MapNeighbors(J.goal))
            if(StrategistJourney::OnMap(Tile) && J.world.tiles[Tile].terrain!=Terrain::River &&
                J.world.tiles[Tile].terrain!=Terrain::Sea && !J.intelligence[0].visible[Tile]) return false;
        return true;
    }

    FSnapshot Capture(const StrategistJourney& J)
    {
        FSnapshot R;
        R.bBattle=J.stage==JourneyStage::Battle;
        if(!R.bBattle || !J.campaignFlow) return R;
        const auto& Sight=J.intelligence[0];
        // Modern battles have their own turning points. Do not surface the old
        // small-battle evacuation timer as a choice in the new Guangzong battle.
        if(J.HasGrandStrategy())
        {
            const auto& G=J.battleReview.grand;
            if(G.ralliedOn)
                R.Moment={EKind::GrandRally,J.goal,TEXT("本隊が合流。前陣の旗を残し、敵本陣へ誰を出すか決めよう。")};
            else if(G.openedOn)
                R.Moment={EKind::GrandOpening,J.goal,TEXT("前陣を突破。本隊の行く手を空け、旗を守る隊を残そう。")};
            else if(G.relievedOn && TacticChain::Living(J.world,6))
                R.Moment={EKind::GrandRelief,J.world.armies[6].tile,TEXT("左翼へ救援が届いた。北の追撃を切り上げ、中央へ戻す隊を決めよう。")};
            return R;
        }
        if(J.HasCoalitionBattle())
        {
            const auto& G=J.battleReview.grand;const auto& C=G.coalition;
            if(J.coalitionPlan==CoalitionPlan::Protect)
            {
                if(C.returnedOn)
                    R.Moment={EKind::CoalitionReturned,J.goal,TEXT("曹操と援軍が西岸へ帰還。主力の旗で帰還点を確保しよう。")};
                else if(C.rescuedOn && TacticChain::Living(J.world,6))
                    R.Moment={EKind::GrandRelief,J.world.armies[6].tile,TEXT("援軍へ救援が届いた。南橋は援軍を先に通し、曹操も連れ帰ろう。")};
            }
            else if(G.openedOn)
                R.Moment={EKind::GrandOpening,J.goal,TEXT("対岸に旗が並んだ。曹操を支え、徐栄軍へ続けて攻めよう。")};
            return R;
        }
        if(J.HasYangrenStrategy())
        {
            const auto& G=J.battleReview.grand;const auto& Y=G.yangren;
            const auto* Cavalry=Sight.Find(8);
            if(Y.repelledOn)
                R.Moment={EKind::GrandRally,J.goal,TEXT("騎兵の反撃を退けた。孫堅を支えながら、辻を安全に確保しよう。")};
            else if(Y.counterOn)
                R.Moment={EKind::GrandCounter,Cavalry && Cavalry->current ? Cavalry->observed.tile:J.goal,
                    TEXT("呂布の親衛騎兵が反撃。追撃を止め、孫堅と旗を支え合う隊を決めよう。")};
            else if(!J.SunJianAtRally())
            {
                const int Tile=J.world.armies[StrategistJourney::AlliedArmy].tile;
                if(Sight.EnemyNear(J.world,Tile,1))
                    R.Moment={EKind::SunJianBlocked,Tile,TEXT("孫堅隊が止まった。辻を攻め続けるか、南道の援護へ回るか。")};
            }
            return R;
        }
        if(J.HasConvoy())
        {
            const auto& Path=J.ConvoyPath();
            const int Front=Path[FMath::Min(J.convoyStep+1,int(Path.size())-1)];
            if(Sight.EnemyNear(J.world,Front,1))
                R.Moment={EKind::EscortBlocked,Front,TEXT("護送路に敵。車を守る隊と、敵を引き離す隊を決めよう。")};
        }
        else if(J.IsPortAssault() && CanSeePortOpening(J))
            R.Moment={EKind::PortOpening,J.goal,TEXT("河港に好機。道を開いた武将とは別の隊で旗を立てよう。")};
        else if(J.Definition().kind==JourneyObjective::HoldPositions)
        {
            R.HoldCount=J.heldTurns;
            for(int Tile:{J.goal,J.Definition().secondGoal})
            {
                const auto Check=ReadJourneyDefenseCheck(J,Tile);
                if(Check.turn==J.turn-1 && !Check.secured) { R.LostBeacon=Tile;break; }
            }
        }
        else if(J.mission==YellowTurbanMission && J.GuangzongCommandsLeft()>0 && J.battleReview.guangzong.roadOfficer<0)
            R.Moment={EKind::GuangzongSplit,GuangzongRoad,TEXT("南翼が転進。前陣を急ぐか、民の街道へ一隊回すか。")};
        else if(J.HasSunJian() && !J.SunJianAtRally())
        {
            const int Tile=J.world.armies[StrategistJourney::AlliedArmy].tile;
            if(Sight.EnemyNear(J.world,Tile,1))
                R.Moment={EKind::SunJianBlocked,Tile,TEXT("孫堅隊が止まった。辻を攻め続けるか、南道の援護へ回るか。")};
        }
        else if(J.depotInterception)
        {
            const auto* Column=Sight.Find(4);
            if(Column && Column->current)
            {
                const int Tile=Column->observed.tile+1;
                for(int Id=0;Id<3;++Id)
                    if(J.world.armies[Id].tile==Tile && CanOccupyObjective(J.world.armies[Id]) && !J.chain.disordered[Id])
                    {
                        R.Moment={EKind::DepotIntercept,Tile,TEXT("荷駄の前を塞いだ。車は残して、護衛を崩そう。")};
                        break;
                    }
            }
        }
        return R;
    }

    FMoment Compare(const FSnapshot& Before,const FSnapshot& After)
    {
        if(!Before.bBattle || !After.bBattle) return {};
        if(Before.HoldCount>0 && After.HoldCount==0 && After.LostBeacon>=0)
            return {EKind::BeaconLost,After.LostBeacon,TEXT("烽火の守りが崩れた。空いた翼を見て、誰を戻すか決めよう。")};
        return Before.Moment.Kind!=After.Moment.Kind ? After.Moment:FMoment{};
    }
}
