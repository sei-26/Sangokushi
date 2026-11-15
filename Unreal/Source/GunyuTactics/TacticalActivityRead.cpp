#include "TacticalTrial.h"
#include "TacticalActivity.h"
FUnitActivity ATacticalField::ReadUnitActivity(int Id) const
{ return gunyuactivity::Read(Id,VisualUnits(),CurrentBeat(),BeatProgress()); }
FString ATacticalField::ReadPlaybackReport() const
{
    const auto* B=CurrentBeat();if(!B) return FString();
    const auto C=ReadTacticCinema();
    if(C.bVisible && !C.bResolved) return C.Tactic+TEXT(" / 発動準備：")+C.Voice;
    const float Progress=BeatProgress();
    if(B->Kind==EBattleCue::Enemy)
    {
        if(B->MissionBefore.Convoy>=0 && Progress<.78f) return TEXT("交戦と護送車の動きを確認中。");
        if(B->MissionBefore.Held>=0 && Progress<.84f) return TEXT("交戦を確認中。烽火の守備はこの指揮の終わりに判定。");
    }
    if(Progress>=.48f) return B->Reason;
    // Normal moves/enemy beats must not print the resolved report ahead of time.
    if(B->Kind==EBattleCue::Enemy) return TEXT("敵の指揮を確認中。視認している部隊の動きを追おう。");
    const auto R=ReadUnitActivity(B->Actor);const auto& U=VisualUnits();
    if(R.Partner>=0 && U.IsValidIndex(R.Partner)) return R.Caption+TEXT(" / 対象：")+U[R.Partner].Name;
    return R.Caption.IsEmpty() ? TEXT("部隊の行動を確認中。"):R.Caption;
}
