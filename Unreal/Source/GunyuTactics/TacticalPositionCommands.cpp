#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;
bool ATacticalField::CanCompareTacticPositions() const
{
    const auto& J=State->Journey;const int Id=State->Selected;
    if(IsPlaying() || J.stage!=JourneyStage::Battle || State->Input!=ETacticalInput::Select || State->bRealtimeStepping || State->bRealtimeChainRequested ||
        Id<0 || Id>=3 || !TacticChain::Living(J.world,Id) || (!TacticChain::Definition(State->SelectedTactics[Id]).targetsEnemy && ReliefSource()<0)) return false;
    for(const auto& Order:State->Orders) if(Order.army==Id) return false;
    return true;
}
bool ATacticalField::BeginTacticPositionComparison()
{
    if(!CanCompareTacticPositions()) return false;
    State->Input=ETacticalInput::Position;State->ComparedPosition=-1;State->ComparedRelief=-1;
    State->Report=TEXT("仮配置を比較中。番号の選択では移動しません。確定するまで潜伏・守備と行動権を保持します。");
    if(bRealtimeCommand) State->Realtime.paused=true;
    Refresh();return true;
}
void ATacticalField::SelectTacticPosition(int Tile)
{
    if(IsPlaying() || State->Input!=ETacticalInput::Position || ReadTacticPosition(Tile).Tile<0) return;
    State->ComparedPosition=Tile;Refresh();
}
bool ATacticalField::CommitTacticPosition()
{
    if(IsPlaying() || State->Input!=ETacticalInput::Position) return false;
    const int Tile=State->ComparedPosition;const auto Read=ReadTacticPosition(Tile);
    if(!Read.bMoveAllowed) return false; // Revalidate the current observation, never trust a cached green marker.
    State->bConfirmEndTurn=State->bConfirmWithdrawal=false;State->Input=ETacticalInput::Select;
    if(bRealtimeCommand)
    {
        State->Realtime.paused=true;
        const bool Queued=State->Realtime.Queue(State->Journey,State->Selected,JourneyDirective::March,Tile);
        State->Report=Queued ? TEXT("行軍を指示して停止。再開後は敵味方も動く。到着してから戦法を再確認しよう。"):
            TEXT("行軍を指示できません。地形と部隊を確認しよう。");
        Refresh();return Queued;
    }
    const auto Before=ObserveUnits();const bool Moved=State->Journey.Move(State->Selected,Tile);
    State->Report=JourneyText(State->Journey.message);
    if(Moved) QueueAction(EBattleCue::March,State->Selected,-1,Before);
    Refresh();return Moved;
}
