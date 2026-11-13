#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyEnemyDoctrine.hpp"
#include "JourneyEpisode.hpp"

void ATacticalField::ToggleThreatOverlay()
{
    if (IsPlaying() || State->Journey.stage != frontline::JourneyStage::Battle) return;
    State->bThreatOverlay = !State->bThreatOverlay;
    Refresh();
}

void ATacticalField::BuildEnemyPresentation()
{
    const auto& J = State->Journey;
    auto& H = State->HUD;
    H.bThreatOverlay = State->bThreatOverlay;
    H.EnemyPlanHint.Reset();
    H.EnemyCounter = TEXT("敵の札を選ぶと、対処のヒントと推定攻撃圏を表示。");
    for (const auto& Plan : J.ReadEnemyPlans())
    {
        const auto* Seen = J.intelligence[0].Find(Plan.army);
        if (!Seen || !Seen->current) continue;
        FEnemyReadPresentation Read;
        Read.Tile = Seen->observed.tile;
        Read.Name = JourneyText(J.world.generals[Seen->observed.general].name);
        Read.Intent = JourneyText(frontline::EnemyIntentName(Plan.intent));
        Read.Counter = JourneyText(frontline::EnemyIntentCounter(Plan.intent));
        const frontline::JourneyEnemyContext Context{J.Definition().kind,J.goal,J.Definition().secondGoal,J.enemyPolicy,J.HasConvoy(),J.campaignFlow,J.HasSunJian()};
        const FString DoctrineHint=JourneyText(frontline::EnemyDoctrineHint(Plan,Context));
        if(!DoctrineHint.IsEmpty()) Read.Counter=DoctrineHint;
        if (Seen->observed.stance == battle::Stance::Guard && Plan.intent != frontline::JourneyEnemyIntent::Recover)
            Read.Counter = TEXT("固守中は誘導できない。牽制→崩し、または固有戦法で崩そう。");
        if(J.campaignFlow && Plan.intent==frontline::JourneyEnemyIntent::Guard && Plan.next!=Plan.from)
        { Read.Intent=TEXT("包囲を避けて再配置する見込み");Read.Counter=TEXT("移動できれば固守が緩む。混乱で足を止め、挟撃する手もある。"); }
        if(Read.Name==JourneyText(frontline::EpisodeSpeakerName(frontline::ZhangLiang)))
            Read.Counter+=TEXT("\n張梁は士気が落ちると、接敵中に一度だけ鼓舞。混乱で止められる。");
        if(J.HasSunJian() && Plan.army==3)
            Read.Counter=TEXT("呂布は平地で接敵すると騎兵突撃。森へ引くか、混乱で発動を止めよう。");
        Read.bSelected = Plan.army == State->Target;
        if (Read.bSelected)
        {
            H.EnemyCounter = Read.Counter;
            H.EnemyPlanHint=DoctrineHint;
            H.ThreatFrom = Plan.from; H.ThreatNext = Plan.next;
            for (int Tile : Plan.danger) H.ThreatCells.Add(Tile);
        }
        H.EnemyReads.Add(MoveTemp(Read));
    }
    if (H.EnemyReads.IsEmpty()) H.EnemyCounter = TEXT("現在視認している敵はいない。視界外の状況は不明。");
}
