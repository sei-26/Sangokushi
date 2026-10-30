#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

FTacticTargetPresentation ATacticalField::ReadTacticTarget(int Target) const
{
    FTacticTargetPresentation Read;
    const auto& J=State->Journey;const auto* Seen=J.intelligence[0].Find(Target);
    if(!Seen || !Seen->current || Seen->observed.troops<=0 || !StrategistJourney::OnMap(Seen->observed.tile) ||
        !J.world.Hostile(J.world.player,Seen->observed.faction)) return Read;
    Read.Army=Target;Read.Tile=Seen->observed.tile;Read.bSelected=Target==State->Target;
    Read.Detail=TacticReservationBlockReason();Read.Badge=TEXT("予約不可");
    if(!Read.Detail.IsEmpty()) return Read;
    const auto Tactic=State->SelectedTactics[State->Selected];
    if(!TacticChain::Definition(Tactic).targetsEnemy) return Read;
    const int Prefix=int(State->Orders.size());
    auto Predict=[&](int Destination)
    {
        auto Orders=State->Orders;Orders.push_back({State->Selected,Target,Tactic,Destination});
        return J.Preview(Orders); // Sanitized observation copy, never actual hidden occupancy.
    };
    ChainResult Result;
    if(Tactic==ChainTactic::Lure)
    {
        for(int Tile:J.world.MapNeighbors(J.world.armies[State->Selected].tile))
        {
            if(!StrategistJourney::OnMap(Tile) || !J.intelligence[0].visible[Tile]) continue;
            auto Candidate=Predict(Tile);
            const bool Ready=Candidate.completed==Prefix+1;
            if(Result.events.empty() || Ready) Result=std::move(Candidate);
            if(Ready) ++Read.RetreatChoices;
        }
    }
    else Result=Predict(-1);
    Read.bReady=Result.completed==Prefix+1;
    if(Read.bReady)
    {
        const auto& Event=Result.events.back();Read.Step=Prefix+1;
        Read.ComboPercent=Event.comboPercent;Read.Damage=Event.damage;
        Read.Badge=Prefix==0 ? TEXT("単独可"):FString::Printf(TEXT("%d手目可"),Read.Step);
        Read.Detail=FString::Printf(TEXT("第%d手の成立見込み / 火力%d％"),Read.Step,Read.ComboPercent);
        if(Tactic==ChainTactic::Lure)
            Read.Detail+=FString::Printf(TEXT(" / 後退先%d候補。予約して水色の後退先を選ぶ。"),Read.RetreatChoices);
        else Read.Detail+=FString::Printf(TEXT(" / 予測損害 %d"),Read.Damage);
        Read.Detail+=TEXT("\n")+JourneyText(Event.reason);
    }
    else
    {
        Read.Badge=Result.completed<Prefix ? TEXT("先行手停止"):TEXT("条件外");
        Read.Detail=Result.events.empty() ? TEXT("確認できる後退先がない。配置と視界を確認しよう。"):
            (Result.completed<Prefix ? FString::Printf(TEXT("先行する第%d手が停止。予約欄で条件を確認。\n"),Result.completed+1):FString())+JourneyText(Result.events.back().reason);
    }
    Read.Detail+=TEXT("\n捕捉情報からの予測。未確認の部隊や次の更新で結果は変わります。");
    return Read;
}
void ATacticalField::BuildTacticTargets()
{
    auto& H=State->HUD;const auto& J=State->Journey;
    if(J.stage!=JourneyStage::Battle || IsPlaying() || State->Selected<0 || State->Selected>=3 || !TacticChain::Living(J.world,State->Selected)) return;
    if(!TacticChain::Definition(State->SelectedTactics[State->Selected]).targetsEnemy)
    { if(State->Input==ETacticalInput::Select) H.TacticTargetSummary=TEXT("味方支援 / 敵の指定は不要");return; }
    H.bTacticTargetOverlay=State->Input==ETacticalInput::Select;
    for(const auto& Seen:J.intelligence[0].contacts)
    {
        auto Read=ReadTacticTarget(Seen.id);if(Read.Army<0) continue;
        if(Read.bReady) ++H.ReadyTacticTargets;
        H.TacticTargets.Add(MoveTemp(Read));
    }
    if(State->Input!=ETacticalInput::Select) return; // Keep movement/lure cell hints distinct from target colors.
    H.TacticTargetSummary=FString::Printf(TEXT("成立見込み %d / 捕捉 %d隊"),H.ReadyTacticTargets,H.TacticTargets.Num());
    for(const auto& Order:State->Orders) if(Order.army==State->Selected)
    { H.TacticTargetSummary=TEXT("予約済み / 変更は予約欄の×");break; }
}
bool ATacticalField::SelectNextTacticTarget()
{
    if(IsPlaying() || State->Journey.stage!=JourneyStage::Battle || State->Input!=ETacticalInput::Select) return false;
    const auto& Reads=State->HUD.TacticTargets;int Current=-1;
    for(int I=0;I<Reads.Num();++I) if(Reads[I].Army==State->Target) Current=I;
    int Next=-1;
    for(int Offset=1;Offset<=Reads.Num();++Offset)
    {
        const auto& Read=Reads[(Current+Offset)%Reads.Num()];
        if(Read.bReady && ReadTacticTarget(Read.Army).bReady) { Next=Read.Army;break; }
    }
    if(Next<0) return false;
    State->Target=Next;
    if(bRealtimeCommand) State->Realtime.paused=true;
    State->bConfirmEndTurn=State->bConfirmWithdrawal=false;
    State->Report=TEXT("成立見込みの敵を選択。敵札の説明と予約の予測を確認し、自分の合図で発動しよう。");
    Refresh();return true;
}
