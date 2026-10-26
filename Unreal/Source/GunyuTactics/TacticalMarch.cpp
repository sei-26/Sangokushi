#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

void ATacticalField::AppendWaypointAt(int Tile)
{
    if (!bRealtimeCommand || IsPlaying() || State->Journey.stage!=JourneyStage::Battle) return;
    State->Realtime.paused=true;State->bRealtimeStepping=false;
    const bool Added=State->Realtime.AppendWaypoint(State->Journey,State->Selected,Tile);
    State->Report=Added ? TEXT("行軍地点を追加。最大４地点を順番に通る。Vで到着後の指示を設定できる。") :
        TEXT("地点を追加できません。最大４地点・連続した同一地点は禁止。川・都市を避け、伏兵予定の到着地は森林を選ぼう。");
    if (Added) State->Input=ETacticalInput::Select;
    Refresh();
}
void ATacticalField::IssueMarchPlan(ETacticalCommand Command)
{
    auto& RT=State->Realtime;RT.paused=true;State->bRealtimeStepping=false;
    if (Command==ETacticalCommand::RealtimeUndoWaypoint)
    {
        RT.UndoWaypoint(State->Selected);
        State->Report=TEXT("最後の行軍地点を取り消した。到着後は待機に戻る。残る地点がなければ行軍を解除。");
    }
    else
    {
        const auto Current=RT.orders[State->Selected].arrival;
        const auto Next=Current==JourneyArrival::Wait ? JourneyArrival::Hold : Current==JourneyArrival::Hold ? JourneyArrival::Ambush : JourneyArrival::Wait;
        if (!RT.SetArrival(State->Journey,State->Selected,Next)) RT.SetArrival(State->Journey,State->Selected,JourneyArrival::Wait);
        State->Report=TEXT("到着後の指示を変更。固守・伏兵準備は移動の余力を使い、到着の次の更新から行う。連鎖が成立する場合は発動を優先する。");
    }
    Refresh();
}
void ATacticalField::BuildMarchPresentation()
{
    auto& H=State->HUD;H.ArrivalLabel=TEXT("到着後：待機");
    if (!bRealtimeCommand || !H.bBattle) return;
    const auto& RT=State->Realtime;const auto& Order=RT.orders[State->Selected];
    if (Order.kind!=JourneyDirective::March)
    {
        H.MarchSummary=Order.kind==JourneyDirective::Hold ? TEXT("固守を継続中。右クリックで新しい行軍先を指定できる。") :
            Order.kind==JourneyDirective::Ambush ? TEXT("伏兵準備を継続中。地形・移動の余力・敵の接近で成否が決まる。") :
            TEXT("行軍指示なし。右クリックで目的地、Shift＋右クリックで経由地を追加。");
        if (Order.kind==JourneyDirective::Hold) H.ArrivalLabel=TEXT("現在：固守");
        if (Order.kind==JourneyDirective::Ambush) H.ArrivalLabel=TEXT("現在：伏兵準備");
        return;
    }
    const auto Plan=RT.MarchPlan(State->Journey,State->Selected);
    for (int Tile : Plan.route) H.MarchRoute.Add(Tile);
    H.MarchSummary=FString::Printf(TEXT("残り%d地点 / %s。全地点を通った後、指定した構えへ移る。"),static_cast<int>(Plan.points.size()),Plan.complete ? TEXT("観測情報で経路あり") : TEXT("途中の経路が未成立"));
    H.ArrivalLabel=Order.arrival==JourneyArrival::Hold ? TEXT("到着後：固守") : Order.arrival==JourneyArrival::Ambush ? TEXT("到着後：伏兵準備") : TEXT("到着後：待機");
    for (int Index=0;Index<static_cast<int>(Plan.points.size());++Index)
    {
        FMarchPointPresentation Point;Point.Tile=Plan.points[Index];
        const auto TileTerrain=State->Journey.world.tiles[Point.Tile].terrain;
        const TCHAR* Name=TileTerrain==Terrain::Forest ? TEXT("森林") : TileTerrain==Terrain::Mountain ? TEXT("山地") : TileTerrain==Terrain::Bridge ? TEXT("橋") : TEXT("平地");
        Point.Label=FString::Printf(TEXT("%d / %s / %s"),Index+1,Index+1==static_cast<int>(Plan.points.size()) ? TEXT("到着地") : TEXT("経由地"),Name);
        H.MarchPoints.Add(MoveTemp(Point));
    }
}
