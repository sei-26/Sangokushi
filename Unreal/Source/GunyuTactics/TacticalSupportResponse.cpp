#include "TacticalMissionResponse.h"
#include "StrategistJourney.hpp"
namespace gunyumission
{
    namespace
    {
        int Tile(const FVector& P)
        {
            if(!FMath::IsFinite(P.X) || !FMath::IsFinite(P.Y) || FMath::Abs(P.X)>100000 || FMath::Abs(P.Y)>100000) return -1;
            const auto Cell=hexgrid::Pick(P.X/200.,P.Y/200.);
            if(Cell.first<0 || Cell.first>=frontline::StrategistJourney::Columns || Cell.second<0 || Cell.second>=frontline::StrategistJourney::Rows) return -1;
            return frontline::Campaign::At(Cell.first,Cell.second);
        }
        int Distance(int A,int B)
        { return hexgrid::Distance(A%frontline::Width,A/frontline::Width,B%frontline::Width,B/frontline::Width); }
    }
    gunyuresponse::FRead ReadSupport(const FBattleBeat* B)
    {
        gunyuresponse::FRead R;
        const bool Offensive=B && (B->Kind==EBattleCue::Breakthrough || B->Kind==EBattleCue::Ambush || B->Kind==EBattleCue::Feint || B->Kind==EBattleCue::Disrupt);
        if(!B || !B->MissionBefore.bBattle || !B->MissionAfter.bBattle || !frontline::StrategistJourney::OnMap(B->SupportGoal) ||
            (B->Kind!=EBattleCue::Lure && !Offensive)) return R;
        auto Known=[&](int Id)
        { return B->Before.IsValidIndex(Id) && B->After.IsValidIndex(Id) && B->Before[Id].Id==Id && B->After[Id].Id==Id; };
        auto LivingAlly=[&](int Id)
        { return Id>=0 && Id<3 && Known(Id) && !B->Before[Id].bEnemy && !B->After[Id].bEnemy && B->Before[Id].Troops>0 && B->After[Id].Troops>0; };
        const int A=B->Actor,G=B->SupportGuard,T=B->Target;
        if(A==G || !LivingAlly(A) || !LivingAlly(G) || T<3 || !Known(T) || !B->Before[T].bEnemy || !B->After[T].bEnemy || B->Before[T].Troops<=0 ||
            Tile(B->Before[G].Position)!=B->SupportGoal || Tile(B->After[G].Position)!=B->SupportGoal) return R;
        const int From=Tile(B->Before[T].Position),To=Tile(B->After[T].Position);
        if(From<0 || Distance(From,B->SupportGoal)>1) return R;
        const bool Defeated=B->After[T].Troops<=0 && Offensive;
        if(!Defeated && B->Kind!=EBattleCue::Lure && B->Kind!=EBattleCue::Breakthrough) return R;
        if(!Defeated && (B->After[T].Troops<=0 || To<0 || Distance(To,B->SupportGoal)<=1)) return R;
        const int ActorFrom=Tile(B->Before[A].Position),ActorTo=Tile(B->After[A].Position);
        if(ActorFrom<0 || ActorTo<0 || Distance(ActorFrom,From)!=1) return R;
        if(!Defeated && (B->Kind==EBattleCue::Lure ? (To!=ActorFrom || ActorTo==ActorFrom):ActorTo!=From)) return R;
        R.Actor=A;R.ReplyActor=G;R.Speaker=B->After[A].Name;R.Weight=1;R.Sound=gunyuresponse::ESound::Support;
        const TCHAR* Side=B->SupportNorth ? TEXT("北"):TEXT("南");
        R.Caption=FString(Side)+TEXT("の烽火 / ")+(Defeated ? TEXT("敵を撃破"):B->Kind==EBattleCue::Lure ? TEXT("敵を引き離した"):TEXT("敵を押し返した"));
        const TCHAR* Pulled[]={TEXT("追ってきた！ 軍師、火から敵を離したぞ。"),TEXT("敵は私を追ってきた。軍師、火から引き離した。"),TEXT("こっちへ来たな！ 軍師、火から引き離したぜ！")};
        const TCHAR* Pushed[]={TEXT("押し返した！ 軍師、今の一手が届いた。"),TEXT("烽火から押し返した。軍師、次の采配を。"),TEXT("押し返してやった！ 軍師、次はどうする！")};
        const TCHAR* Defeats[]={TEXT("この敵は倒した！ 軍師、次はどこへ？"),TEXT("烽火の敵は討った。軍師、次の一手を。"),TEXT("よし、一隊仕留めた！ 軍師、次を頼むぜ！")};
        R.Line=Defeated ? Defeats[A]:B->Kind==EBattleCue::Lure ? Pulled[A]:Pushed[A];
        const TCHAR* Address[]={TEXT("兄者"),TEXT("雲長"),TEXT("翼徳")};
        const TCHAR* Thanks[]={TEXT("%s、助かった。ここは私が引き受ける！"),TEXT("%s、見事だ。この火は私が受け持つ。"),TEXT("%s、助かった！ こっちは俺に任せとけ！")};
        const FString Reply=G==1 && A==0 ? FString(TEXT("兄者、こちらは私が引き受けます。")):FString(Thanks[G]).Replace(TEXT("%s"),Address[A]);
        R.Reply=B->After[G].Name+TEXT("「")+Reply+TEXT("」");return R;
    }
}
