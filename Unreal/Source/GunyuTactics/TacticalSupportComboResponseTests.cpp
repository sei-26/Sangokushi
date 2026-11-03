#include "TacticalMissionResponse.h"
#include "StrategistJourney.hpp"
namespace gunyumission
{
    namespace
    {
        FVector ComboPosition(int X,int Y)
        { const auto P=hexgrid::Center(X,Y);return FVector(P.first*200.,P.second*200.,0); }
        TArray<FBattleBeat> ComboResponseCase()
        {
            FBattleBeat A;A.Kind=EBattleCue::Feint;A.Actor=0;A.Target=3;A.SupportGuard=1;A.SupportGoal=frontline::Campaign::At(7,6);A.SupportNorth=true;
            A.MissionBefore.bBattle=A.MissionAfter.bBattle=true;A.ChainStep=A.Combo=1;A.ComboPercent=100;A.Before.SetNum(4);
            const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛"),TEXT("敵先鋒")};
            for(int Id=0;Id<4;++Id) { auto& U=A.Before[Id];U.Id=Id;U.Troops=1400;U.Morale=90;U.Name=Names[Id];U.bEnemy=Id==3; }
            A.Before[0].Position=ComboPosition(8,7);A.Before[1].Position=ComboPosition(7,6);A.Before[2].Position=ComboPosition(10,10);A.Before[3].Position=ComboPosition(8,6);
            A.After=A.Before;A.After[0].Morale-=10;A.After[3].Troops-=120;A.After[3].Morale-=12;A.After[3].bPressured=true;
            FBattleBeat B=A;B.Kind=EBattleCue::Disrupt;B.Actor=1;B.ChainStep=B.Combo=2;B.ComboPercent=125;B.Before=A.After;B.After=B.Before;
            B.After[1].Morale-=10;B.After[3].Troops-=300;B.After[3].Morale-=18;B.After[3].bPressured=false;B.After[3].bDisordered=true;return {A,B};
        }
    }
    bool SupportComboSmokeTest()
    {
        auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_SUPPORT_COMBO_RESPONSE: FAIL %s"),Why);return V; };
        auto B=ComboResponseCase();const auto Joined=ReadSupportCombo(B);
        if(!Check(Joined.Actor==1 && Joined.ReplyActor==0 && Joined.Caption.Contains(TEXT("攻めをつないだ")) && Joined.Caption.Contains(TEXT("固守は不可")) && Joined.Reply.Contains(TEXT("雲長")) && !Joined.Line.Contains(TEXT("守り切")),TEXT("two completed ordinary attacks name guard and supporter without claiming secured defense"))) return false;
        B[1].After[3].Troops=0;
        if(!Check(ReadSupportCombo(B).Caption.Contains(TEXT("撃破")) && !ReadSupportCombo(B).Caption.Contains(TEXT("引き離")),TEXT("actual local defeat is distinct from displacement"))) return false;
        // Synthetic snapshot boundary: validates the caption reader, not a claim that a stationary guard can push in current rules.
        B=ComboResponseCase();B[1].After[3].Position=ComboPosition(10,6);B[1].SupportNorth=B[0].SupportNorth=false;
        if(!Check(ReadSupportCombo(B).Caption.Contains(TEXT("南")) && ReadSupportCombo(B).Caption.Contains(TEXT("引き離")),TEXT("observed final separation remains local to its annotated wing"))) return false;
        B=ComboResponseCase();B[1].Kind=EBattleCue::Stop;
        if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("a stopped follower cannot publish the first hand as a shared success"))) return false;
        B=ComboResponseCase();B.Pop();
        if(!Check(ReadSupportCombo(B).Line.IsEmpty() && ReadSupportCombo({}).Line.IsEmpty(),TEXT("a first hand alone is not a two-officer result"))) return false;
        for(int Id:{0,1})
        {
            B=ComboResponseCase();B[1].After[Id].Troops=0;
            if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("dead participants do not speak"))) return false;
            B=ComboResponseCase();B[0].Before[Id].Id=-1;
            if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("unknown participants do not receive credit"))) return false;
            B=ComboResponseCase();B[1].After[Id].bEnemy=true;
            if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("nonfriendly participants do not receive friendly credit"))) return false;
        }
        B=ComboResponseCase();B[1].After[1].Position=ComboPosition(8,6);
        if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("a guard who left the beacon cannot claim remaining there"))) return false;
        B=ComboResponseCase();--B[1].Before[3].Troops;
        if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("discontinuous per-event snapshots cannot fabricate a combined outcome"))) return false;
        B=ComboResponseCase();B[1].Target=2;
        if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("unrelated targets do not combine their effects"))) return false;
        B=ComboResponseCase();B[1].SupportGoal=frontline::Campaign::At(8,9);
        if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("different beacon metadata cannot create a shared response"))) return false;
        B=ComboResponseCase();B[1].SupportNorth=false;
        if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("inconsistent wing metadata cannot choose a caption"))) return false;
        B=ComboResponseCase();B[1].Actor=2;
        if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("the annotated guard must actually perform the follow-up"))) return false;
        B=ComboResponseCase();B[1].Combo=1;B[1].ComboPercent=100;
        if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("separate single attacks do not invent a shared combo"))) return false;
        B=ComboResponseCase();B[1].MissionAfter.bBattle=false;
        if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("terminal results belong to the existing aftermath"))) return false;
        B=ComboResponseCase();B[1].bSignature=true;
        if(!Check(ReadSupportCombo(B).Line.IsEmpty(),TEXT("ordinary support decisions do not invent signature authorization"))) return false;
        UE_LOG(LogTemp,Display,TEXT("GUNYU_SUPPORT_COMBO_RESPONSE: PASS (complete two-hand local outcomes, named living participants, guard position, continuous snapshots and stopped/partial/unknown/terminal guards)"));return true;
    }
}
