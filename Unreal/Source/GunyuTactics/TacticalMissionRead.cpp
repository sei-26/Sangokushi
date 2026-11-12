#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyDefense.hpp"
using namespace frontline;
void ATacticalField::BuildMissionPresentation()
{
    auto& H=State->HUD;const auto& J=State->Journey;
    if(J.stage!=JourneyStage::Battle) return;
    H.MissionFocusTile=J.goal;H.bConvoyMission=J.HasConvoy();
    H.MissionDetail=H.Objective;
    auto EnemiesNear=[&](int Tile)
    {
        int Count=0;
        for(const auto& Seen:J.intelligence[0].contacts)
            if(Seen.current && Seen.observed.troops>0 && J.world.Hostile(0,Seen.observed.faction) && J.world.MapDistance(Seen.observed.tile,Tile)<=1) ++Count;
        return Count;
    };
    if(J.HasConvoy() && TacticChain::Living(J.world,StrategistJourney::ConvoyArmy))
    {
        const auto& Path=J.ConvoyPath();const int Next=Path[FMath::Min(J.convoyStep+1,int(Path.size())-1)];
        H.MissionFocusTile=Next;
        const FString Progress=FString::Printf(TEXT("護送 %d/%d ｜ "),J.convoyStep,int(Path.size())-1);
        if(EnemiesNear(Next)>0)
        {
            H.MissionAdvice=TEXT("まず、車道を空けよう");
            H.MissionStep=Progress+TEXT("前路の敵を退け、車の通る道をつくろう。");
        }
        else
        {
            int Blocker=-1;
            for(int A=0;A<3;++A) if(TacticChain::Living(J.world,A) && J.world.armies[A].tile==Next) { Blocker=A;break; }
            if(Blocker>=0)
            {
                H.MissionAdvice=TEXT("味方も、車道の脇へ");
                H.MissionStep=Progress+JourneyText(J.world.generals[J.world.armies[Blocker].general].name)+TEXT("隊が進路上。脇へ移して車を通そう。");
            }
            else if(!J.intelligence[0].visible[Next])
            {
                H.MissionAdvice=TEXT("前路を確かめよう");
                H.MissionStep=Progress+TEXT("次の道は未確認。視界を広げ、車を守ろう。");
            }
            else
            {
                H.MissionAdvice=TEXT("車を出口へ届けよう");
                H.MissionStep=Progress+TEXT("見えている前路は空いた。次の指揮で車を進めよう。");
            }
        }
        H.MissionDetail+=TEXT("\n目標へ / U：車の次の区間を見る。道を空けて指揮を進めるか、護送を武将たちに任せる。視界外の敵は不明。");
    }
    else if(J.Definition().kind==JourneyObjective::HoldPositions)
    {
        BuildDefensePresentation();
        const bool North=ReadJourneyDefense(J,J.goal).secured,South=ReadJourneyDefense(J,J.Definition().secondGoal).secured;
        H.MissionFocusTile=!North ? J.goal:!South ? J.Definition().secondGoal:J.goal;
        H.MissionAdvice=North && South ? TEXT("両翼を守り抜こう"):!North ? TEXT("北の守備を整えよう"):TEXT("南の守備を整えよう");
        H.MissionStep=FString::Printf(TEXT("烽火 %d/%d ｜ %s"),J.heldTurns,J.RequiredHoldTurns(),North && South ?
            TEXT("両翼を守る配置。次の指揮でも維持しよう。"):!North ? TEXT("北の守備を整えよう。占位・兵・士気と近くの敵を確認。"):TEXT("南の守備を整えよう。占位・兵・士気と近くの敵を確認。"));
        H.MissionDetail+=TEXT("\n目標へ / U：守備を整えたい烽火を見る。判定の案内は現在の捕捉に基づき、視界外の敵は不明。");
    }
    else if(J.Definition().kind==JourneyObjective::CaptureCamp)
    {
        H.MissionAdvice=J.mission==CoalitionMission ? TEXT("前哨を押さえよう"):
            J.mission==YellowTurbanMission ? TEXT("前陣を押さえよう"):TEXT("指揮所を押さえよう");
        H.MissionStep=FString(J.mission==CoalitionMission ? TEXT("前哨占領"):J.mission==YellowTurbanMission ? TEXT("前陣占領"):TEXT("河港占領"))+
            (EnemiesNear(J.goal)>0 ? TEXT(" ｜ 付近の敵を離し、占領する隊を通そう。"):
                TEXT(" ｜ 一隊を入れよう。見えていない周囲は確認を。"));
        H.MissionDetail+=TEXT("\n目標へ / U：")+H.GoalLabel+TEXT("を見る。周囲を確保して一隊を入れる。");
    }
    else
    {
        const auto* Front=J.intelligence[0].Find(3);
        if(Front && Front->current && Front->observed.troops>0) H.MissionFocusTile=Front->observed.tile;
        H.MissionAdvice=J.preparation==0 && J.chain.concealed[1] ? TEXT("関羽の伏兵を一撃へ"):TEXT("先鋒を崩し、退路を開こう");
        H.MissionStep=TEXT("退路確保 ｜ 先鋒を崩すか、東の出口へ一隊を通そう。");
        H.MissionDetail+=TEXT("\n目標へ / U：捕捉中の先鋒を見る。先鋒が見えなければ、公開された出口を見る。");
    }
    if(J.preparation==0)
        H.MissionDetail+=J.chain.concealed[1] ? TEXT("\n軍議の先遣伏兵：関羽が潜伏中。動けば潜伏が解ける。"):TEXT("\n軍議の先遣伏兵：関羽は今は潜伏していない。現在の位置から打てる策を比べよう。");
    else H.MissionDetail+=J.CanResupply() ? TEXT("\n軍議の予備糧：一度の補給を使える。詳細の采配から配る時を選ぼう。"):TEXT("\n軍議の予備糧：今は補給を使えない。使用済みか残兵を確認しよう。");
}
