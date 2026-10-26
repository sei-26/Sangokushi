#include "TacticalTrial.h"
#include "TacticalState.h"

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
        if (Seen->observed.stance == battle::Stance::Guard && Plan.intent != frontline::JourneyEnemyIntent::Recover)
            Read.Counter = TEXT("固守中は誘導できない。牽制→崩し、または固有戦法で崩そう。");
        Read.bSelected = Plan.army == State->Target;
        if (Read.bSelected)
        {
            H.EnemyCounter = Read.Counter;
            H.ThreatFrom = Plan.from; H.ThreatNext = Plan.next;
            for (int Tile : Plan.danger) H.ThreatCells.Add(Tile);
        }
        H.EnemyReads.Add(MoveTemp(Read));
    }
    if (H.EnemyReads.IsEmpty()) H.EnemyCounter = TEXT("現在視認している敵はいない。視界外の状況は不明。");
}
