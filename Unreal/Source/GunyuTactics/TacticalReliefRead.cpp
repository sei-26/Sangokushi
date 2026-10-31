#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;
int ATacticalField::ReliefSource() const
{
    const auto& J=State->Journey;const int Id=State->Selected;
    if(J.stage!=JourneyStage::Battle || Id<0 || Id>=3 || !TacticChain::Living(J.world,Id)) return -1;
    const int From=J.world.armies[Id].tile;const auto& Duty=J.assignments[Id];
    return (J.Definition().kind==JourneyObjective::HoldPositions && (From==J.goal || From==J.Definition().secondGoal)) ||
        (Duty.duty==JourneyDuty::Hold && Duty.tile==From) ? From:-1;
}
FReliefOfficerRead ATacticalField::ReadReliefOfficer(int Officer) const
{
    FReliefOfficerRead Read;const auto& J=State->Journey;const int From=ReliefSource(),Out=State->Selected;
    if(From<0 || Officer<0 || Officer>=3 || Officer==Out) return Read;
    Read.Army=Officer;Read.Tile=J.world.armies[Officer].tile;
    Read.Name=JourneyText(J.world.generals[J.world.armies[Officer].general].name);Read.Status=TEXT("交代不可");
    if(!TacticChain::Living(J.world,Officer)) { Read.Detail=TEXT("この隊は戦闘不能。");return Read; }
    for(const auto& Order:State->Orders) if(Order.army==Officer)
    { Read.Detail=TEXT("戦法を予約中。先に予約を見直そう。");return Read; }
    const int Destination=State->ComparedPosition;
    if(!ReadTacticPosition(Destination).bMoveAllowed) { Read.Detail=TEXT("先に、出る隊が移動できる番号を選ぶ。");return Read; }
    if(J.world.MapDistance(Read.Tile,From)!=1)
    { Read.Detail=TEXT("守備地点の隣接マスへ先に待機させよう。");return Read; }
    // Move both officers in an observation-only copy; do not advance objectives or scout.
    auto View=std::make_unique<StrategistJourney>(J);
    for(int I=0;I<int(View->world.armies.size());++I) if(J.world.Hostile(J.world.player,View->world.armies[I].faction))
    { const auto* Seen=J.intelligence[0].Find(I);View->world.armies[I]=Seen && Seen->current ? Seen->observed:Army{}; }
    std::u32string Reason;
    if(!View->chain.Move(View->world,J.intelligence[0],Out,Destination,Reason) ||
        !View->chain.Move(View->world,J.intelligence[0],Officer,From,Reason))
    { Read.Detail=JourneyText(Reason);return Read; }
    Read.bCanIssue=true;
    const auto& Incoming=J.world.armies[Officer];
    const bool Beacon=J.Definition().kind==JourneyObjective::HoldPositions;
    const bool SourceBeacon=Beacon && (From==J.goal || From==J.Definition().secondGoal);
    const bool SourceSecured=View->PositionSecured(From);
    const bool Both= !Beacon || (View->PositionSecured(J.goal) && View->PositionSecured(J.Definition().secondGoal));
    Read.bSecured=!bRealtimeCommand && SourceSecured && Both;
    Read.Status=bRealtimeCommand ? TEXT("交代可・空白あり"):Read.bSecured ? TEXT("交代後の維持見込み"):TEXT("交代可・条件不足");
    Read.Detail=Read.bSecured ? TEXT("次の指揮へ進む前に2隊を順に移動。捕捉情報では守備条件を満たす。"):
        !SourceSecured ? (Incoming.troops<150 || Incoming.morale<25 ? TEXT("後任は兵150・士気25の維持条件を満たさない。"):
        TEXT("隣接敵がいる。到着した指揮は移動済みで固守できない。先に敵を退けよう。")):
        !Both ? TEXT("もう一方の烽火が維持条件を満たさない。両方の守備を確認。"):
        TEXT("後任も移動して守備を引き継ぐ。");
    if(bRealtimeCommand)
    {
        const FString Conditions=Read.Detail;
        Read.Detail=SourceBeacon ? TEXT("占有中の地点には同じ周期で入れない。最短2周期で交代し、維持は一度途切れる。敵や衝突でさらに遅れる。"):
            TEXT("占有中の地点には同じ周期で入れない。最短2周期、少なくとも1周期は守備地点が空く。");
        if(!SourceSecured || !Both) Read.Detail+=TEXT(" ")+Conditions;
        Read.Detail+=TEXT(" 到着周期は固守せず、次の周期以降に準備する。");
        for(const auto& Plan:J.ReadEnemyPlans()) if(Plan.next==Destination && Plan.next!=Plan.from)
        {
            Read.Status=TEXT("交代可・進路注意");
            Read.Detail=TEXT("前進先が敵の推定進路と重なる。移動が止まる可能性。\n")+Read.Detail;break;
        }
    }
    if(J.chain.concealed[Officer]) Read.Detail+=TEXT(" 後任の潜伏と伏兵の準備も解除される。");
    const auto& Duty=J.assignments[Officer];
    if((Beacon && (Incoming.tile==J.goal || Incoming.tile==J.Definition().secondGoal)) ||
        (Duty.duty==JourneyDuty::Hold && Duty.tile==Incoming.tile))
        Read.Detail+=TEXT(" 後任が今守っている地点も空く。");
    Read.Detail+=TEXT(" 未確認の敵・移動後の新しい情報で結果は変わる。");
    return Read;
}
void ATacticalField::BuildReliefPresentation()
{
    auto& P=State->HUD.Position;auto& R=P.Relief;
    if(!P.bVisible || ReliefSource()<0) return;
    R.bVisible=true;R.Source=ReliefSource();R.Incoming=State->ComparedRelief;
    R.Detail=TEXT("後任を選ぶ。2隊の移動と守備条件を比較できます。");
    for(int I=0;I<3;++I) if(I!=State->Selected)
    {
        auto Read=ReadReliefOfficer(I);
        if(I==R.Incoming) { R.bCanIssue=Read.bCanIssue;R.bSecured=Read.bSecured;R.IncomingTile=Read.Tile;R.Detail=Read.Name+TEXT(" / ")+Read.Status+TEXT("\n")+Read.Detail; }
        R.Officers.Add(MoveTemp(Read));
    }
    R.Cost=bRealtimeCommand ? TEXT("2隊の継続指示・経由地を置換して停止。後任は到着後に固守。予約・合図・他隊は保持。"):
        TEXT("2隊の移動を各1回消費。失敗・任務終了なら途中で止まる。戦法・固有の使用状況は維持。");
    R.ConfirmLabel=bRealtimeCommand ? TEXT("2隊の前指示を交代行軍に置換"):TEXT("2隊の交代を確定");
    P.ConfirmLabel=bRealtimeCommand ? (State->Realtime.orders[State->Selected].kind==JourneyDirective::None ? TEXT("選択隊だけ行軍を指示"):TEXT("選択隊だけの前指示を置換")):TEXT("選択隊だけを移動");
}
