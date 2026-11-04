#include "TacticalMissionResponse.h"
#include "StrategistJourney.hpp"
#include "JourneyDefense.hpp"
namespace gunyumission
{
    namespace
    {
        int DefenseTile(const FVector& P)
        {
            if(!FMath::IsFinite(P.X) || !FMath::IsFinite(P.Y) || FMath::Abs(P.X)>100000 || FMath::Abs(P.Y)>100000) return -1;
            const auto Cell=hexgrid::Pick(P.X/200.,P.Y/200.);
            return Cell.first>=0 && Cell.first<frontline::StrategistJourney::Columns && Cell.second>=0 && Cell.second<frontline::StrategistJourney::Rows
                ? frontline::Campaign::At(Cell.first,Cell.second):-1;
        }
    }
    gunyuresponse::FRead ReadDefense(const FBattleBeat* B)
    {
        using Reason=frontline::JourneyDefenseReason;
        gunyuresponse::FRead R;if(!B || B->Kind!=EBattleCue::Enemy || B->bSignature) return R;
        const auto& A=B->MissionBefore;const auto& Z=B->MissionAfter;
        if(!A.bBattle || !Z.bBattle || A.Held<0 || Z.Held!=0 || A.Required<=0 || A.Required!=Z.Required ||
           A.CommandTurn<=0 || Z.CommandTurn!=A.CommandTurn+1 || Z.DefenseChecks.Num()!=2) return R;
        if(!frontline::StrategistJourney::OnMap(A.PrimaryGoal) || !frontline::StrategistJourney::OnMap(A.SecondaryGoal) ||
           A.PrimaryGoal==A.SecondaryGoal || Z.PrimaryGoal!=A.PrimaryGoal || Z.SecondaryGoal!=A.SecondaryGoal ||
           Z.DefenseChecks[0].Goal!=Z.PrimaryGoal || Z.DefenseChecks[1].Goal!=Z.SecondaryGoal) return R;
        for(const auto& C:Z.DefenseChecks)
        {
            if(!frontline::StrategistJourney::OnMap(C.Goal) || C.Turn!=A.CommandTurn || C.Guard< -1 || C.Guard>=3 ||
               C.Reason<=int(Reason::Unchecked) || C.Reason>int(Reason::Overwhelmed) ||
               C.bSecured!=(C.Reason==int(Reason::Secured)) || (C.Guard<0)!=(C.Reason==int(Reason::Vacant)) || (C.bGuarded && C.bAttackSpent)) return R;
        }
        if(Z.DefenseChecks[0].Goal==Z.DefenseChecks[1].Goal) return R;
        auto Living=[&](int Id)
        {
            return Id>=0 && Id<3 && B->Before.IsValidIndex(Id) && B->After.IsValidIndex(Id) &&
                B->Before[Id].Id==Id && B->After[Id].Id==Id && !B->Before[Id].bEnemy && !B->After[Id].bEnemy &&
                B->Before[Id].Troops>0 && B->After[Id].Troops>0;
        };
        for(int Wing=0;Wing<2;++Wing)
        {
            const auto& C=Z.DefenseChecks[Wing];const int G=C.Guard;
            if(C.Reason!=int(Reason::GuardSpent) || C.bSecured || C.bGuarded || !C.bAttackSpent || !Living(G) ||
               B->After[G].Troops<500 || B->After[G].Morale<40 || DefenseTile(B->Before[G].Position)!=C.Goal || DefenseTile(B->After[G].Position)!=C.Goal) continue;
            R.Actor=G;R.Speaker=B->After[G].Name;R.Weight=1;
            R.Caption=FString(Wing==0 ? TEXT("北"):TEXT("南"))+TEXT("の烽火 / 固守なし・維持できず");
            const TCHAR* Lines[]={TEXT("手は使った。軍師、次はこの火を守る一手を！"),TEXT("軍師、手は使った。次は敵を退け、火を守ろう。"),TEXT("手を使っちまった！ 軍師、次はどう守る！")};
            R.Line=Lines[G];
            for(int Id:{0,1,2}) if(Id!=G && Living(Id)) { R.ReplyActor=Id;break; }
            if(R.ReplyActor>=0)
            {
                const TCHAR* Address[]={TEXT("兄者"),TEXT("雲長"),TEXT("翼徳")};
                const TCHAR* Replies[]={TEXT("%s、次の守りを合わせよう。"),TEXT("%s、次は守りを整えましょう。"),TEXT("%s、分かった！ 次の采配を待つぜ！")};
                R.Reply=B->After[R.ReplyActor].Name+TEXT("「")+FString(Replies[R.ReplyActor]).Replace(TEXT("%s"),Address[G])+TEXT("」");
            }
            return R;
        }
        return R;
    }
}
