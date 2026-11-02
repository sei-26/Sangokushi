#include "TacticalMissionResponse.h"
namespace gunyumission
{
    bool CreditSmokeTest()
    {
        auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_MISSION_CREDIT_READ: FAIL %s"),Why);return V; };
        FBattleBeat B;B.Kind=EBattleCue::Enemy;B.Before.SetNum(6);
        const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
        for(int I=0;I<3;++I) { B.Before[I].Id=I;B.Before[I].Troops=1400;B.Before[I].Name=Names[I]; }
        B.Before[5].Id=5;B.Before[5].Troops=600;B.After=B.Before;B.After[5].Position.X=100;
        B.MissionBefore.bBattle=B.MissionAfter.bBattle=true;B.MissionBefore.Convoy=1;B.MissionAfter.Convoy=2;B.MissionAfter.Total=6;B.RoadClearedBy=2;
        auto Road=Read(&B);
        if(!Check(Road.Actor==2 && Road.Caption.Contains(TEXT("張飛隊")) && Road.ReplyActor==0 && Road.Reply.Contains(TEXT("翼徳")),TEXT("the real road clearer owns the portrait and a familiar acknowledgment on an ordinary segment"))) return false;
        B.After[4].Id=4;B.After[4].Troops=9999;B.After[4].Position.X=90000;
        if(!Check(ReadCredit(&B).Line==Road.Line,TEXT("private enemies cannot select the credited officer"))) return false;
        B.MissionAfter.Convoy=1;
        if(!Check(ReadCredit(&B).Line.IsEmpty(),TEXT("clearing a road without actual convoy progress is not yet credited"))) return false;
        B.MissionAfter.Convoy=2;B.After[5].Position=B.Before[5].Position;
        if(!Check(ReadCredit(&B).Line.IsEmpty(),TEXT("a predicted count without cart motion grants no credit"))) return false;
        B.After[5].Position.X=100;B.After[2].Troops=0;
        if(!Check(ReadCredit(&B).Line.IsEmpty(),TEXT("a defeated clearer cannot speak"))) return false;
        B.After[2].Troops=1400;B.RoadClearedBy=-1;B.MissionBefore.Convoy=B.MissionAfter.Convoy=-1;
        B.MissionBefore.Held=1;B.MissionAfter.Held=2;B.MissionBefore.Required=B.MissionAfter.Required=3;B.ReliefIncoming=0;B.ReliefOutgoing=2;
        auto Relief=Read(&B);
        if(!Check(Relief.Actor==0 && Relief.ReplyActor==2 && Relief.Caption.Contains(TEXT("張飛 → 劉備")) && Relief.Line.Contains(TEXT("引き継いだ")),TEXT("confirmed handover names both actual participants and thanks the successor"))) return false;
        B.bReliefRecovered=true;
        if(!Check(ReadCredit(&B).Caption.Contains(TEXT("取り戻")) && !ReadCredit(&B).Caption.Contains(TEXT("つないだ")),TEXT("recovered guard does not claim uninterrupted maintenance"))) return false;
        B.MissionAfter.Held=0;
        if(!Check(ReadCredit(&B).Line.IsEmpty() && Read(&B).Caption.Contains(TEXT("途切れ")),TEXT("lost progress takes recovery precedence over a handover triumph"))) return false;
        B.MissionAfter.Held=2;B.MissionAfter.bBattle=false;
        if(!Check(ReadCredit(&B).Line.IsEmpty() && ReadCredit(nullptr).Line.IsEmpty(),TEXT("terminal or empty snapshots never invent personal credit"))) return false;
        UE_LOG(LogTemp,Display,TEXT("GUNYU_MISSION_CREDIT_READ: PASS (named actual participants, real progress gates, private/dead actor protection, recovery and terminal guards)"));return true;
    }
}
