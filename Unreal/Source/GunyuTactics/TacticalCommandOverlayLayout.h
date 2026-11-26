#pragma once
// Shared by the actual panels and the nameplate placement/verification.
namespace gunyucommandlayout
{
    constexpr float EnemyTop=116,EnemyWidth=1000,EnemyHeight=88;
    constexpr float FeedTop=690,FeedWidth=1240,FeedHeight=78;
    constexpr float EnemyDetailTop(bool Detailed) { return Detailed ? 174.f:210.f; }
}
