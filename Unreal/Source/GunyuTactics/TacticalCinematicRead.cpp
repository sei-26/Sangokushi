#include "TacticalTrial.h"
#include "TacticalCinematicStyle.h"
FTacticCinematicRead ATacticalField::ReadTacticCinema() const
{
    FTacticCinematicRead R;const auto* B=CurrentBeat();
    if(!B || !gunyucinema::IsTactic(B->Kind) || !B->Before.IsValidIndex(B->Actor) || B->Before[B->Actor].Id<0) return R;
    const float P=BeatProgress();
    R.bVisible=true;R.Actor=B->Actor;R.Target=B->Target;R.Officer=B->Before[B->Actor].Name;
    R.Tactic=gunyucinema::Name(B->Kind);R.Voice=gunyucinema::Voice(B->Kind);R.Accent=gunyucinema::Color(B->Kind);
    R.Chain=B->ChainStep>0 ? FString::Printf(TEXT("連鎖 / 第%d手%s"),B->ChainStep,B->bSignature ? TEXT(" ・ 固有戦法") : TEXT("")) : TEXT("部隊の行動");
    R.Intro=FMath::SmoothStep(0.f,.07f,P)*(1-FMath::SmoothStep(.28f,.43f,P));
    R.CameraWeight=FMath::SmoothStep(.06f,.28f,P)*(1-FMath::SmoothStep(.82f,.99f,P));
    R.bResolved=P>=.48f;R.bHit=R.bResolved && B->Kind!=EBattleCue::Stop;
    R.Impact=R.bResolved ? FMath::SmoothStep(.48f,.53f,P)*(1-FMath::SmoothStep(.81f,.98f,P)) : 0;
    if(!R.bResolved) return R; // Never report future damage or state changes before impact.
    if(B->Kind==EBattleCue::Stop) { R.Primary=TEXT("連鎖停止");R.Secondary=TEXT("条件を作り直し、次の指揮へ");return R; }
    if(B->Kind==EBattleCue::Rally)
    {
        int Count=0,Gain=0;
        for(int I=0;I<B->Before.Num();++I)
            if(B->After.IsValidIndex(I) && B->Before[I].Id>=0 && !B->Before[I].bEnemy && B->Before[I].Troops>0 && gunyucinema::SupportChanged(B->Before[I],B->After[I]))
            { ++Count;Gain+=FMath::Max(0,B->After[I].Morale-B->Before[I].Morale); }
        R.Primary=Gain>0 ? FString::Printf(TEXT("士気 +%d"),Gain) : TEXT("鼓舞");R.Secondary=FString::Printf(TEXT("味方%d隊 / 士気合計 +%d / 次撃へ"),Count,Gain);return R;
    }
    if(!B->Before.IsValidIndex(B->Target) || !B->After.IsValidIndex(B->Target) || B->Before[B->Target].Id<0 || B->After[B->Target].Id<0) return R;
    const auto& A=B->Before[B->Target];const auto& Z=B->After[B->Target];
    const int Loss=FMath::Max(0,A.Troops-Z.Troops),Morale=FMath::Max(0,A.Morale-Z.Morale);
    if(B->Kind==EBattleCue::Lure)
    {
        R.Primary=A.Position.Equals(Z.Position) ? TEXT("誘導不成立") : TEXT("敵を引き出した");
        const int BaitLoss=B->After.IsValidIndex(B->Actor) ? FMath::Max(0,B->Before[B->Actor].Troops-B->After[B->Actor].Troops) : 0;
        R.Secondary=FString::Printf(TEXT("囮兵 -%d / 次の手へつなぐ"),BaitLoss);return R;
    }
    R.Primary=Loss>0 ? FString::Printf(TEXT("−%d"),Loss) : TEXT("損害なし");
    R.Secondary=TEXT("敵兵損耗");
    if(Morale>0) R.Secondary+=FString::Printf(TEXT(" / 士気 −%d"),Morale);
    if(!A.bDisordered && Z.bDisordered) R.Secondary+=TEXT(" / 混乱");
    else if(!A.bPressured && Z.bPressured) R.Secondary+=TEXT(" / 隙");
    if(A.bGuarded && !Z.bGuarded) R.Secondary+=TEXT(" / 固守解除");
    if(!A.Position.Equals(Z.Position)) R.Secondary+=TEXT(" / 押し返し");
    return R;
}
