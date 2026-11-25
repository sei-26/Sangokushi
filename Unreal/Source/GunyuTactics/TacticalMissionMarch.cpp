#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

void ATacticalField::BuildMissionMarch()
{
    auto& P=State->HUD.MissionMarch;P={};
    const auto& J=State->Journey;const int Actor=State->Selected,Focus=State->HUD.MissionFocusTile;
    if(!bRealtimeCommand || J.stage!=JourneyStage::Battle || State->bAutoBattle || IsPlaying() ||
       Actor<0 || Actor>=3 || !TacticChain::Living(J.world,Actor) || !StrategistJourney::OnMap(Focus) ||
       State->HUD.bGrandOrdersPending) return;
    P.bVisible=true;P.Army=Actor;P.Focus=Focus;P.Label=TEXT("今の目標へ進む");
    P.Detail=TEXT("選択した隊だけに行軍を指示して停止。再開後に通常の移動を使う。戦法や占領の成功は代行しない。");
    if(State->Input!=ETacticalInput::Select || State->bRealtimeStepping || State->bRealtimeChainRequested ||
       !State->Orders.empty() || State->ChainSignal.Armed())
    { P.Label=TEXT("予約・指定を終えてから行軍");return; }
    const auto& Unit=J.world.armies[Actor];const auto& Duty=J.assignments[Actor];
    const bool Flag=Unit.tile==J.goal && (J.Definition().kind==JourneyObjective::HoldPositions ||
        (J.HasGrandBattle() && J.battleReview.grand.openedOn));
    if(Flag || (Duty.duty==JourneyDuty::Hold && Duty.tile==Unit.tile))
    { P.Label=TEXT("この隊は旗・守備を支える");P.Detail=TEXT("今いる持ち場を優先。別の隊を選んで進める。ここを離れる場合は、自分で移動先を指定しよう。");return; }
    if(J.chain.concealed[Actor])
    { P.Label=TEXT("潜伏中：動くなら場所を指定");P.Detail=TEXT("移動すると潜伏が解ける。伏兵を残すか、自分で移動先を決めよう。");return; }
    // Queue and path-find only on the normal observation-based clock copy.
    // Never refresh intelligence from a hypothetical destination.
    auto Clock=State->Realtime;
    JourneyMarchPlan Plan;
    if(Clock.Queue(J,Actor,JourneyDirective::March,Focus)) Plan=Clock.MarchPlan(J,Actor);
    if(Plan.route.empty() || !Plan.complete)
    {
        const auto Approach=State->Realtime.ReliefApproach(J,Actor,Focus);
        if(Approach.state!=JourneyApproachState::Clear && Approach.state!=JourneyApproachState::Adjacent)
        { P.Label=TEXT("目標への進路を確認しよう");P.Detail+=TEXT("\n捕捉中の敵や味方で道が塞がれている。手動で進路を選ぶか、別の隊を使おう。");return; }
        Plan.route=Approach.route;
    }
    if(Plan.route.empty()) return;
    P.Destination=Plan.route.back();
    if(P.Destination==Unit.tile)
    { P.Label=TEXT("目標はすぐそば：戦法を選ぼう");P.Detail=TEXT("すでに目標付近にいる。敵がいれば戦法で崩し、味方なら行く手を空けて支えよう。");return; }
    for(int Tile:Plan.route) P.Route.Add(Tile);
    const auto& Existing=State->Realtime.orders[Actor];
    if(Existing.kind==JourneyDirective::March)
    { P.Label=TEXT("行軍中：進路は V で確認");P.Detail=TEXT("現在の経由地と到着後の指示を保持。別の場所へ向かうなら地図で行軍先を指定し直そう。");return; }
    const FString Name=JourneyText(J.world.generals[Unit.general].name);
    P.Label=Name+TEXT("を目標へ進める");P.bCanIssue=true;
    P.Detail+=FString::Printf(TEXT("\n行き先 (%d,%d) ／ 捕捉情報で%d歩。敵味方が動くため到着の保証はない。前の固守・伏兵準備は置き換える。"),
        P.Destination%Width,P.Destination/Width,P.Route.Num()-1);
    if(J.chain.moved[Actor]) P.Detail+=TEXT("\n移動は使用済み。行動の回復を待ってから進む。");
}

bool ATacticalField::IssueMissionMarch(const FTacticalPresentation::FMissionMarch& Expected)
{
    if(!Expected.bCanIssue || IsPlaying()) return false;
    const auto Pressed=Expected; // The caller may pass a reference into the HUD refreshed below.
    Refresh(); // Mission, actor and route must still match the proposal that was pressed.
    const auto Now=State->HUD.MissionMarch;
    if(!Now.bCanIssue || Now.Army!=Pressed.Army || Now.Focus!=Pressed.Focus || Now.Destination!=Pressed.Destination || Now.Route!=Pressed.Route)
    { State->Report=TEXT("役目や進路が変わった。今の目標を確認して、もう一度指示しよう。");Refresh();return false; }
    if(!State->Realtime.Queue(State->Journey,Now.Army,JourneyDirective::March,Now.Destination)) return false;
    State->bConfirmEndTurn=State->bConfirmWithdrawal=false;
    State->Realtime.paused=true;
    State->Report=TEXT("選んだ隊へ目標付近までの行軍を指示した。Spaceで再開。到着したら、戦法か守備を自分で選ぼう。");
    Refresh();return true;
}
