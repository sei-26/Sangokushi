#include "TacticalMissionResponse.h"
#include "StrategistJourney.hpp"
namespace gunyumission
{
    namespace
    {
        FVector Position(int X,int Y)
        { const auto P=hexgrid::Center(X,Y);return FVector(P.first*200.,P.second*200.,0); }
        FBattleBeat SupportBeat()
        {
            FBattleBeat B;B.Kind=EBattleCue::Lure;B.Actor=0;B.Target=3;B.SupportGuard=1;B.SupportGoal=frontline::Campaign::At(7,6);B.SupportNorth=true;
            B.MissionBefore.bBattle=B.MissionAfter.bBattle=true;B.Before.SetNum(6);
            const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛"),TEXT("敵先鋒")};
            for(int I=0;I<4;++I) { B.Before[I].Id=I;B.Before[I].Troops=1400;B.Before[I].Name=Names[I];B.Before[I].bEnemy=I==3; }
            B.Before[0].Position=Position(7,8);B.Before[1].Position=Position(7,6);B.Before[2].Position=Position(10,10);B.Before[3].Position=Position(7,7);
            B.After=B.Before;B.After[0].Position=Position(7,9);B.After[0].Troops-=60;B.After[3].Position=B.Before[0].Position;return B;
        }
    }
    bool SupportSmokeTest()
    {
        auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_SUPPORT_RESPONSE: FAIL %s"),Why);return V; };
        auto B=SupportBeat();const auto Pulled=ReadSupport(&B);
        if(!Check(Pulled.Actor==0 && Pulled.ReplyActor==1 && Pulled.Caption.Contains(TEXT("北")) && Pulled.Caption.Contains(TEXT("引き離")) && Pulled.Reply.Contains(TEXT("兄者")) && !Pulled.Line.Contains(TEXT("守り切")),TEXT("actual lure away names the supporter and the guard without claiming objective success"))) return false;
        B.Before[4].Id=4;B.Before[4].Troops=9999;B.After[4]=B.Before[4];B.After[4].Position=Position(14,12);
        if(!Check(ReadSupport(&B).Line==Pulled.Line && ReadSupport(&B).Reply==Pulled.Reply,TEXT("unrelated enemy snapshots never choose the credited officers"))) return false;
        B=SupportBeat();B.SupportNorth=false;B.Kind=EBattleCue::Breakthrough;B.Before[0].Position=Position(8,6);B.After[0].Position=B.Before[3].Position;
        const auto Pushed=ReadSupport(&B);
        if(!Check(Pushed.Caption.Contains(TEXT("南")) && Pushed.Caption.Contains(TEXT("押し返")) && Pushed.ReplyActor==1,TEXT("actual push away has its distinct local outcome and chosen wing"))) return false;
        B.After[0].Position=B.Before[0].Position;B.After[3].Position=B.Before[3].Position;B.After[3].Troops=0;
        if(!Check(ReadSupport(&B).Caption.Contains(TEXT("撃破")) && !ReadSupport(&B).Caption.Contains(TEXT("押し返")),TEXT("actual defeat is separate from displacement"))) return false;
        for(const auto Kind:{EBattleCue::Ambush,EBattleCue::Feint,EBattleCue::Disrupt})
        {
            B=SupportBeat();B.Kind=Kind;B.After[0].Position=B.Before[0].Position;B.After[3].Position=B.Before[3].Position;B.After[3].Troops=0;
            if(!Check(ReadSupport(&B).Caption.Contains(TEXT("撃破")) && ReadSupport(&B).ReplyActor==1,TEXT("all ordinary attacking tactics acknowledge an actual local defeat with the present guard"))) return false;
            B.After[3].Troops=200;
            if(!Check(ReadSupport(&B).Line.IsEmpty(),TEXT("a nonlethal setup attack does not invent threat removal"))) return false;
        }
        B=SupportBeat();B.Before[0].Position=Position(8,6);B.After[0].Position=Position(9,6);B.After[3].Position=B.Before[0].Position;
        if(!Check(ReadSupport(&B).Line.IsEmpty(),TEXT("lure around the beacon within adjacency does not claim separation"))) return false;
        B=SupportBeat();B.After[3].Position=B.Before[3].Position;
        if(!Check(ReadSupport(&B).Line.IsEmpty(),TEXT("a successful cue without displacement has no separation claim"))) return false;
        B=SupportBeat();B.After[1].Position=Position(8,6);
        if(!Check(ReadSupport(&B).Line.IsEmpty(),TEXT("a guard who left the beacon cannot answer as its present guard"))) return false;
        for(int Id:{0,1})
        {
            B=SupportBeat();B.After[Id].Troops=0;if(!Check(ReadSupport(&B).Line.IsEmpty(),TEXT("defeated supporter or guard cannot speak"))) return false;
            B=SupportBeat();B.Before[Id].bEnemy=true;if(!Check(ReadSupport(&B).Line.IsEmpty(),TEXT("enemy officers cannot receive friendly support credit"))) return false;
        }
        B=SupportBeat();B.Before[3].Id=-1;
        if(!Check(ReadSupport(&B).Line.IsEmpty(),TEXT("an enemy unknown before the action cannot supply personal credit"))) return false;
        B=SupportBeat();B.After[3].Id=-1;
        if(!Check(ReadSupport(&B).Line.IsEmpty(),TEXT("missing target result cannot fabricate displacement or death"))) return false;
        B=SupportBeat();B.Before[3].Troops=0;
        if(!Check(ReadSupport(&B).Line.IsEmpty(),TEXT("an already defeated target is not a new support success"))) return false;
        B=SupportBeat();B.SupportGuard=0;
        if(!Check(ReadSupport(&B).Line.IsEmpty(),TEXT("supporter cannot thank itself as a different guard"))) return false;
        B=SupportBeat();B.SupportGoal=-1;
        if(!Check(ReadSupport(&B).Line.IsEmpty(),TEXT("ordinary unannotated tactics do not invent a support role"))) return false;
        B=SupportBeat();B.Kind=EBattleCue::Stop;
        if(!Check(ReadSupport(&B).Line.IsEmpty(),TEXT("stopped tactics cannot receive support success"))) return false;
        B=SupportBeat();B.MissionAfter.bBattle=false;
        if(!Check(ReadSupport(&B).Line.IsEmpty() && ReadSupport(nullptr).Line.IsEmpty(),TEXT("terminal and absent results belong to the existing aftermath"))) return false;
        UE_LOG(LogTemp,Display,TEXT("GUNYU_SUPPORT_RESPONSE: PASS (actual local lure/push/ordinary tactic defeat, distinct wings, living named guard, no fabricated separation or nonlethal removal, private/unknown/stopped/terminal guards)"));return true;
    }
}
