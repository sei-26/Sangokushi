#include "TacticalTrial.h"
#include "TacticalState.h"

using namespace frontline;

const FTacticalPresentation& ATacticalField::Presentation() const { return State->HUD; }
FString ATacticalField::Summary() const { return State->HUD.Title + TEXT("\n") + State->HUD.Objective; }
FString ATacticalField::Detail() const { return State->HUD.Report; }

bool ATacticalField::CanIssue(ETacticalCommand Command) const
{
    if (IsRealtimeAction(Command)) return CanRealtimeIssue(Command);
    if (Command == ETacticalCommand::Skip) return IsPlaying();
    if (IsPlaying()) return false;
    const auto& J = State->Journey;
    if (Command == ETacticalCommand::Restart || Command == ETacticalCommand::Cancel) return true;
    if (Command == ETacticalCommand::Continue) return J.CanContinue();
    if(Command==ETacticalCommand::StoryReplyPreserve || Command==ETacticalCommand::StoryReplyCoordinate) return J.NeedsStoryReply();
    if (Command == ETacticalCommand::Dialogue) return J.stage == JourneyStage::Encounter;
    if(Command==ETacticalCommand::DifficultyStory || Command==ETacticalCommand::DifficultyStandard) return J.stage==JourneyStage::Preparation || J.stage==JourneyStage::Encounter;
    if(Command==ETacticalCommand::PlanExample) return J.stage==JourneyStage::Preparation;
    if (Command == ETacticalCommand::PlanAmbush || Command == ETacticalCommand::PlanSupplies) return J.stage == JourneyStage::Preparation;
    if (Command == ETacticalCommand::Deploy) return J.CanPrepare(State->PreparationChoice);
    if (Command == ETacticalCommand::Reinforce)
        return J.stage == JourneyStage::Preparation && !J.reinforced && J.world.cities[0].gold >= 250 && J.world.cities[0].food >= 500;
    if (Command == ETacticalCommand::Retry) return J.stage == JourneyStage::Debrief && !J.won && State->BeforeBattle != nullptr;
    if (Command == ETacticalCommand::PromiseRoad || Command == ETacticalCommand::PromiseRescue) return J.stage == JourneyStage::Preparation && J.mission == 0;
    if (Command==ETacticalCommand::RouteRoad || Command==ETacticalCommand::RouteSouthernWood) return J.stage==JourneyStage::Preparation && J.IsRescue();
    if (Command == ETacticalCommand::TalkLiu || Command == ETacticalCommand::TalkGuan || Command == ETacticalCommand::TalkZhang)
        return J.stage == JourneyStage::Preparation || J.stage == JourneyStage::Debrief;
    if (J.stage != JourneyStage::Battle) return false;
    if (Command == ETacticalCommand::EndTurn || Command == ETacticalCommand::Withdraw) return true;
    if (Command == ETacticalCommand::Clear) return !State->Orders.empty();
    if (Command == ETacticalCommand::Execute) return !State->Orders.empty();
    if (Command == ETacticalCommand::Sample) return J.mission == 0 && J.turn == 1;
    const int Id = State->Selected;
    if (!TacticChain::Living(J.world, Id) || Id >= 3) return false;
    if (Command == ETacticalCommand::Move) return !J.chain.moved[Id];
    if (J.chain.spent[Id]) return false;
    if (Command == ETacticalCommand::Hold) return !J.chain.moved[Id];
    if (Command == ETacticalCommand::PrepareAmbush)
        return J.world.generals[Id].tactic == Tactic::Ambush && !J.chain.moved[Id] && !J.chain.concealed[Id];
    if (Command == ETacticalCommand::QueueTactic) return TacticReservationBlockReason().IsEmpty();
    const auto* Contact = J.intelligence[0].Find(State->Target);
    if (!Contact || !Contact->current) return false;
    if (Command == ETacticalCommand::Strike) return J.world.CanStrike(Id, Contact->observed.tile);
    return false;
}

void ATacticalField::SelectOfficer(int Index)
{
    if ((IsPlaying() && !bRealtimeCommand) || State->Journey.stage != JourneyStage::Battle) return;
    if (Index < 0 || Index >= 3 || !TacticChain::Living(State->Journey.world, Index)) return;
    State->Selected = Index;
    State->bConfirmWithdrawal = false;
    State->Input = ETacticalInput::Select;
    State->bConfirmEndTurn = false;
    Refresh();
}

void ATacticalField::SelectAt(int Tile)
{
    if ((IsPlaying() && !bRealtimeCommand) || State->Journey.stage != JourneyStage::Battle) return;
    auto& J = State->Journey;
    if (bRealtimeCommand && IsPlaying())
    {
        for (const auto& Unit : State->Units)
        {
            if (Unit.Id<0 || Unit.Troops<=0) continue;
            const auto Cell=hexgrid::Pick(Unit.Position.X/200.,Unit.Position.Y/200.);
            if (frontline::Campaign::At(Cell.first,Cell.second)!=Tile) continue;
            if (!Unit.bEnemy && Unit.Id<3) SelectOfficer(Unit.Id);
            else if (Unit.bEnemy) { State->Target=Unit.Id;State->Input=ETacticalInput::Select;Refresh(); }
            return;
        }
        return;
    }
    for (int Id = 0; Id < 3; ++Id)
        if (TacticChain::Living(J.world, Id) && J.world.armies[Id].tile == Tile)
        { SelectOfficer(Id); return; }
    for (const auto& Contact : J.intelligence[0].contacts)
        if (Contact.current && Contact.observed.tile == Tile)
        {
            State->Target = Contact.id;
            State->Input = ETacticalInput::Select;
            State->bConfirmWithdrawal = false;
            State->bConfirmEndTurn = false;
            Refresh();
            return;
        }
}
void ATacticalField::ClickTile(int Tile)
{
    if (State->Input == ETacticalInput::Select) SelectAt(Tile);
    else CommandAt(Tile);
}

void ATacticalField::CommandAt(int Tile)
{
    if (bRealtimeCommand && State->Journey.stage==JourneyStage::Battle && State->Input!=ETacticalInput::Lure)
    {
        for (const auto& Unit : State->Units)
        {
            if (!Unit.bEnemy || Unit.Id<0 || Unit.Troops<=0) continue;
            const auto Cell=hexgrid::Pick(Unit.Position.X/200.,Unit.Position.Y/200.);
            if (frontline::Campaign::At(Cell.first,Cell.second)==Tile)
            { State->Target=Unit.Id;Issue(ETacticalCommand::Strike);return; }
        }
        const bool Queued=State->Realtime.Queue(State->Journey,State->Selected,frontline::JourneyDirective::March,Tile);
        if (Queued) State->Input=ETacticalInput::Select;
        State->Report=Queued ? TEXT("行軍を指示。各更新で進路を見直し、到着すると停止する。") : TEXT("ここへは行軍できません。川・都市・範囲外を避けてください。");
        Refresh();return;
    }
    if (IsPlaying()) return;
    auto& J = State->Journey;
    if (J.stage != JourneyStage::Battle || !StrategistJourney::OnMap(Tile)) return;
    State->bConfirmEndTurn = false;
    State->bConfirmWithdrawal = false;
    if (State->Input == ETacticalInput::Lure)
    {
        // Only offer withdrawal cells that the same observed-state preview accepts.
        if (!State->Reachable.Contains(Tile))
            State->Report = TEXT("水色の後退候補を選んでください。敵から離れる隣接マスが必要です。");
        else if (CanIssue(ETacticalCommand::QueueTactic))
        {
            State->Orders.push_back({State->Selected, State->Target, State->SelectedTactics[State->Selected], Tile});
            State->Input = ETacticalInput::Select;
            State->Report = TEXT("誘導を予約。関羽の伏兵、張飛の突破を続けて組めます。");
        }
    }
    else
    {
        const auto Before = ObserveUnits();
        const bool bMoved = J.Move(State->Selected, Tile);
        if (bMoved) State->Input = ETacticalInput::Select;
        State->Report = JourneyText(J.message);
        if (bMoved) QueueAction(EBattleCue::March, State->Selected, -1, Before);
    }
    Refresh();
}

void ATacticalField::EditOrder(int Index, int Direction)
{
    if (IsPlaying()) return;
    State->bConfirmWithdrawal = false;
    if (State->Journey.stage != JourneyStage::Battle || Index < 0 || Index >= static_cast<int>(State->Orders.size())) return;
    if (Direction == 0) State->Orders.erase(State->Orders.begin() + Index);
    else
    {
        const int Other = Index + (Direction < 0 ? -1 : 1);
        if (Other < 0 || Other >= static_cast<int>(State->Orders.size())) return;
        std::swap(State->Orders[Index], State->Orders[Other]);
    }
    State->bConfirmEndTurn = false;
    State->Input = ETacticalInput::Select;
    State->Report = TEXT("予約を変更。順序に沿って成立条件を再計算しました。");
    Refresh();
}

void ATacticalField::Issue(ETacticalCommand Command)
{
    auto& J = State->Journey;
    if (!CanIssue(Command)) return;
    if (IsRealtimeAction(Command)) { IssueRealtime(Command);return; }
    if (Command==ETacticalCommand::DifficultyStory || Command==ETacticalCommand::DifficultyStandard || Command==ETacticalCommand::PlanExample || Command == ETacticalCommand::Dialogue || Command == ETacticalCommand::Reinforce || Command == ETacticalCommand::PlanAmbush || Command == ETacticalCommand::PlanSupplies || Command == ETacticalCommand::Deploy || Command == ETacticalCommand::Retry || Command == ETacticalCommand::PromiseRoad || Command == ETacticalCommand::PromiseRescue || Command == ETacticalCommand::TalkLiu || Command == ETacticalCommand::TalkGuan || Command == ETacticalCommand::TalkZhang || Command==ETacticalCommand::RouteRoad || Command==ETacticalCommand::RouteSouthernWood || Command==ETacticalCommand::StoryReplyPreserve || Command==ETacticalCommand::StoryReplyCoordinate)
    { IssueJourney(Command); return; }
    if (bRealtimeCommand && (Command==ETacticalCommand::QueueTactic || Command==ETacticalCommand::Sample)) State->Realtime.paused=true;
    const auto Before = ObserveUnits();
    if (Command != ETacticalCommand::Withdraw) State->bConfirmWithdrawal = false;
    if (Command != ETacticalCommand::EndTurn) State->bConfirmEndTurn = false;
    switch (Command)
    {
    case ETacticalCommand::Move:
        State->Input = ETacticalInput::Move;
        State->Report = TEXT("水色のマスをクリックして移動。各隊１指揮に１マス。Escで取消。");
        break;
    case ETacticalCommand::Strike:
    {
        const bool bSuccess = J.Strike(State->Selected, State->Target);
        State->Input = ETacticalInput::Select; State->Report = JourneyText(J.message);
        if (bSuccess) QueueAction(EBattleCue::Strike, State->Selected, State->Target, Before);
        break;
    }
    case ETacticalCommand::PrepareAmbush:
    {
        const bool bSuccess = J.Ambush(State->Selected);
        State->Input = ETacticalInput::Select; State->Report = JourneyText(J.message);
        if (bSuccess) QueueAction(EBattleCue::Conceal, State->Selected, -1, Before);
        break;
    }
    case ETacticalCommand::Hold:
    {
        const bool bSuccess = J.Hold(State->Selected);
        State->Input = ETacticalInput::Select; State->Report = JourneyText(J.message);
        if (bSuccess) QueueAction(EBattleCue::Guard, State->Selected, -1, Before);
        break;
    }
    case ETacticalCommand::QueueTactic:
    {
        const auto Tactic = State->SelectedTactics[State->Selected];
        if (Tactic == ChainTactic::Lure)
        {
            State->Input = ETacticalInput::Lure;
            State->Report = TEXT("誘導の後退先を水色の候補から選ぶ。後続の戦法は、敵が移動した後の配置で判定されます。");
        }
        else
        {
            State->Orders.push_back({State->Selected, TacticChain::Definition(Tactic).targetsEnemy ? State->Target : -1, Tactic, -1});
            State->Input = ETacticalInput::Select;
            State->Report = TEXT("選んだ戦法を予約。未成立の手は、先行する誘導・牽制・崩しなどで条件を作ってください。");
        }
        break;
    }
    case ETacticalCommand::Sample:
        State->Orders = {{0, 3, ChainTactic::Lure, Campaign::At(8, 11)}, {1, 3, ChainTactic::Ambush, -1}, {2, 3, ChainTactic::Breakthrough, -1}};
        State->Input = ETacticalInput::Select;
        State->Report = TEXT("誘導で敵を伏兵の隣へ動かし、伏兵で混乱させ、突破で押し返す。↑↓で順序を変えて比較できます。");
        break;
    case ETacticalCommand::Execute:
    {
        DispatchQueuedChain();
        break;
    }
    case ETacticalCommand::EndTurn:
        if (!State->Orders.empty() && !State->bConfirmEndTurn)
        {
            State->bConfirmEndTurn = true;
            State->Report = TEXT("未発動の予約があります。「予約を破棄して進める」で敵の指揮へ。Escで取り消せます。");
            break;
        }
        J.EndTurn(); State->Orders.clear(); State->bConfirmEndTurn = false;
        State->Input = ETacticalInput::Select; State->Report = JourneyText(J.message);
        QueueAction(EBattleCue::Enemy, -1, -1, Before); break;
    case ETacticalCommand::Withdraw:
        if (!State->bConfirmWithdrawal)
        {
            State->bConfirmWithdrawal = true;
            State->Report = TEXT("任務を中止して生存兵と残りの糧を回収します。約束は未達になります。もう一度「撤収を決める」で実行、Escで取り消します。");
            break;
        }
        J.Withdraw(); State->Orders.clear(); State->Input = ETacticalInput::Select;
        State->bConfirmWithdrawal = false; State->Report = JourneyText(J.message);
        QueueAction(EBattleCue::Withdraw, -1, -1, Before); break;
    case ETacticalCommand::Clear:
        State->Orders.clear(); State->Input = ETacticalInput::Select;
        State->Report = TEXT("予約を取り消しました。部隊の行動は消費していません。"); break;
    case ETacticalCommand::Cancel:
        State->Input = ETacticalInput::Select;
        State->Report = TEXT("対象指定を取り消しました。予約済みの戦法は保持しています。"); break;
    case ETacticalCommand::Continue:
        if(J.Continue())
        {
            if(!bLegacyJourney) J.UpgradeCampaignAtCouncil();
            if(J.stage==JourneyStage::Preparation && !bStartWithCouncil && !J.campaignFlow)
            {
                State->BeforeBattle=std::make_unique<StrategistJourney>(J);
                J.Prepare(0,FMath::Clamp(EnemyPolicy,0,2));
            }
        }
        State->Orders.clear(); State->Input = ETacticalInput::Select; State->Target = 3;
        State->Report = JourneyText(J.message); break;
    case ETacticalCommand::Restart:
        StartTrial(); return;
    case ETacticalCommand::Skip:
        FinishPlayback(); return;
    }
    Refresh();
}

void ATacticalField::Action(FKey Key)
{
    if (Key==EKeys::F5) { Issue(ETacticalCommand::RealtimeMode);return; }
    if (bRealtimeCommand && (State->Journey.stage==JourneyStage::Battle || IsPlaying()))
    {
        if (Key==EKeys::SpaceBar) { Issue(ETacticalCommand::RealtimePause);return; }
        if (Key==EKeys::Period) { Issue(ETacticalCommand::RealtimeStep);return; }
        if (Key==EKeys::X) { Issue(ETacticalCommand::RealtimeStop);return; }
        if (Key==EKeys::B) { Issue(ETacticalCommand::RealtimeChainSignal);return; }
        if (IsPlaying())
        {
            if (Key==EKeys::One) SelectOfficer(0);
            if (Key==EKeys::Two) SelectOfficer(1);
            if (Key==EKeys::Three) SelectOfficer(2);
            if (Key==EKeys::M) Issue(ETacticalCommand::Move);
            if (Key==EKeys::G) Issue(ETacticalCommand::Hold);
            if (Key==EKeys::Escape) { State->Input=ETacticalInput::Select;Refresh(); }
            return;
        }
    }
    if (IsPlaying())
    {
        if (Key == EKeys::SpaceBar || Key == EKeys::Escape) Issue(ETacticalCommand::Skip);
        return;
    }
    if (State->Journey.stage != JourneyStage::Battle)
    {
        if(State->Journey.NeedsStoryReply())
        {
            if(Key==EKeys::One) { Issue(ETacticalCommand::StoryReplyPreserve);return; }
            if(Key==EKeys::Two) { Issue(ETacticalCommand::StoryReplyCoordinate);return; }
        }
        if (Key == EKeys::N) Issue(ETacticalCommand::Restart);
        if (Key == EKeys::Enter)
        {
            if (CanIssue(ETacticalCommand::Dialogue)) Issue(ETacticalCommand::Dialogue);
            else if (CanIssue(ETacticalCommand::Deploy)) Issue(ETacticalCommand::Deploy);
            else if (CanIssue(ETacticalCommand::Continue)) Issue(ETacticalCommand::Continue);
            else if (CanIssue(ETacticalCommand::Retry)) Issue(ETacticalCommand::Retry);
        }
        return;
    }
    if (Key == EKeys::One) SelectOfficer(0);
    if (Key == EKeys::Two) SelectOfficer(1);
    if (Key == EKeys::Three) SelectOfficer(2);
    if (Key == EKeys::M) Issue(ETacticalCommand::Move);
    if (Key == EKeys::A) Issue(ETacticalCommand::PrepareAmbush);
    if (Key == EKeys::G) Issue(ETacticalCommand::Hold);
    if (Key == EKeys::F) Issue(ETacticalCommand::Strike);
    // Q/W/E remain role-specific shortcuts; the visible button uses the officer's tactic.
    if ((Key == EKeys::Q && State->Selected == 0) || (Key == EKeys::W && State->Selected == 1) || (Key == EKeys::E && State->Selected == 2)) Issue(ETacticalCommand::QueueTactic);
    if (Key == EKeys::T) Issue(ETacticalCommand::Sample);
    if (Key == EKeys::C) Issue(ETacticalCommand::Execute);
    if (Key == EKeys::R) Issue(ETacticalCommand::Clear);
    if (Key == EKeys::SpaceBar) Issue(ETacticalCommand::EndTurn);
    if (Key == EKeys::Enter) Issue(ETacticalCommand::Continue);
    if (Key == EKeys::N) Issue(ETacticalCommand::Restart);
    if (Key == EKeys::Escape) Issue(ETacticalCommand::Cancel);
}
