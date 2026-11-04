#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyDefense.hpp"
using namespace frontline;
namespace
{
    FString DefenseCause(JourneyDefenseReason Reason)
    {
        switch(Reason)
        {
        case JourneyDefenseReason::Secured:return TEXT("守備成立");
        case JourneyDefenseReason::Vacant:return TEXT("守備隊なし");
        case JourneyDefenseReason::Depleted:return TEXT("兵・士気不足");
        case JourneyDefenseReason::NeedsGuard:return TEXT("敵接近・固守なし");
        case JourneyDefenseReason::GuardSpent:return TEXT("行動済・固守なし");
        case JourneyDefenseReason::GuardWeak:return TEXT("固守の兵・士気不足");
        case JourneyDefenseReason::Overwhelmed:return TEXT("複数の敵が接近");
        default:return TEXT("未確認");
        }
    }
}
void ATacticalField::BuildDefensePresentation()
{
    auto& H=State->HUD;const auto& J=State->Journey;
    if(J.stage!=JourneyStage::Battle || J.Definition().kind!=JourneyObjective::HoldPositions) return;
    for(int Goal:{J.goal,J.Definition().secondGoal})
    {
        const auto Now=ReadJourneyDefense(J,Goal),Last=ReadJourneyDefenseCheck(J,Goal);
        FBeaconDefensePresentation P;P.Goal=Goal;P.Guard=Now.guard;P.bReady=Now.secured;P.bLastSecured=Last.secured;
        const FString Side=Goal==J.goal ? TEXT("北"):TEXT("南");
        const FString Name=Now.guard>=0 ? JourneyText(J.world.generals[J.world.armies[Now.guard].general].name):TEXT("空位");
        FString Next=DefenseCause(Now.reason);
        if(Now.secured) Next=Now.guarded ? TEXT("固守中（捕捉）"):TEXT("配置○（捕捉）");
        else if(Now.reason==JourneyDefenseReason::GuardSpent) Next=TEXT("固守待ち");
        else if(Now.reason==JourneyDefenseReason::NeedsGuard)
            Next=Now.moved && bRealtimeCommand ? TEXT("固守待ち"):bRealtimeCommand && State->Realtime.orders[Now.guard].kind==JourneyDirective::Hold ? TEXT("固守指示あり"):TEXT("固守で備えよう");
        P.Current=Name+TEXT("：")+Next;
        const FString LastCause=Last.reason==JourneyDefenseReason::Overwhelmed ? FString(TEXT("敵の圧力")):
            Last.reason==JourneyDefenseReason::GuardSpent || Last.reason==JourneyDefenseReason::NeedsGuard ? FString(TEXT("固守なし")):DefenseCause(Last.reason);
        P.Last=Last.turn>0 ? FString::Printf(TEXT("前回%d %s %s"),Last.turn,Last.secured ? TEXT("○"):TEXT("×"),*LastCause):TEXT("前回：報告なし");
        P.Detail=Side+TEXT("の烽火 / 現在の捕捉\n")+P.Current+FString::Printf(TEXT("\n捕捉中の隣接敵%d隊。占位は兵150・士気25以上。"),Now.threats);
        P.Detail+=TEXT("敵1隊なら固守と兵500・士気40以上が必要。2隊以上は退ける。視界外の敵や、この後の移動で結果は変わる。");
        if(Now.guard>=0)
        {
            const auto& U=J.world.armies[Now.guard];P.Detail+=FString::Printf(TEXT("\n%s：兵%d・士気%d。"),*Name,U.troops,U.morale);
            if(Now.attackSpent) P.Detail+=TEXT("今の行動は使用済み。行動回復後に固守を試みる。敵を別の隊で退けることもできる。");
            else if(Now.guarded) P.Detail+=TEXT("現在は固守中。兵・士気と敵の条件を合わせて、次の実判定で守備成立を確認する。");
            else if(Now.moved && bRealtimeCommand) P.Detail+=TEXT("移動済みのため、常設の固守は今回の更新では行わない。");
            else if(Now.canGuard) P.Detail+=TEXT("固守を指示して備えられる。指示だけでは、実際の守備成立は確定しない。");
        }
        P.Detail+=TEXT("\n直前の全軍更新：")+P.Last+TEXT(" / ")+(Last.reason==JourneyDefenseReason::Overwhelmed ? FString(TEXT("敵を退けきれなかった")):DefenseCause(Last.reason));
        if(Last.guard>=0) P.Detail+=TEXT(" / 判定時の守備隊：")+JourneyText(J.world.generals[J.world.armies[Last.guard].general].name);
        P.Detail+=TEXT("。両翼の実判定がそろった時だけ維持が進む。");
        H.MissionDetail+=TEXT("\n\n")+P.Detail;H.Defenses.Add(MoveTemp(P));
    }
}
