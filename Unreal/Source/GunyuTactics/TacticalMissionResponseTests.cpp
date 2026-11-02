#include "TacticalMissionResponse.h"
namespace gunyumission
{
    bool SmokeTest()
    {
        auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_MISSION_RESPONSE: FAIL %s"),Why);return V; };
        FBattleBeat B;B.Kind=EBattleCue::Enemy;B.Before.SetNum(6);
        const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
        for(int I=0;I<3;++I) { B.Before[I].Id=I;B.Before[I].Name=Names[I];B.Before[I].Troops=1400; }
        B.Before[5].Id=5;B.Before[5].Troops=600;B.After=B.Before;B.After[5].Position.X+=100;
        B.MissionBefore.bBattle=B.MissionAfter.bBattle=true;B.MissionBefore.Convoy=0;B.MissionAfter.Convoy=1;
        B.MissionBefore.Total=B.MissionAfter.Total=6;B.MissionAfter.bRescue=true;
        const auto First=Read(&B);
        if(!Check(First.Actor==0 && First.ReplyActor==2 && First.Caption.Contains(TEXT("1/6")) && First.Line.Contains(TEXT("軍師")),TEXT("real first movement has personal recognition and a distinct ally reply"))) return false;
        B.After[4].Id=4;B.After[4].Troops=9000;B.After[4].Position.X=99999;
        if(!Check(Read(&B).Line==First.Line && Read(&B).Reply==First.Reply,TEXT("private enemy data cannot choose the response"))) return false;
        B.MissionBefore.Convoy=1;B.MissionAfter.Convoy=2;
        if(!Check(Read(&B).Line.IsEmpty(),TEXT("ordinary segments do not repeat chatter every command"))) return false;
        B.MissionBefore.Convoy=2;B.MissionAfter.Convoy=3;
        if(!Check(Read(&B).Caption.Contains(TEXT("道半ば")),TEXT("halfway progress has a different milestone"))) return false;
        B.After[5].Position=B.Before[5].Position;
        if(!Check(Read(&B).Line.IsEmpty(),TEXT("a counter alone cannot fabricate cart movement"))) return false;
        B.MissionBefore.Convoy=B.MissionAfter.Convoy=-1;B.MissionBefore.Held=0;B.MissionAfter.Held=1;
        B.MissionBefore.Required=B.MissionAfter.Required=3;
        if(!Check(Read(&B).Actor==1 && Read(&B).Caption.Contains(TEXT("1/3")),TEXT("actual first maintained guard is recognized by Guan Yu"))) return false;
        B.MissionBefore.Held=1;B.MissionAfter.Held=2;
        if(!Check(Read(&B).Line.IsEmpty(),TEXT("unchanged holding rhythm is quiet"))) return false;
        B.MissionAfter.Held=0;
        if(!Check(Read(&B).Caption.Contains(TEXT("途切れ")) && Read(&B).Sound==gunyuresponse::ESound::None,TEXT("lost progress asks for recovery without a triumph cue"))) return false;
        B.After[1].Troops=0;
        if(!Check(Read(&B).Actor==0 && Read(&B).ReplyActor==2,TEXT("a defeated officer cannot speak or answer"))) return false;
        B.MissionAfter.bBattle=false;
        if(!Check(Read(&B).Line.IsEmpty(),TEXT("terminal outcomes stay with the existing aftermath"))) return false;
        B.MissionAfter.bBattle=true;B.Kind=EBattleCue::March;
        if(!Check(Read(&B).Line.IsEmpty() && Read(nullptr).Line.IsEmpty(),TEXT("movement orders and empty reads cannot invent objective progress"))) return false;
        UE_LOG(LogTemp,Display,TEXT("GUNYU_MISSION_RESPONSE: PASS (actual cart/guard milestones, quiet intermediate steps, recovery, living speakers, private data and terminal guards)"));return true;
    }
}
