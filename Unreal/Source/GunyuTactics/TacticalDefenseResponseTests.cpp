#include "TacticalMissionResponse.h"
#include "StrategistJourney.hpp"
#include "JourneyDefense.hpp"
namespace gunyumission
{
    namespace
    {
        FVector DefensePosition(int X,int Y)
        { const auto P=hexgrid::Center(X,Y);return FVector(P.first*200.,P.second*200.,0); }
        FBattleBeat DefenseResponseCase()
        {
            FBattleBeat B;B.Kind=EBattleCue::Enemy;B.MissionBefore.bBattle=B.MissionAfter.bBattle=true;
            B.MissionBefore.CommandTurn=3;B.MissionAfter.CommandTurn=4;B.MissionBefore.Held=1;B.MissionAfter.Held=0;
            B.MissionBefore.Required=B.MissionAfter.Required=3;B.Before.SetNum(3);
            const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
            for(int Id=0;Id<3;++Id) { auto& U=B.Before[Id];U.Id=Id;U.Name=Names[Id];U.Troops=1400;U.Morale=80; }
            B.Before[0].Position=DefensePosition(6,6);B.Before[1].Position=DefensePosition(7,6);B.Before[2].Position=DefensePosition(8,9);B.After=B.Before;
            FBeaconDefenseProgress N;N.Goal=frontline::Campaign::At(7,6);N.Guard=1;N.Turn=3;N.Reason=int(frontline::JourneyDefenseReason::GuardSpent);N.bAttackSpent=true;
            FBeaconDefenseProgress S;S.Goal=frontline::Campaign::At(8,9);S.Guard=2;S.Turn=3;S.Reason=int(frontline::JourneyDefenseReason::Secured);S.bSecured=S.bGuarded=true;
            B.MissionBefore.PrimaryGoal=B.MissionAfter.PrimaryGoal=N.Goal;B.MissionBefore.SecondaryGoal=B.MissionAfter.SecondaryGoal=S.Goal;
            B.MissionAfter.DefenseChecks={N,S};return B;
        }
    }
    bool DefenseSmokeTest()
    {
        using Reason=frontline::JourneyDefenseReason;
        auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_DEFENSE_RESPONSE: FAIL %s"),Why);return V; };
        auto B=DefenseResponseCase();const auto R=ReadDefense(&B);
        if(!Check(R.Actor==1 && R.ReplyActor==0 && R.Caption.Contains(TEXT("北")) && R.Caption.Contains(TEXT("維持できず")) && R.Reply.Contains(TEXT("雲長")) && !R.Line.Contains(TEXT("撃破")) && !R.Caption.Contains(TEXT("両翼")),TEXT("actual local failed defense names its living guard without inventing attack success or global failure"))) return false;
        Swap(B.MissionAfter.DefenseChecks[0],B.MissionAfter.DefenseChecks[1]);
        if(!Check(ReadDefense(&B).Line.IsEmpty(),TEXT("swapping checks cannot relabel a northern failure as southern"))) return false;
        B=DefenseResponseCase();B.MissionAfter.PrimaryGoal=frontline::Campaign::At(8,6);
        if(!Check(ReadDefense(&B).Line.IsEmpty(),TEXT("checks from another mission's goals cannot become current outcomes"))) return false;
        B=DefenseResponseCase();Swap(B.MissionBefore.PrimaryGoal,B.MissionBefore.SecondaryGoal);Swap(B.MissionAfter.PrimaryGoal,B.MissionAfter.SecondaryGoal);Swap(B.MissionAfter.DefenseChecks[0],B.MissionAfter.DefenseChecks[1]);
        if(!Check(ReadDefense(&B).Caption.Contains(TEXT("南")),TEXT("a real southern failure is labeled by its canonical checked goal"))) return false;
        for(auto Kind:{EBattleCue::Feint,EBattleCue::Stop,EBattleCue::Guard})
        { B=DefenseResponseCase();B.Kind=Kind;if(!Check(ReadDefense(&B).Line.IsEmpty(),TEXT("no defense consequence is anticipated during a tactic or stopped action"))) return false; }
        B=DefenseResponseCase();B.MissionAfter.bBattle=false;
        if(!Check(ReadDefense(&B).Line.IsEmpty() && ReadDefense(nullptr).Line.IsEmpty(),TEXT("terminal results and absent beats do not fabricate a battlefield reply"))) return false;
        B=DefenseResponseCase();--B.MissionAfter.DefenseChecks[0].Turn;
        if(!Check(ReadDefense(&B).Line.IsEmpty(),TEXT("a previous cycle's check cannot replay as the new outcome"))) return false;
        B=DefenseResponseCase();B.MissionAfter.CommandTurn=B.MissionBefore.CommandTurn;
        if(!Check(ReadDefense(&B).Line.IsEmpty(),TEXT("the consequence requires an actual new command cycle"))) return false;
        for(auto Cause:{Reason::Secured,Reason::Vacant,Reason::Depleted,Reason::NeedsGuard,Reason::GuardWeak,Reason::Overwhelmed})
        { B=DefenseResponseCase();B.MissionAfter.DefenseChecks[0].Reason=int(Cause);B.MissionAfter.DefenseChecks[0].bSecured=Cause==Reason::Secured;if(!Check(ReadDefense(&B).Line.IsEmpty(),TEXT("other defense causes cannot claim spent-action guard failure"))) return false; }
        for(int Invalid=0;Invalid<7;++Invalid)
        {
            B=DefenseResponseCase();
            if(Invalid==0) B.After[1].Troops=0;
            if(Invalid==1) B.Before[1].Id=-1;
            if(Invalid==2) B.After[1].bEnemy=true;
            if(Invalid==3) B.After[1].Position=DefensePosition(8,6);
            if(Invalid==4) B.Before[1].Position=DefensePosition(8,6);
            if(Invalid==5) B.After[1].Morale=39;
            if(Invalid==6) B.After[1].Troops=499;
            if(!Check(ReadDefense(&B).Line.IsEmpty(),TEXT("dead unknown nonfriendly weak moved or newly arrived guards do not speak this consequence"))) return false;
        }
        B=DefenseResponseCase();B.MissionAfter.DefenseChecks[0].bAttackSpent=false;
        if(!Check(ReadDefense(&B).Line.IsEmpty(),TEXT("an unspent guard cannot claim its attack was used"))) return false;
        B=DefenseResponseCase();B.MissionAfter.DefenseChecks[0].bGuarded=true;
        if(!Check(ReadDefense(&B).Line.IsEmpty(),TEXT("contradictory actual guard facts are rejected"))) return false;
        B=DefenseResponseCase();B.MissionAfter.DefenseChecks[1].Goal=B.MissionAfter.DefenseChecks[0].Goal;
        if(!Check(ReadDefense(&B).Line.IsEmpty(),TEXT("duplicate wing metadata cannot represent two checked beacons"))) return false;
        B=DefenseResponseCase();B.MissionAfter.DefenseChecks.Pop();
        if(!Check(ReadDefense(&B).Line.IsEmpty(),TEXT("partial checks are not a complete cycle snapshot"))) return false;
        B=DefenseResponseCase();B.MissionAfter.Held=1;
        if(!Check(ReadDefense(&B).Line.IsEmpty(),TEXT("a contradictory advancing global hold count cannot report a failed wing"))) return false;
        B=DefenseResponseCase();B.After[0].Troops=B.After[2].Troops=0;
        if(!Check(ReadDefense(&B).Actor==1 && ReadDefense(&B).Reply.IsEmpty(),TEXT("the living guard can report alone without a dead reply"))) return false;
        UE_LOG(LogTemp,Display,TEXT("GUNYU_DEFENSE_RESPONSE: PASS (actual next-cycle spent guard, exact living local speaker, stale/partial/contradictory/terminal guards and no invented shared attack)"));return true;
    }
}
