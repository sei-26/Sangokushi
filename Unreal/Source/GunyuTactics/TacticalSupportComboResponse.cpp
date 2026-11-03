#include "TacticalMissionResponse.h"
#include "StrategistJourney.hpp"
namespace gunyumission
{
    namespace
    {
        int ComboTile(const FVector& P)
        {
            if(!FMath::IsFinite(P.X) || !FMath::IsFinite(P.Y) || FMath::Abs(P.X)>100000 || FMath::Abs(P.Y)>100000) return -1;
            const auto Cell=hexgrid::Pick(P.X/200.,P.Y/200.);
            if(Cell.first<0 || Cell.first>=frontline::StrategistJourney::Columns || Cell.second<0 || Cell.second>=frontline::StrategistJourney::Rows) return -1;
            return frontline::Campaign::At(Cell.first,Cell.second);
        }
        bool SameComboUnit(const FVisualUnit& A,const FVisualUnit& B)
        {
            return A.Id==B.Id && A.Troops==B.Troops && A.Morale==B.Morale && A.Position.Equals(B.Position,.01f) && A.bEnemy==B.bEnemy &&
                A.bConcealed==B.bConcealed && A.bDisordered==B.bDisordered && A.bPressured==B.bPressured && A.bInspired==B.bInspired && A.bGuarded==B.bGuarded;
        }
        bool OrdinarySupportCue(EBattleCue Kind)
        { return Kind==EBattleCue::Lure || Kind==EBattleCue::Ambush || Kind==EBattleCue::Breakthrough || Kind==EBattleCue::Feint || Kind==EBattleCue::Disrupt; }
        int ComboDistance(int A,int B)
        { return hexgrid::Distance(A%frontline::Width,A/frontline::Width,B%frontline::Width,B/frontline::Width); }
    }
    gunyuresponse::FRead ReadSupportCombo(const TArray<FBattleBeat>& Beats)
    {
        gunyuresponse::FRead R;if(Beats.Num()!=2) return R;
        const auto& First=Beats[0];const auto& Last=Beats[1];const int S=First.Actor,G=First.SupportGuard,T=First.Target,Goal=First.SupportGoal;
        if(S<0 || S>=3 || G<0 || G>=3 || S==G || Last.Actor!=G || T<3 || Last.Target!=T || !frontline::StrategistJourney::OnMap(Goal)) return R;
        if(Last.SupportGoal!=Goal || Last.SupportGuard!=G || Last.SupportNorth!=First.SupportNorth ||
           First.ChainStep!=1 || Last.ChainStep!=2 || First.Combo!=1 || Last.Combo!=2 || First.ComboPercent!=100 || Last.ComboPercent!=125) return R;
        for(const auto& B:Beats)
        {
            if(!B.MissionBefore.bBattle || !B.MissionAfter.bBattle || B.bSignature || !OrdinarySupportCue(B.Kind)) return R;
            for(int Id:{S,G,T})
            {
                if(!B.Before.IsValidIndex(Id) || !B.After.IsValidIndex(Id) || B.Before[Id].Id!=Id || B.After[Id].Id!=Id) return R;
                const auto& A=B.Before[Id];const auto& Z=B.After[Id];
                if(Id!=T && (A.bEnemy || Z.bEnemy || A.Troops<=0 || Z.Troops<=0)) return R;
                if(Id==T && (!A.bEnemy || !Z.bEnemy || A.Troops<=0 || Z.Troops<0)) return R;
                if(Id==G && (ComboTile(A.Position)!=Goal || ComboTile(Z.Position)!=Goal)) return R;
            }
        }
        for(int Id:{S,G,T}) if(!SameComboUnit(First.After[Id],Last.Before[Id])) return R;
        const int From=ComboTile(First.Before[T].Position),To=ComboTile(Last.After[T].Position);
        if(From<0 || To<0 || ComboDistance(From,Goal)>1) return R;
        const bool Defeated=Last.After[T].Troops==0,Away=!Defeated && ComboDistance(To,Goal)>1;
        R.Actor=G;R.ReplyActor=S;R.Speaker=Last.After[G].Name;R.Weight=1;R.Sound=gunyuresponse::ESound::Support;
        R.Caption=FString(First.SupportNorth ? TEXT("北"):TEXT("南"))+TEXT("の烽火 / ")+
            (Defeated ? TEXT("連携で撃破"):Away ? TEXT("連携で敵を引き離した"):TEXT("攻めをつないだ"))+TEXT("・今の固守は不可");
        const TCHAR* Joined[]={TEXT("合わせた！ 軍師、二人の一手がつながった。"),TEXT("隙は逃さぬ。軍師、二人の一手がつながった。"),TEXT("よし、合わせたぜ！ 軍師、次を頼む！")};
        const TCHAR* Defeats[]={TEXT("二人で討った！ 軍師、次の采配を。"),TEXT("つないだ一手で敵を討った。軍師、次の指示を。"),TEXT("仕留めたぜ！ 軍師、この連携で次も行こう！")};
        const TCHAR* Removed[]={TEXT("敵を火から離した！ 軍師、次の一手を。"),TEXT("二人で敵を烽火から離した。軍師、次の采配を。"),TEXT("火から敵を離したぜ！ 軍師、次はどうする！")};
        R.Line=Defeated ? Defeats[G]:Away ? Removed[G]:Joined[G];
        const TCHAR* Address[]={TEXT("兄者"),TEXT("雲長"),TEXT("翼徳")};
        const TCHAR* Replies[]={TEXT("%s、届いたな。次の守りも考えよう。"),TEXT("%s、確かに合いました。次の守りに備えましょう。"),TEXT("%s、見事だ！ 次の采配を待とうぜ！")};
        R.Reply=Last.After[S].Name+TEXT("「")+FString(Replies[S]).Replace(TEXT("%s"),Address[G])+TEXT("」");return R;
    }
}
