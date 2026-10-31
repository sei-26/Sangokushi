#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;
void ATacticalField::SelectReliefOfficer(int Officer)
{
    if(IsPlaying() || State->Input!=ETacticalInput::Position || ReadReliefOfficer(Officer).Army<0) return;
    State->ComparedRelief=Officer;Refresh();
}
bool ATacticalField::CommitRelief()
{
    if(IsPlaying() || State->Input!=ETacticalInput::Position || !ReadReliefOfficer(State->ComparedRelief).bCanIssue) return false;
    const int Out=State->Selected,In=State->ComparedRelief,From=ReliefSource(),To=State->ComparedPosition;
    if(bRealtimeCommand)
    {
        auto Orders=State->Realtime;
        if(!Orders.Queue(State->Journey,Out,JourneyDirective::March,To) || !Orders.Queue(State->Journey,In,JourneyDirective::March,From) ||
            !Orders.SetArrival(State->Journey,In,JourneyArrival::Hold)) return false;
        Orders.paused=true;State->Realtime=std::move(Orders);
        State->Journey.RecordReliefOrder(Out,In,From,To,true);
        State->Input=ETacticalInput::Select;State->bConfirmEndTurn=State->bConfirmWithdrawal=false;
        State->Report=TEXT("2隊の交代行軍を指示して停止。後任は同じ周期に入れず、守備に空白が出る。予約と合図は保持。再開後の状況を確認しよう。");
        Refresh();return true;
    }
    State->Journey.RecordReliefOrder(Out,In,From,To,false);
    const auto Before=ObserveUnits();
    const bool Departed=State->Journey.Move(Out,To);const auto DepartureReason=JourneyText(State->Journey.message);
    const auto Middle=ObserveUnits();
    const bool Arrived=Departed && State->Journey.stage==JourneyStage::Battle && State->Journey.Move(In,From);
    const auto ArrivalReason=JourneyText(State->Journey.message);const auto After=ObserveUnits();
    State->Input=ETacticalInput::Select;State->bConfirmEndTurn=State->bConfirmWithdrawal=false;
    State->Report=!Departed ? DepartureReason:Arrived ? TEXT("2隊が交代。移動を各1回消費。今の守備条件を見て、戦法と次の指揮を決めよう。"):
        TEXT("先に出る隊だけ移動。任務の終了か後任の条件を確認。実際に使った移動は戻りません。");
    if(Departed && !State->bNoPlayback)
    {
        State->Beats.Reset();
        auto Add=[this](int Actor,const TArray<FVisualUnit>& A,const TArray<FVisualUnit>& B,const FString& Title,const FString& Reason)
        {
            FBattleBeat Beat;Beat.Before=A;Beat.After=B;Beat.Actor=Actor;Beat.Kind=EBattleCue::March;
            Beat.Title=A[Actor].Name+TEXT(" / ")+Title;Beat.Reason=Reason;Beat.Duration=1.8f;State->Beats.Add(MoveTemp(Beat));
        };
        Add(Out,Before,Middle,TEXT("守備地点から前進"),DepartureReason);
        if(Arrived) Add(In,Middle,After,TEXT("守備を引き継ぐ"),ArrivalReason);
        BeginPlayback();
    }
    Refresh();return Departed && Arrived;
}
