#include "TacticalBattleResponse.h"
#include "Sound/SoundWave.h"
namespace gunyuresponse
{
    bool SmokeTest()
    {
        auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_BATTLE_RESPONSE: FAIL %s"),Why);return V; };
        TArray<FVisualUnit> U;U.SetNum(6);
        const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛"),TEXT("黄巾先鋒")};
        for(int I=0;I<4;++I) { U[I].Id=I;U[I].Troops=1400;U[I].Name=Names[I];U[I].bEnemy=I==3;U[I].Morale=90; }
        FBattleBeat B;B.Actor=1;B.Target=3;B.Kind=EBattleCue::Ambush;B.Combo=2;B.Before=B.After=U;
        B.Before[1].bConcealed=true;B.After[3].Troops=800;
        if(!Check(Read(&B,U,.47f).Line.IsEmpty() && Read(&B,U,.47f).Sound==ESound::None,TEXT("resolved snapshots cannot leak success or sound before impact"))) return false;
        const auto Two=Read(&B,U,.6f,2);
        if(!Check(Two.Actor==1 && Two.Line.Contains(TEXT("仕込み")) && Two.Caption.Contains(TEXT("2連鎖")) && Two.Sound==ESound::Combo2 && Two.Reply.IsEmpty(),TEXT("prepared ambush names its real officer, payoff and current combo only"))) return false;
        const auto Relay=Read(&B,U,.86f,2);
        if(!Check(Relay.Line==Two.Line && Relay.ReplyActor==2 && Relay.Reply.Contains(TEXT("次は俺だ")),TEXT("short queued acknowledgment keeps the current officer's result visible"))) return false;
        B.Combo=3;if(!Check(Read(&B,U,.6f).Sound==ESound::Combo3,TEXT("third actual combo receives its own rising cue"))) return false;
        B.Combo=1;if(!Check(Read(&B,U,.6f).Sound==ESound::Connect && !Read(&B,U,.6f).Caption.Contains(TEXT("連鎖成立")),TEXT("solo tactics retain a payoff without invented combo"))) return false;
        U[3].Id=-1;if(!Check(Read(&B,U,.6f).Line.IsEmpty() && Read(&B,U,.6f).Sound==ESound::None,TEXT("unobserved enemy cannot produce a success line or cue"))) return false;
        B.Kind=EBattleCue::Stop;const auto Stopped=Read(&B,U,.86f,2);
        if(!Check(Stopped.Sound==ESound::Stop && Stopped.Reply.IsEmpty() && Stopped.Caption.Contains(TEXT("立て直")),TEXT("stopped chain is sober and never hands off a fabricated success"))) return false;
        U[3]=B.Before[3];B.Kind=EBattleCue::Disrupt;B.After=B.Before;B.Before[3].bGuarded=true;
        if(!Check(Read(&B,U,.6f).Caption.Contains(TEXT("固守")),TEXT("real guard removal takes priority over a generic hit"))) return false;
        B.Before=B.After=U;B.Kind=EBattleCue::Strike;
        if(!Check(Read(&B,U,.6f,2).Sound==ESound::None && Read(&B,U,.86f,2).Reply.IsEmpty() && Read(&B,U,.6f).Caption.Contains(TEXT("崩せなかった")),TEXT("no-effect strike grants no triumph or handoff"))) return false;
        B.Kind=EBattleCue::Lure;B.After[3].Position+=FVector(100,0,0);
        if(!Check(Read(&B,U,.6f).Caption.Contains(TEXT("誘い出")),TEXT("actual lure movement can succeed without invented casualties"))) return false;
        B.Kind=EBattleCue::Rally;B.Target=-1;B.Before=B.After=U;
        B.After[0].bInspired=true;
        if(!Check(Read(&B,U,.6f).Sound==ESound::Support,TEXT("real inspiration is acknowledged even without healing or morale gain"))) return false;
        B.After=B.Before;
        if(!Check(Read(&B,U,.6f).Sound==ESound::None && Read(&B,U,.86f,2).Reply.IsEmpty(),TEXT("unchanged allies do not create a support payoff"))) return false;
        U[1].Id=-1;if(!Check(Read(&B,U,.6f).Line.IsEmpty(),TEXT("missing actor has no phantom speaker"))) return false;
        if(!Check(Read(nullptr,U,.6f).Weight==0 && Read(&B,U,1).Weight==0,TEXT("no response persists after completion or skip"))) return false;
        for(auto S:{ESound::Connect,ESound::Combo2,ESound::Combo3,ESound::Support,ESound::Stop})
        {
            const auto* Wave=LoadObject<USoundWave>(nullptr,SoundPath(S));
            if(!Check(Wave && Wave->GetDuration()>=.3f && Wave->GetDuration()<=1.f,TEXT("editable PCM response sounds are available and bounded"))) return false;
        }
        UE_LOG(LogTemp,Display,TEXT("GUNYU_BATTLE_RESPONSE: PASS (observed actual payoff, solo/combo, support/stop/no-effect, private target, queued acknowledgment, bounded original audio)"));return true;
    }
}
