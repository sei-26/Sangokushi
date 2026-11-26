#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyCouncilStage.hpp"
#include "JourneyPreludeOpening.hpp"

using namespace frontline;

void ATacticalField::PreferOpeningOfficer(int Officer)
{
    if(State->HUD.OpeningOrder.Officer<0 || State->OpeningOfficer==Officer ||
        Officer<0 || Officer>=3 || !TacticChain::Living(State->Journey.world,Officer)) return;
    State->OpeningOfficer=Officer;
    Refresh();
}

void ATacticalField::DismissOpeningOrder()
{
    if(State->OpeningOfficer<0) return;
    State->OpeningOfficer=-1;
    Refresh();
}

void ATacticalField::BuildOpeningOrder()
{
    const auto& J=State->Journey;
    const int Id=State->OpeningOfficer;
    if(!J.campaignFlow || J.mission>=YellowTurbanMission || J.turn!=1 || !State->BeforeBattle ||
        J.world.revision!=State->OpeningRevision || State->bAutoBattle ||
        (bRealtimeCommand && !RealtimePaused()) || Id<0 || !TacticChain::Living(J.world,Id)) return;

    const auto Brief=BuildCouncilBattleBrief(*State->BeforeBattle);
    if(!Brief.title) return;
    const auto& Suggestion=Brief.roles[Id];
    const auto& Duty=J.assignments[Id];
    const bool Assigned=Duty.duty!=JourneyDuty::None;
    const auto& Unit=J.world.armies[Id];
    auto& Read=State->HUD.OpeningOrder;
    Read.Officer=Id;
    Read.Tile=Assigned ? Duty.tile:Suggestion.tile;
    Read.Name=JourneyText(J.world.generals[Unit.general].name);
    Read.Place=Assigned ? State->HUD.Officers[Id].DutyBadge:JourneyText(Suggestion.title);
    Read.ActionLabel=TEXT("地点を見る");

    if(Id==0 && J.preparation==0 && MatchesPreludeOpening(*State->BeforeBattle,0))
    {
        Read.Tile=Unit.tile;Read.Place=TEXT("最初の一手：敵を森へ誘う");
        Read.Instruction=TEXT("劉備の誘導で敵を森へ寄せよう。一人でも発動でき、関羽の伏兵へつなぐこともできる。");
        Read.Action=EOpeningOrderAction::Tactics;Read.ActionLabel=TEXT("戦法一覧を開く");return;
    }

    const auto& Order=State->Realtime.orders[Id];
    if(bRealtimeCommand && Order.kind!=JourneyDirective::None)
    {
        if(Order.kind==JourneyDirective::March && Read.Tile!=Order.destination)
        {
            Read.Tile=Order.destination;
            Read.Place=TEXT("指示した行軍先");
        }
        Read.Instruction=TEXT("命令を受領。Spaceで戦いを進める。");
        return;
    }
    if(J.chain.concealed[Id])
    {
        Read.Tile=Unit.tile;
        Read.Instruction=TEXT("潜伏中。位置を保ち、敵が伏兵の間合いに入るのを待つ。");
        return;
    }
    if(Read.Tile==Unit.tile)
    {
        const bool AtBeacon=J.Definition().kind==JourneyObjective::HoldPositions &&
            (Unit.tile==J.goal || Unit.tile==J.Definition().secondGoal);
        if((AtBeacon || Duty.duty==JourneyDuty::Hold) && Duty.duty!=JourneyDuty::Ambush && !J.guarding[Id])
        {
            Read.Action=EOpeningOrderAction::Hold;
            Read.ActionLabel=TEXT("ここで固守を指示");
            Read.Instruction=TEXT("持ち場に着いている。固守して敵に備えよう。");
        }
        else Read.Instruction=Assigned ? TEXT("持ち場に着いている。部隊の構えを確かめよう。"):
            TEXT("この付近で敵を待ち、味方と戦法を合わせよう。");
        return;
    }
    Read.Action=EOpeningOrderAction::Move;
    Read.ActionLabel=TEXT("移動先を選ぶ");
    if(State->Selected==Id && State->Input==ETacticalInput::Move)
    {
        Read.ActionLabel=TEXT("目印を見直す");
        Read.Instruction=bRealtimeCommand ? TEXT("地図で行き先を選び、Spaceで進める。"):TEXT("水色の隣接マスを選ぶ。");
    }
    else if(J.IsRescue())
        Read.Instruction=TEXT("護送路の脇から前へ。車が通るマスは空けよう。");
    else Read.Instruction=TEXT("地図で目印を見て、そこへ向かう一歩を選ぼう。");
}

bool ATacticalController::ShowOpeningOrder() const
{
    return Field && Field->Presentation().OpeningOrder.Officer>=0 && !Field->IsPlaying() &&
        (!Field->bRealtimeCommand || Field->RealtimePaused()) && !bEpisodeOpen && !bDetailedBattle &&
        !bSaveMenuOpen && !bBattleLogOpen && !bMarchPlanOpen && !bChainReviewOpen && !bTacticPickerOpen &&
        !Field->Presentation().Position.bVisible;
}

void ATacticalController::OpenFirstCommand()
{
    if(!ShowOpeningOrder()) return;
    const auto Read=Field->Presentation().OpeningOrder;
    if(Read.Action==EOpeningOrderAction::Solo) { Field->UseFirstStrike(Read.SoloVersion,Read.SoloTarget);return; }
    Field->SelectOfficer(Read.Officer);
    if(Read.Action==EOpeningOrderAction::Tactics) { ToggleTacticPicker();return; }
    const int Start=Field->Presentation().Officers[Read.Officer].Tile;
    FocusBattleTile(Read.Tile);
    if(Read.Action==EOpeningOrderAction::Move)
    {
        if(!Field->bRealtimeCommand)
        {
            const auto Origin=Field->SurfaceLocation(Start);
            if(FVector::DistSquared2D(Origin,Focus)>800*800)
            {
                Focus=FMath::Lerp(Origin,Focus,.28);
                Distance=2600;
            }
        }
        Field->Issue(ETacticalCommand::Move);
    }
    if(Read.Action==EOpeningOrderAction::Hold && Field->CanIssue(ETacticalCommand::Hold))
    {
        Field->Issue(ETacticalCommand::Hold);
        Field->DismissOpeningOrder();
    }
}
