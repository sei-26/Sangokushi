#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

FTacticPositionCell ATacticalField::ReadTacticPosition(int Tile) const
{
    FTacticPositionCell Read;const auto& J=State->Journey;const int Id=State->Selected;
    if(IsPlaying() || J.stage!=JourneyStage::Battle || Id<0 || Id>=3 || !TacticChain::Living(J.world,Id) ||
        !StrategistJourney::OnMap(Tile) || !J.intelligence[0].visible[Tile] || J.world.MapDistance(J.world.armies[Id].tile,Tile)!=1) return Read;
    Read.Tile=Tile;
    // Keep all nonhostile armies, replace every hostile with its current contact.
    // Never refresh intelligence on the hypothetical move: that would scout for free.
    auto View=std::make_unique<Campaign>(J.world);
    for(int I=0;I<int(View->armies.size());++I) if(J.world.Hostile(J.world.player,View->armies[I].faction))
    { const auto* Seen=J.intelligence[0].Find(I);View->armies[I]=Seen && Seen->current ? Seen->observed:Army{}; }
    auto Chain=J.chain;std::u32string Reason;
    Read.bMoveAllowed=Chain.Move(*View,J.intelligence[0],Id,Tile,Reason);
    if(!Read.bMoveAllowed) { Read.Result=JourneyText(Reason);return Read; }
    const auto Tactic=State->SelectedTactics[Id];auto Orders=State->Orders;
    Orders.push_back({Id,State->Target,Tactic,-1});
    const auto Result=Chain.Preview(*View,J.intelligence[0],Orders);
    Read.bReady=Result.completed==int(Orders.size());
    if(Read.bReady)
    {
        const auto& Event=Result.events.back();Read.Step=Event.combo;Read.ComboPercent=Event.comboPercent;Read.Damage=Event.damage;
        Read.Result=FString::Printf(TEXT("移動後：第%d手 / 火力%d％\n予測損害 %d"),Read.Step,Read.ComboPercent,Read.Damage);
    }
    else Read.Result=Result.events.empty() ? TEXT("戦法の対象・予約を確認しよう。"):
        (Result.completed<int(State->Orders.size()) ? FString::Printf(TEXT("先行する第%d手が停止。\n"),Result.completed+1):FString())+JourneyText(Result.events.back().reason);
    if(J.chain.concealed[Id]) Read.Risk+=TEXT("移動で潜伏解除。\n準備済み伏兵の効果を失う。\n");
    const int From=J.world.armies[Id].tile;const auto& Duty=J.assignments[Id];
    if(J.Definition().kind==JourneyObjective::HoldPositions && (From==J.goal || From==J.Definition().secondGoal))
        Read.Risk+=TEXT("烽火を離れる。維持を続ける代役を確認。\n");
    else if(Duty.duty==JourneyDuty::Hold && Duty.tile==From)
        Read.Risk+=TEXT("軍議で任せた守備地点を離れる。\n");
    if(J.HasConvoy() && J.convoyStep+1<int(J.ConvoyPath().size()) && Tile==J.ConvoyPath()[J.convoyStep+1])
        Read.Risk+=TEXT("車の次の進路を塞ぐ。護送前に道を空けよう。\n");
    int Adjacent=0;
    for(const auto& Seen:J.intelligence[0].contacts)
        if(Seen.current && Seen.observed.troops>0 && J.world.Hostile(J.world.player,Seen.observed.faction) && View->MapDistance(Tile,Seen.observed.tile)<=1) ++Adjacent;
    if(Adjacent>0) Read.Risk+=FString::Printf(TEXT("捕捉した敵%d隊に隣接。反撃に注意。\n"),Adjacent);
    Read.Risk=Read.Risk.TrimEnd();
    if(Read.Risk.IsEmpty()) Read.Risk=TEXT("移動1回を消費。使用済みの行動は戻らない。");
    else Read.Risk+=TEXT("\n移動1回を消費。使用済みの行動は戻らない。");
    return Read;
}
void ATacticalField::BuildTacticPositions()
{
    auto& P=State->HUD.Position;const auto& J=State->Journey;
    if(State->Input!=ETacticalInput::Position || IsPlaying() || J.stage!=JourneyStage::Battle) return;
    P.bVisible=true;P.SelectedTile=State->ComparedPosition;
    P.Title=JourneyText(J.world.generals[J.world.armies[State->Selected].general].name)+TEXT(" / ")+State->HUD.SelectedTacticName;
    const auto* Seen=J.intelligence[0].Find(State->Target);
    if(TacticChain::Definition(State->SelectedTactics[State->Selected]).targetsEnemy && Seen && Seen->current) P.Title+=TEXT(" → ")+JourneyText(J.world.generals[Seen->observed.general].name);
    for(int Tile:J.world.MapNeighbors(J.world.armies[State->Selected].tile))
    { auto Read=ReadTacticPosition(Tile);if(Read.Tile>=0) P.Cells.Add(MoveTemp(Read)); }
    P.Result=TEXT("地図か一覧の番号を選ぶ。移動後の戦法を比較できます。");
    P.Risk=TEXT("閲覧・仮選択では部隊を動かしません。");
    for(const auto& Read:P.Cells) if(Read.Tile==P.SelectedTile)
    { P.Result=Read.Result;P.Risk=Read.Risk;P.bCanCommit=Read.bMoveAllowed;break; }
    P.ConfirmLabel=bRealtimeCommand ? TEXT("このマスへ行軍を指示"):TEXT("このマスへ移動を確定");
    P.Timing=bRealtimeCommand ? TEXT("隣接1歩の仮配置。再開後は敵味方も動くため、到着後に戦法を再確認。"):
        TEXT("現在の捕捉情報での予測。移動で新しい敵が見えるなど、実際の成立条件は変わります。");
    if(bRealtimeCommand && State->Realtime.orders[State->Selected].kind!=JourneyDirective::None)
    {
        P.ConfirmLabel=TEXT("行軍へ切替（前の指示を置換）");
        P.Timing=TEXT("確定で選択隊の継続指示・経由地を置き換えます。 ")+P.Timing;
    }
    BuildReliefPresentation();
}
