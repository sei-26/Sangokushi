#include "TacticalMissionResponse.h"
namespace gunyumission
{
    gunyuresponse::FRead Read(const FBattleBeat* B)
    {
        gunyuresponse::FRead R;
        if(!B || B->Kind!=EBattleCue::Enemy || !B->MissionBefore.bBattle || !B->MissionAfter.bBattle) return R;
        const auto& A=B->MissionBefore;const auto& Z=B->MissionAfter;
        const auto Credit=ReadCredit(B);if(!Credit.Line.IsEmpty()) return Credit;
        const auto Defense=ReadDefense(B);if(!Defense.Line.IsEmpty()) return Defense;
        auto Living=[&](int Id)
        {
            return B->Before.IsValidIndex(Id) && B->After.IsValidIndex(Id) && B->Before[Id].Id==Id && B->After[Id].Id==Id &&
                !B->Before[Id].bEnemy && !B->After[Id].bEnemy && B->Before[Id].Troops>0 && B->After[Id].Troops>0;
        };
        int Event=-1,Preferred=0;
        if(A.Convoy>=0 && Z.Convoy>A.Convoy && Z.Total>0 && A.Total==Z.Total && Living(5) &&
           !B->Before[5].Position.Equals(B->After[5].Position))
        {
            if(A.Convoy==0) Event=0;
            else if(A.Convoy*2<A.Total && Z.Convoy*2>=Z.Total) Event=1;
        }
        else if(A.Held>=0 && Z.Held>=0 && Z.Required>0 && A.Required==Z.Required)
        {
            Preferred=1;
            if(A.Held==0 && Z.Held>0) Event=2;
            else if(A.Held>0 && Z.Held==0) Event=3;
        }
        if(Event<0) return R;
        for(int Id:{Preferred,0,1,2}) if(Living(Id)) { R.Actor=Id;break; }
        if(R.Actor<0) return {};
        R.Speaker=B->After[R.Actor].Name;
        const TCHAR* Lines[4][3]={
            {TEXT("車が動いた！ 軍師、その道をつないでくれ！"),TEXT("車が進んだ。軍師、この道を守ろう。"),TEXT("動いたぞ！ 軍師、俺が道を開ける！")},
            {TEXT("ここまで来た。皆、最後まで一緒に行こう。"),TEXT("道半ばだ。軍師、次の一手を頼む。"),TEXT("半分だ！ ここからも俺たちの出番だな！")},
            {TEXT("二つの火がつながった。皆、ここを守ろう！"),TEXT("両翼が応えた。軍師、ここから守り抜く。"),TEXT("両方ついたぞ！ 軍師、俺はここを守る！")},
            {TEXT("火が途切れた。皆、もう一度つなごう。"),TEXT("火が途切れた。軍師、二つの守りを取り戻そう。"),TEXT("途切れちまった！ 軍師、守りを立て直そうぜ！")}};
        R.Line=Lines[Event][R.Actor];
        if(Event<=1) R.Caption=FString::Printf(TEXT("%s %d/%d / %s"),Z.bRescue ? TEXT("季安の護送"):TEXT("糧車の護送"),Z.Convoy,Z.Total,Event==0 ? TEXT("車が動き始めた"):TEXT("道半ばまで来た"));
        else R.Caption=Event==2 ? FString::Printf(TEXT("烽火 %d/%d / 両翼を維持"),Z.Held,Z.Required):TEXT("烽火の守りが途切れた");
        R.Sound=Event==3 ? gunyuresponse::ESound::None:gunyuresponse::ESound::Support;
        for(int Id:{2,0,1}) if(Id!=R.Actor && Living(Id)) { R.ReplyActor=Id;break; }
        if(R.ReplyActor>=0)
        {
            const TCHAR* Replies[4][3]={
                {TEXT("皆の道だ。最後まで守ろう！"),TEXT("前路は私が見る。皆、続け。"),TEXT("おう！ 車の連中、ついてこい！")},
                {TEXT("その先も、君と共に。"),TEXT("残る道も、気を抜くな。"),TEXT("まだまだ行けるぜ！")},
                {TEXT("皆の火を、消させはしない。"),TEXT("守備を保て。軍師の合図を待つ。"),TEXT("こっちも任せとけ！")},
                {TEXT("次の策を聞こう。皆で立て直す。"),TEXT("守る場所を、もう一度そろえよう。"),TEXT("分かった。今度は合わせる！")}};
            R.Reply=B->After[R.ReplyActor].Name+TEXT("「")+Replies[Event][R.ReplyActor]+TEXT("」");
        }
        R.Weight=1;return R;
    }
}
