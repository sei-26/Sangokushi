#include "TacticalTrial.h"
#include "TacticalActivity.h"
FUnitActivity ATacticalField::ReadUnitActivity(int Id) const
{ return gunyuactivity::Read(Id,VisualUnits(),CurrentBeat(),BeatProgress()); }
FString ATacticalField::ReadPlaybackReport() const
{
    const auto* B=CurrentBeat();if(!B) return FString();
    const auto C=ReadTacticCinema();
    if(C.bVisible && !C.bResolved) return C.Tactic+TEXT(" / 発動準備：")+C.Voice;
    if(BeatProgress()>=.48f) return B->Reason;
    // Normal moves/enemy beats must not print the resolved report ahead of time.
    if(B->Kind==EBattleCue::Enemy) return TEXT("敵の指揮を確認中。視認している部隊の動きを追おう。");
    const auto R=ReadUnitActivity(B->Actor);const auto& U=VisualUnits();
    if(R.Partner>=0 && U.IsValidIndex(R.Partner)) return R.Caption+TEXT(" / 対象：")+U[R.Partner].Name;
    return R.Caption.IsEmpty() ? TEXT("部隊の行動を確認中。"):R.Caption;
}
