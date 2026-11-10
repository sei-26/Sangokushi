#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyMarchBlocker.hpp"

using namespace frontline;

namespace
{
    bool DutyReady(const StrategistJourney& J,int Army)
    {
        const auto& A=J.assignments[Army];
        return J.world.armies[Army].tile==A.tile &&
            (A.duty==JourneyDuty::Hold ? J.guarding[Army]:J.chain.concealed[Army]);
    }
}

bool ATacticalField::CanResumeDuty() const
{
    const auto& J=State->Journey;
    const int Id=State->Selected;
    return bRealtimeCommand && !IsPlaying() && !State->bAutoBattle && !State->bRealtimeStepping &&
        State->Orders.empty() && !State->ChainSignal.Armed() && J.campaignFlow && J.stage==JourneyStage::Battle &&
        Id>=0 && Id<3 && TacticChain::Living(J.world,Id) && J.assignments[Id].duty!=JourneyDuty::None &&
        State->Realtime.orders[Id].kind==JourneyDirective::None && !DutyReady(J,Id);
}

void ATacticalField::ResumeDuty()
{
    if(!CanResumeDuty()) return;
    const int Id=State->Selected;
    const auto& J=State->Journey;
    const auto& A=J.assignments[Id];
    auto Candidate=State->Realtime;
    const auto Arrival=A.duty==JourneyDuty::Hold ? JourneyArrival::Hold:JourneyArrival::Ambush;
    if(!Candidate.Queue(J,Id,JourneyDirective::March,A.tile) || !Candidate.SetArrival(J,Id,Arrival)) return;
    Candidate.paused=true;
    State->Realtime=std::move(Candidate);
    State->Input=ETacticalInput::Select;State->bConfirmWithdrawal=State->bConfirmEndTurn=false;
    State->Report=TEXT("担当地点への行軍を指示。到着後に")+FString(Arrival==JourneyArrival::Hold ? TEXT("固守"):TEXT("伏兵準備"))+TEXT("を試みます。");
    Refresh();
}

void ATacticalField::BuildDutyProgress()
{
    auto& H=State->HUD;
    const int Id=H.Selected;
    if(!H.Officers.IsValidIndex(Id) || !H.Officers[Id].bLiving || H.Officers[Id].DutyTile<0) return;
    const auto& J=State->Journey;
    const auto& A=J.assignments[Id];
    auto& P=H.DutyProgress;
    P.Tile=A.tile;P.bCanResume=CanResumeDuty();
    const auto& Order=State->Realtime.orders[Id];
    const bool Hold=A.duty==JourneyDuty::Hold;
    const TCHAR* DutyAction=Hold ? TEXT("固守"):TEXT("伏兵準備");
    const auto Arrival=Hold ? JourneyArrival::Hold:JourneyArrival::Ambush;
    const bool March=bRealtimeCommand && Order.kind==JourneyDirective::March;
    if(J.world.armies[Id].tile==A.tile)
    {
        const bool Ready=DutyReady(J,Id);
        const bool Ordered=bRealtimeCommand && (Order.kind==(Hold ? JourneyDirective::Hold:JourneyDirective::Ambush) ||
            (March && Order.destination==A.tile && Order.waypoints.empty() && Order.arrival==Arrival));
        P.Caption=Ready ? FString(Hold ? TEXT("到着・固守中"):TEXT("到着・潜伏中")):
            FString::Printf(TEXT("到着・%s%s"),DutyAction,Ordered ? (A.completed ? TEXT("指示あり"):TEXT("待ち")):TEXT("なし"));
        if(Ready) P.Detail=TEXT("担当地点で構えています。");
        else if(Ordered && A.completed) P.Detail=TEXT("この地点で構えた記録があります。次の指揮でも現在の命令を続けます。");
        else P.Detail=TEXT("担当地点に到着。兵・士気や行動権、周囲の敵で準備の成否が変わります。");
        return;
    }
    if(March && (Order.waypoints.empty() ? Order.destination:Order.waypoints.back())==A.tile)
    {
        const auto& Read=State->OrderReads[Id];
        if(Read.status==JourneyRouteStatus::NoRoute)
        {
            const auto Block=ReadJourneyMarchBlocker(J,State->Realtime,Id);
            P.Caption=TEXT("進路待ち");
            P.Detail=TEXT("捕捉中の情報では、次の地点へ通れる道が見つかりません。");
            if(Block.army>=0)
            {
                const FString Name=JourneyText(J.world.generals[J.world.armies[Block.army].general].name);
                P.InspectTile=Block.tile;
                P.InspectLabel=TEXT("道をふさぐ隊を見る");
                P.Caption+=TEXT("・")+Name;
                P.Detail=Name+TEXT("が道をふさいでいます。この隊が道を空ければ、通れる経路があります。");
            }
            return;
        }
        if(!Read.complete && Read.route.size()<=1)
        {
            P.Caption=TEXT("次の区間の進路待ち");
            P.Detail=TEXT("経由地点より先に、通れる道が見つかりません。");
            return;
        }
        if(BuildDutyTraffic()) return;
        P.Caption=FString::Printf(TEXT("担当へ行軍・あと%dマス%s"),FMath::Max(0,int(Read.route.size())-1),Read.complete ? TEXT(""):TEXT("以上"));
        const bool Matches=Order.arrival==Arrival;
        P.Detail=Matches ? FString::Printf(TEXT("到着したら%sを試みます。"),DutyAction):TEXT("到着後は、軍議と異なる指示が設定されています。");
        if(!Read.complete) P.Detail+=TEXT("\n途中までの見通しです。");
        return;
    }
    const bool Other=bRealtimeCommand && Order.kind!=JourneyDirective::None;
    P.Caption=Other ? TEXT("別の命令中"):A.completed ? TEXT("担当地点を離れた"):TEXT("担当への指示なし");
    P.Detail=P.bCanResume ? TEXT("「担当へ行軍を指示」で軍議の行き先と到着後の構えを指定します。"):
        TEXT("担当地点は軍議で決めた場所です。現在の指示と位置を確認してください。");
}
