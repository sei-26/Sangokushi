#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

namespace
{
    const TCHAR* NoticeKind(JourneyNoticeKind Kind)
    {
        switch (Kind)
        {
        case JourneyNoticeKind::Critical:return TEXT("損耗に注意");
        case JourneyNoticeKind::Defeated:return TEXT("壊滅");
        case JourneyNoticeKind::ConvoyHit:return TEXT("護送車に被害");
        case JourneyNoticeKind::Stalled:return TEXT("行軍が停滞");
        case JourneyNoticeKind::TargetLost:return TEXT("追撃対象を見失った");
        default:return TEXT("敵を捕捉");
        }
    }
}
void ATacticalField::RecordBattleWatch(const JourneyWatchSnapshot& Before)
{
    if (!bRealtimeCommand) return;
    const auto Notices=State->BattleWatch.Record(Before,JourneyBattleWatch::Capture(State->Journey,State->Realtime));
    if (Notices.empty()) return;
    State->bWatchPending=State->bWatchPauseAfterPlayback=true;
    if (IsPlaying()) return; // Publish only when this movement/damage has been shown.
    if (bPauseOnBattleAlert) State->Realtime.paused=true;
    State->bWatchPauseAfterPlayback=false;
    PublishBattleNotices();
}
void ATacticalField::PublishBattleNotices()
{
    if (!State->bWatchPending || IsPlaying()) return;
    State->bWatchPending=false;
    const auto& History=State->BattleWatch.History();
    if (History.empty()) return;
    const auto& Notice=History.front();
    State->Report=FString::Printf(TEXT("伝令 / %s：%s。%s  Lで戦況履歴。"),*JourneyText(Notice.name),NoticeKind(Notice.kind),*JourneyText(Notice.text));
}
void ATacticalField::BuildBattleWatchPresentation()
{
    auto& H=State->HUD;H.bAlertPause=bPauseOnBattleAlert;
    for (const auto& Notice : State->BattleWatch.History())
    {
        FBattleNoticePresentation Item;Item.Army=Notice.army;
        Item.Title=FString::Printf(TEXT("更新%d / %s / %s"),FMath::Max(0,Notice.turn-1),*JourneyText(Notice.name),NoticeKind(Notice.kind));
        Item.Detail=JourneyText(Notice.text);H.BattleNotices.Add(MoveTemp(Item));
    }
}
