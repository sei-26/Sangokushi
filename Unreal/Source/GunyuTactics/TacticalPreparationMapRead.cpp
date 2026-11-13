#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyBattlefield.hpp"

void ATacticalField::BuildPreparationMap()
{
    using namespace frontline;
    const auto& J=State->Journey;auto& Map=State->HUD.Journey.Map;
    Map={};Map.Name=JourneyText(BattlefieldDefinition(J.Battlefield()).name);
    Map.Objective=(J.IsRescue() ? FString(TEXT("季安の車を出口へ送り届ける")):JourneyText(J.ObjectiveBrief()))+FString::Printf(TEXT(" ／ 期限%d指揮"),J.TurnBudget());
    for(int Y=1;Y<StrategistJourney::Rows-1;++Y) for(int X=1;X<StrategistJourney::Columns-1;++X)
    {
        const int Tile=Campaign::At(X,Y);
        Map.Cells.Add({Tile,uint8(J.PlanningTerrain(Tile))});
    }
    const auto Starts=J.PreparationPositions(State->PreparationChoice);
    for(int I=0;I<3;++I) Map.Starts.Add({Starts[I],I});
    if(J.HasConvoy())
    {
        Map.Starts.Add({J.ConvoyPath().front(),-1});
        for(int Tile:J.ConvoyPath()) Map.ConvoyRoute.Add(Tile);
    }
    for(int I=0;I<3;++I)
        Map.Duties.Add({J.assignments[I].tile,I,int(J.assignments[I].duty),JourneyText(J.AssignmentStatus(I))});
    Map.Blocked={Campaign::At(2,J.mission==2 ? 6:10),Campaign::At(14,3)};
    Map.bChooseRoute=J.IsRescue();Map.bSouthern=J.convoyRoute==JourneyConvoyRoute::SouthernWood;
    auto Add=[&](int Tile,const TCHAR* Name,const FString& Description,std::initializer_list<FString> Hints)
    { FPlanningSite Site;Site.Tile=Tile;Site.Name=Name;Site.Description=Description;for(const auto& H:Hints) Site.Hints.Add(H);Map.Sites.Add(MoveTemp(Site)); };
    if(J.Definition().kind==JourneyObjective::HoldPositions)
    {
        for(int I=0;I<2;++I)
            Add(I==0 ? J.Definition().goal:J.Definition().secondGoal,I==0 ? TEXT("北の烽火"):TEXT("南の烽火"),
                FString::Printf(TEXT("両方を同時に%d指揮守る。固守は兵500・士気40以上で、隣接敵１隊まで耐えられる。"),J.RequiredHoldTurns()),
                {TEXT("片方だけ守っても役目は果たせない。残る隊を先に決めよう。"),TEXT("救援に出るなら、元いた場所の守りも確かめよ。"),TEXT("呼ばれりゃ走るぜ。でも両方を空っぽにするなよ！")});
    }
    else if(J.HasSunJian())
    {
        Add(J.Definition().goal,TEXT("陽人の辻"),TEXT("一隊を入れ、隣接する敵を退けて確保する。"),
            {TEXT("孫堅が来るまで旗を残そう。"),TEXT("北の林から呂布を引きつける。"),TEXT("南の敵は俺が引き受けるぞ！")});
        Add(YangrenRally,TEXT("孫堅の合流地点"),TEXT("孫堅隊は南から自動で進む。合流地点に味方の隊を置かず、進路を空けておく。"),{});
        Add(SunJianStart(J.coalitionPlan==CoalitionPlan::Protect),TEXT("孫堅隊の出陣位置"),
            J.coalitionPlan==CoalitionPlan::Protect ? TEXT("張邈の案内で峠の手前から出陣する。"):TEXT("孫堅隊は南の街道を進む。"),{});
    }
    else if(J.mission==CoalitionMission)
        Add(J.Definition().goal,TEXT("董卓軍の前哨"),TEXT("北橋を渡り、隣接する敵を退けて一隊を入れる。"),
            {TEXT("先行隊へ合図を送れる場所だ。"),TEXT("前哨の守備を引き離そう。"),TEXT("俺を通す橋を決めてくれ！")});
    else if(J.mission==YellowTurbanMission)
    {
        Add(J.Definition().goal,TEXT("黄巾の前陣"),TEXT("隣接する敵を退け、一隊を入れると占領できる。"),
            {TEXT("占領したら、追う隊にも戻る合図を。"),TEXT("守備隊を前陣から引き離す。"),TEXT("俺を通すなら、南の敵に一発食らわせようぜ！")});
        Add(Campaign::At(10,10),TEXT("南の平地"),TEXT("前陣へ回り込む道。南から来る反撃隊に注意。"),
            {TEXT("正面へ全員を出さず、ここを通る隊を残そう。"),TEXT("南へ敵が向いたなら、正面にも道が開く。"),TEXT("合図が来たら、ここから行くぜ。")});
    }
    else if(J.Definition().kind==JourneyObjective::CaptureCamp)
        Add(J.Definition().goal,TEXT("河港の指揮所"),TEXT("一隊を入れ、隣接する敵を退けて安全に占領すると決着。全滅させたり護送車を待ったりする必要はない。"),
            {TEXT("ここを取れば、本隊が動ける。追撃する隊と占領する隊を決めよう。"),TEXT("斬った数で終わる戦いではない。最後にここへ入るのは誰だ。"),TEXT("橋は俺が抜く！ その後、旗を立てる奴も呼んどけよ！")});
    else
        Add(J.IsRescue() ? Campaign::At(12,11):J.Definition().goal,J.mission==1 ? TEXT("糧車の搬入先"):J.HasConvoy() ? TEXT("護送の出口"):TEXT("退路の出口"),
            J.HasConvoy() ? TEXT("車がここへ着くまで護送は続く。味方で出口を塞がず、敵を寄せつけない配置を考える。"):
                TEXT("安全に確保するか、先鋒を崩して道を開く。撃破だけに頼らない勝ち方も考えられる。"),
            {TEXT("最後に誰がここを空けるかまで、決めておこう。"),TEXT("追撃と任務の達成は同じではない。出口への道を残せ。"),TEXT("突っ込んだ後に戻る役も要るんだな。合図を頼むぜ！")});
    int Bridge=0;
    for(const auto& Cell:Map.Cells)
        if(Cell.Terrain==uint8(Terrain::Bridge))
        {
            const bool Defense=J.Definition().kind==JourneyObjective::HoldPositions;
            const TCHAR* Name=Defense ? (Bridge==0 ? TEXT("西の橋"):TEXT("東の橋")) : (Bridge==0 ? TEXT("北橋"):TEXT("南橋"));
            Add(Cell.Tile,Name,Defense ? TEXT("川の両岸へ援護を送る道。橋に留まる隊と、通り抜ける隊が重ならないように考える。"):
                J.mission==CoalitionMission ? (Bridge==0 ? TEXT("前哨に近い渡河点。占領する隊を通す。"):TEXT("反撃隊の正面に出る渡河点。北橋へ向かう隊を援護できる。")):
                J.Definition().kind==JourneyObjective::CaptureCamp ? TEXT("河港へ攻め入る渡河点。北から回るか、南から突破するか。渡河後に指揮所へ向かう隊を決める。"):
                Bridge==0 ? TEXT("護送路とは別の渡河点。北へ回す隊と、車の側に残す隊を分けて考えられる。"):
                    TEXT("糧車の通る橋。味方が止まっていても車は進めない。渡った後の配置まで考える。"),
                {Defense ? TEXT("橋へ全員を集めると、烽火の守りが空く。分担しよう。"):
                    TEXT("車が渡るまで、誰が道を空けるか考えよう。"),
                 TEXT("橋の先を見よ。渡り始める時と、退く時の道も要る。"),
                 TEXT("狭い所は一隊ずつだ。行く順番を決めようぜ！")});
            ++Bridge;
        }
    if(J.mission==0 || J.mission==YellowTurbanMission)
    {
        Add(J.Definition().positions[1],TEXT("伏兵の森"),J.campaignFlow ? FString(TEXT("関羽の通常出陣位置。先遣伏兵なら、ここから陸路２歩以内の森へ開戦前に入れる。関羽を選び、森林と伏兵を指定しよう。")):FString(TEXT("関羽の出陣位置。森林で伏兵を準備し、敵を近づける配置を考える。敵が動かない場合の手も残す。")),
            {TEXT("敵の目を引くなら、戻る場所も残しておきたい。"),TEXT("森で備える。敵が寄らぬ時の手も考えておけ。"),TEXT("敵が崩れたら俺だ。飛び出す道は空けとけよ！")});
        if(J.IsRescue())
            Add(Campaign::At(9,12),TEXT("南の林道"),TEXT("街道を避ける護送路。案内料120と、１区間多くかかる時間を比べて選ぶ。"),
                {TEXT("遠回りでも、最後まで送り届ける道を選ぼう。"),TEXT("余分な一指揮が要る。糧と、前路の守りを確かめよ。"),TEXT("林道を行くなら、車の前を空ける役も忘れるなよ！")});
    }
}
