#include "TacticalPresentation.h"
#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyBattlefield.hpp"
#include <memory>

using namespace frontline;

void ATacticalField::BuildPresentation()
{
    if (IsPlaying()) return; // Do not expose final soldiers or future outcomes during replay.
    const auto& J = State->Journey;
    auto& HUD = State->HUD;
    HUD = {};
    HUD.Mission=J.mission;
    HUD.bBattle = J.stage == JourneyStage::Battle;
    HUD.bWon = J.won;
    HUD.bComplete = J.stage == JourneyStage::Complete;
    HUD.Selected = State->Selected;
    HUD.GoalTile = J.goal;
    HUD.SecondGoalTile = J.Definition().secondGoal;
    HUD.GoalLabel = J.mission == 0 ? TEXT("脱出地点") : J.Definition().kind==JourneyObjective::CaptureCamp ? TEXT("河港の指揮所"):J.mission == 1 ? TEXT("糧車の搬入先") : TEXT("北の烽火");
    HUD.SecondGoalLabel=J.mission==2 ? TEXT("南の烽火"):HUD.GoalLabel;
    HUD.bHasResupplyPlan=J.campaignFlow && J.preparation==1;
    HUD.ResupplyLabel=J.CanResupply() ? TEXT("全軍へ補給  ／ 残り１回"):TEXT("予備糧なし ／ 補給できない");
    HUD.Difficulty=JourneyText(DifficultyName(J.difficulty));
    HUD.Title = FString::Printf(TEXT("%s  /  %s    更新 %d / %d"), *JourneyText(BattlefieldDefinition(J.mission).name), *JourneyText(J.MissionName()),FMath::Max(0,J.turn-1), J.TurnBudget());
    HUD.Objective = JourneyText(J.ObjectiveProgress());
    if(J.stage==JourneyStage::Battle && J.CurrentPledge()!=JourneyPledge::None)
    {
        const FString Main=J.HasConvoy() ? FString::Printf(TEXT("護送 %d/%d区間 / 護衛%d"),J.convoyStep,int(J.ConvoyPath().size())-1,J.world.armies[StrategistJourney::ConvoyArmy].troops)
            : FString::Printf(TEXT("烽火の維持 %d/%d指揮"),J.heldTurns,J.Definition().requiredProgress);
        HUD.Objective=Main+TEXT(" / ")+JourneyText(J.PledgeProgress());
    }
    HUD.Report = State->Report;
    BuildBattleWatchPresentation();
    BuildMarchPresentation();
    HUD.WithdrawLabel = State->bConfirmWithdrawal ? TEXT("撤収を決める / 任務中止") : TEXT("深追いを止め、撤収する");
    HUD.NextLabel = HUD.bBattle ? (State->bConfirmEndTurn ? TEXT("予約を破棄して進める") : TEXT("次の指揮へ  /  Space"))
        : J.stage == JourneyStage::Debrief && J.won ? TEXT("戦果を引き継いで次へ") : TEXT("設定した任務から再開");
    BuildJourneyPresentation();
    if (!HUD.bBattle)
    {
        HUD.Title = TEXT("群雄戦線  /  ") + JourneyText(J.MissionName());
        HUD.Objective = J.stage == JourneyStage::Debrief
            ? FString::Printf(TEXT("%s  帰還兵 %d / 損失 %d / 信頼 %d"), J.won ? TEXT("勝利") : J.withdrawn ? TEXT("任務中止・撤収") : TEXT("敗北"), J.survivors, J.casualties, J.trust)
            : J.stage == JourneyStage::Complete ? TEXT("突破・護送・防衛の３戦を完了") : JourneyText(J.ObjectiveBrief());
        if (J.stage == JourneyStage::Encounter) HUD.Objective = TEXT("出会いから、任された一つの仕事へ");
        if (J.stage == JourneyStage::Preparation && J.IsRescue()) HUD.Objective = FString::Printf(TEXT("作戦の約束: 季安の避難車を東の出口へ護送 / 全軍更新%d回まで"),J.TurnBudget());
        State->Reachable.Reset();
        return;
    }
    for (int Id = 0; Id < 3; ++Id)
    {
        const auto& Unit = J.world.armies[Id];
        FOfficerPresentation Officer;
        Officer.Name = FString::Printf(TEXT("%d  %s"), Id + 1, *JourneyText(J.world.generals[Unit.general].name));
        Officer.Tactic = JourneyText(TacticChain::Name(State->SelectedTactics[Id]));
        Officer.bLiving = Unit.troops > 0;
        Officer.bSelected = Id == State->Selected;
        Officer.Tile = Unit.tile;
        Officer.Command=TEXT("命令待ち");
        if(bRealtimeCommand)
        {
            const auto Kind=State->Realtime.orders[Id].kind;
            Officer.Command=Kind==JourneyDirective::March ? TEXT("行軍指示") : Kind==JourneyDirective::Attack ? TEXT("攻撃指示") :
                Kind==JourneyDirective::Hold ? TEXT("固守指示") : Kind==JourneyDirective::Ambush ? TEXT("伏兵指示") : TEXT("命令待ち");
            if(State->OrderReads[Id].status==JourneyRouteStatus::NoRoute) Officer.Command=TEXT("進路待ち");
            if(State->OrderReads[Id].status==JourneyRouteStatus::LostTarget) Officer.Command=TEXT("再捕捉待ち");
        }
        Officer.Status = FString::Printf(TEXT("兵 %d  士気 %d\n%s / %s%s"), Unit.troops, Unit.morale,
            J.chain.moved[Id] ? TEXT("移動済") : TEXT("移動可"), J.chain.spent[Id] ? TEXT("行動済") : TEXT("行動可"), J.chain.concealed[Id] ? TEXT("\n潜伏中") : TEXT(""));
        for (const auto& Order : State->Orders)
            if (Order.army == Id)
            {
                Officer.Command=State->ChainSignal.Armed() ? TEXT("合図待ち") : TEXT("連鎖予約");
                Officer.Status += State->ChainSignal.Armed() ? (J.chain.concealed[Id] ? TEXT(" / 合図待ち") : TEXT("\n合図待ち")) :
                    J.chain.concealed[Id] ? TEXT(" / 予約済") : TEXT("\n戦法予約済");
                break;
            }
        if(J.assignments[Id].duty!=JourneyDuty::None)
        {
            Officer.Duty=JourneyText(J.AssignmentStatus(Id));
            Officer.Status+=TEXT("\n")+FString(J.assignments[Id].duty==JourneyDuty::Hold ? TEXT("守備"):TEXT("伏兵"))+
                (J.assignments[Id].completed ? TEXT("○"):TEXT("未"));
        }
        HUD.Officers.Add(MoveTemp(Officer));
    }
    const auto* Contact = J.intelligence[0].Find(State->Target);
    HUD.Target = J.HasConvoy() ? TEXT("対象にできる敵なし\n護送路を空け、車を出口まで送ろう") : TEXT("対象にできる敵なし\n見えている敵を指定するか、周囲を確認しよう");
    if (Contact && Contact->current)
        HUD.Target = FString::Printf(TEXT("対象: %s\n兵 %d / 士気 %d / %s"), *JourneyText(J.world.generals[Contact->observed.general].name), Contact->observed.troops, Contact->observed.morale,
            Contact->observed.stance == battle::Stance::Guard ? TEXT("固守") : Contact->observed.scouting ? TEXT("索敵中") : TEXT("通常"));
    BuildTacticsPresentation();
    BuildEnemyPresentation();
    if (J.HasConvoy() && TacticChain::Living(J.world,StrategistJourney::ConvoyArmy))
    {
        HUD.ConvoyRoute.Add(J.world.armies[StrategistJourney::ConvoyArmy].tile);
        const auto& Path=J.ConvoyPath();
        for (int I=J.convoyStep+1;I<static_cast<int>(Path.size());++I) HUD.ConvoyRoute.Add(Path[I]);
    }
    const auto Preview = J.Preview(State->Orders);
    HUD.Predicted = Preview.completed;
    for (int Index = 0; Index < static_cast<int>(State->Orders.size()); ++Index)
    {
        const auto& Order = State->Orders[Index];
        FOrderPresentation Card;
        Card.Actor=Order.army;Card.Target=TacticChain::Definition(Order.tactic).targetsEnemy ? Order.enemy : -1;
        Card.ActorName=JourneyText(J.world.generals[J.world.armies[Order.army].general].name);
        Card.TacticName=JourneyText(TacticChain::Name(Order.tactic));
        Card.Title = FString::Printf(TEXT("%d  %s・%s"), Index + 1, *JourneyText(J.world.generals[Order.army].name), *JourneyText(TacticChain::Name(Order.tactic)));
        if (Index < static_cast<int>(Preview.events.size()))
        {
            const auto& Event = Preview.events[Index];
            Card.bEvaluated=true;
            Card.bSuccess = Event.success;
            if(Event.success) Card.Combo=FString::Printf(TEXT("%dコンボ / %d％"),Event.combo,Event.comboPercent);
            const TCHAR* Effect = Order.tactic == ChainTactic::Feint ? TEXT("士気-12・隙")
                : Order.tactic == ChainTactic::Breakthrough ? (Event.from!=Event.to ? TEXT("押し返す"):TEXT("正面突撃"))
                : Order.tactic == ChainTactic::SerpentRoar ? TEXT("固守解除・混乱")
                : Order.tactic == ChainTactic::Disrupt ? (Event.setupPercent>100 ? TEXT("固守解除・混乱"):TEXT("固守解除"))
                : Order.tactic == ChainTactic::AzureDragon ? TEXT("固守解除・混乱") : Event.setupPercent>100 ? TEXT("混乱"):TEXT("隙");
            Card.Detail = !Event.success ? JourneyText(Event.reason)
                : Order.tactic == ChainTactic::Lure ? TEXT("成立 / 敵を誘導・囮の兵60を消耗")
                : Order.tactic == ChainTactic::Oath ? TEXT("成立 / 自隊・味方鼓舞 / 次撃+20％")
                : FString::Printf(TEXT("成立 / 敵兵 -%d・%s"), Event.damage, Effect);
            if(Event.success && Event.setupPercent>100) Card.Detail+=TEXT(" / 仕込み+25％");
            if (Event.success && Event.from != Event.to && StrategistJourney::OnMap(Event.from) && StrategistJourney::OnMap(Event.to))
                HUD.Routes.Add({Event.from, Event.to});
            if (Event.success && Order.tactic == ChainTactic::Lure)
                HUD.Routes.Add({J.world.armies[Order.army].tile, Order.destination});
        }
        else Card.Detail = TEXT("先行する戦法が停止するため保留。順序や配置を見直そう。");
        HUD.Orders.Add(MoveTemp(Card));
    }
    State->Reachable.Reset();
    if (!HUD.bBattle || !TacticChain::Living(J.world, State->Selected)) return;
    if (State->Input == ETacticalInput::Select) return;
    if(State->Input==ETacticalInput::Position) { HUD.Hint=TEXT("番号のマスを比較。確定するまで移動・戦法は消費しません。");return; }
    if (bRealtimeCommand && State->Input==ETacticalInput::Move)
    {
        for (int Y=1;Y<StrategistJourney::Rows-1;++Y) for (int X=1;X<StrategistJourney::Columns-1;++X)
        {
            const int Tile=Campaign::At(X,Y);
            const auto T=J.world.tiles[Tile].terrain;
            if (T!=Terrain::River && T!=Terrain::Sea && J.world.CityAt(Tile)<0) State->Reachable.Add(Tile);
        }
        HUD.Hint=TEXT("指揮停止中。行軍の目的地をクリック。敵を右クリックすると攻撃を継続。");return;
    }
    // Strip hidden forces before calculating hints. Never expose true hidden occupancy.
    auto Observed = std::make_unique<Campaign>(J.world);
    for (auto& Unit : Observed->armies) if (Unit.faction != 0) Unit.troops = 0;
    for (const auto& Seen : J.intelligence[0].contacts)
        if (Seen.current && Seen.id >= 0 && Seen.id < static_cast<int>(Observed->armies.size())) Observed->armies[Seen.id] = Seen.observed;
    for (int Tile : J.world.MapNeighbors(J.world.armies[State->Selected].tile))
    {
        if (!StrategistJourney::OnMap(Tile) || !J.intelligence[0].visible[Tile]) continue;
        bool bAllowed = false;
        if (State->Input == ETacticalInput::Lure)
        {
            auto Orders = State->Orders;
            Orders.push_back({State->Selected, State->Target, State->SelectedTactics[State->Selected], Tile});
            bAllowed = J.Preview(Orders).completed == static_cast<int>(Orders.size());
        }
        else
        {
            auto Copy = std::make_unique<Campaign>(*Observed);
            auto Chain = J.chain;
            std::u32string Reason;
            bAllowed = Chain.Move(*Copy, J.intelligence[0], State->Selected, Tile, Reason);
        }
        if (bAllowed) State->Reachable.Add(Tile);
    }
    HUD.Hint = State->Reachable.IsEmpty() ? TEXT("候補なし。行動済み・敵の構え・隣接地形・先行予約を確認してください。Escで取消。")
        : State->Input == ETacticalInput::Lure ? TEXT("水色の後退候補をクリックして誘導を予約。Escで取消。") : TEXT("水色の移動候補をクリック。Escで取消。");
}
