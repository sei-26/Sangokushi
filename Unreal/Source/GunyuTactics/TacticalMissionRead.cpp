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
    if(J.depotInterception)
    {
        const auto* Column=J.intelligence[0].Find(4);
        const auto* Guard=J.intelligence[0].Find(3);
        const bool Visible=Column && Column->current;
        const int RouteTile=Visible ? Column->observed.tile+1:-1;
        int Blocker=-1;
        for(int Officer=0;Visible && Officer<3;++Officer)
            if(TacticChain::Living(J.world,Officer) && J.world.armies[Officer].tile==RouteTile) Blocker=Officer;
        const bool Ready=Blocker>=0 && CanOccupyObjective(J.world.armies[Blocker]) && !J.chain.disordered[Blocker];
        H.MissionFocusTile=Visible && Ready && Guard && Guard->current ? Guard->observed.tile:Visible ? RouteTile:J.goal;
        H.MissionAdvice=Ready ? TEXT("いまだ、護衛を崩そう"):Blocker>=0 ? TEXT("封鎖役を立て直そう"):TEXT("車の１マス東を塞ごう");
        const auto* GuardStatus=Guard && Guard->current && Visible ?
            (J.DepotGuardProtects(Column->observed.tile,Guard->observed.tile,Guard->observed.troops,J.chain.disordered[3]) ?
                TEXT("護衛が近い"):TEXT("護衛が離れたか混乱中")):TEXT("護衛の位置を確認");
        const FString Timing=Ready ? JourneyText(J.world.generals[J.world.armies[Blocker].general].name)+TEXT("が道を塞いだ"):
            Blocker>=0 ? FString(TEXT("封鎖役の兵・士気を確認")):FString::Printf(TEXT("次の移動まで%d指揮"),J.DepotMovesIn());
        H.MissionStep=Visible ? FString::Printf(TEXT("出口まで%dマス ｜ %s ｜ %s"),
            J.world.MapDistance(Column->observed.tile,J.goal),*Timing,GuardStatus):
            TEXT("荷駄隊は視界外 ｜ 目標へ / U で東の撤収口を確認");
        H.MissionDetail=TEXT("荷駄隊は２指揮ごとに東へ進む。１マス東に兵150・士気25以上で混乱していない隊を置く。護衛を混乱させるか３マス以上引き離せば捕獲。荷駄隊を潰すと失敗。");
    }
    else if(J.HasSunJian())
    {
        const auto& Ally=J.world.armies[StrategistJourney::AlliedArmy];
        const bool Joined=J.SunJianAtRally();
        const bool Contested=EnemiesNear(Ally.tile)>0;
        H.MissionFocusTile=Joined ? J.goal:Contested ? Ally.tile:YangrenRally;
        H.MissionAdvice=Joined ? TEXT("辻を押さえよう"):Contested ? TEXT("孫堅の前を開こう"):TEXT("辻と南道を見よう");
        H.MissionStep=Joined ? TEXT("孫堅隊が到着 ｜ 敵を退け、辻に一隊を入れよう。"):
            Contested ? TEXT("孫堅隊が交戦中 ｜ 南道を開き、合流を助けよう。"):
            TEXT("孫堅隊が南から進軍中 ｜ 辻を確保し、合流を待とう。");
        H.MissionDetail+=TEXT("\n孫堅隊は毎指揮進む。接敵すると止まるので、南道から敵を引き離そう。");
    }
    else if(J.HasConvoy() && TacticChain::Living(J.world,StrategistJourney::ConvoyArmy))
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
    else if(J.mission==YellowTurbanMission && J.battleReview.guangzong.tracked)
    {
        const auto& Review=J.battleReview.guangzong;
        const int Left=J.GuangzongCommandsLeft();
        if(Left>0 && Review.roadOfficer<0) H.MissionFocusTile=GuangzongRoad;
        H.MissionAdvice=Left>0 && Review.roadOfficer<0 ? TEXT("前陣か、民の街道か"):TEXT("前陣を押さえよう");
        H.MissionStep=Left>0 ? (Review.roadOfficer>=0 ?
            FString::Printf(TEXT("民は退いた ｜ あと%d指揮で前陣を取れば官軍が先行"),Left):
            FString::Printf(TEXT("黄巾の南翼が転進 ｜ あと%d指揮・街道へ回すなら今"),Left)):
            !Review.alarmTurn ? TEXT("前陣に圧力をかけ、黄巾の南翼を動かそう"):
            Review.roadOfficer>=0 ? TEXT("民は避難済み ｜ 前陣を取れば決着"):TEXT("南翼が戻り始めた ｜ 前陣を取れば決着");
        H.MissionDetail=TEXT("前陣を取れば決着。南翼が転進している間に前陣を取ると、次戦で張飛が前から出陣する。間に街道へ一隊を入れ、近くの敵を退ければ民が避難し、次戦で関羽が脇道から出陣する。両方達成できる。");
    }
    else if(J.Definition().kind==JourneyObjective::CaptureCamp)
    {
        const bool Luoyang=J.mission==LuoyangMission;
        H.MissionAdvice=Luoyang ? H.GoalLabel+TEXT("を押さえよう"):J.mission==CoalitionMission ? TEXT("前哨を押さえよう"):
            J.mission==YellowTurbanMission ? TEXT("前陣を押さえよう"):TEXT("指揮所を押さえよう");
        H.MissionStep=(Luoyang ? H.GoalLabel:FString(J.mission==CoalitionMission ? TEXT("前哨占領"):
            J.mission==YellowTurbanMission ? TEXT("前陣占領"):TEXT("河港占領")))+
            (EnemiesNear(J.goal)>0 ? TEXT(" ｜ 付近の敵を離し、占領する隊を通そう。"):
                TEXT(" ｜ 一隊を入れよう。見えていない周囲は確認を。"));
        H.MissionDetail+=TEXT("\n目標へ / U：")+H.GoalLabel+TEXT("を見る。周囲を確保して一隊を入れる。");
        if(J.IsPortAssault()) BuildPortAssaultPresentation();
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
