#include "TacticalMissionResponse.h"
namespace gunyumission
{
    gunyuresponse::FRead ReadCredit(const FBattleBeat* B)
    {
        gunyuresponse::FRead R;
        if(!B || B->Kind!=EBattleCue::Enemy || !B->MissionBefore.bBattle || !B->MissionAfter.bBattle) return R;
        const auto& A=B->MissionBefore;const auto& Z=B->MissionAfter;
        auto Living=[&](int Id) { return Id>=0 && B->Before.IsValidIndex(Id) && B->After.IsValidIndex(Id) && B->Before[Id].Id==Id && B->After[Id].Id==Id && !B->After[Id].bEnemy && !B->Before[Id].bEnemy && B->Before[Id].Troops>0 && B->After[Id].Troops>0; };
        const bool Road=B->RoadClearedBy>=0 && B->RoadClearedBy<3 && A.Convoy>=0 && Z.Convoy>A.Convoy && Living(5) && !B->Before[5].Position.Equals(B->After[5].Position);
        const bool Relief=B->ReliefIncoming>=0 && B->ReliefIncoming<3 && B->ReliefOutgoing>=0 && B->ReliefOutgoing<3 && B->ReliefIncoming!=B->ReliefOutgoing &&
            B->After.IsValidIndex(B->ReliefOutgoing) && B->After[B->ReliefOutgoing].Id==B->ReliefOutgoing && A.Held>=0 && Z.Held>A.Held;
        const int Id=Road ? B->RoadClearedBy:Relief ? B->ReliefIncoming:-1;
        if(Id<0 || !Living(Id)) return R;
        R.Actor=Id;R.Speaker=B->After[Id].Name;R.Sound=gunyuresponse::ESound::Support;R.Weight=1;
        if(Road)
        {
            const TCHAR* Lines[]={TEXT("道は空けた！ 軍師、皆を先へ進めてくれ。"),TEXT("車道は空いた。軍師、この道を使ってくれ。"),TEXT("道は空けたぜ！ 軍師、車を通してくれ！")};
            R.Line=Lines[Id];R.Caption=FString::Printf(TEXT("護送 %d/%d / %s隊が車道を開いた"),Z.Convoy,Z.Total,*R.Speaker);
        }
        else
        {
            const TCHAR* Held[]={TEXT("引き継いだ！ 軍師、この火は皆で守る。"),TEXT("引き継いだ。軍師、ここからは私が守る。"),TEXT("交代できたぜ！ 軍師、この火は俺が守る！")};
            const TCHAR* Recovered[]={TEXT("火を取り戻した！ 軍師、ここからつなごう。"),TEXT("守備を取り戻した。軍師、ここからは任せてくれ。"),TEXT("取り戻したぜ！ 軍師、今度は俺が守り切る！")};
            R.Line=B->bReliefRecovered ? Recovered[Id]:Held[Id];
            R.Caption=FString::Printf(TEXT("%s → %s / %s"),*B->After[B->ReliefOutgoing].Name,*R.Speaker,B->bReliefRecovered ? TEXT("烽火の守備を取り戻した"):TEXT("烽火の守備をつないだ"));
        }
        const int Preferred=!Road && Relief ? B->ReliefOutgoing:Id==0 ? 1:0;
        for(int Other:{Preferred,0,1,2}) if(Other!=Id && Living(Other)) { R.ReplyActor=Other;break; }
        if(R.ReplyActor>=0)
        {
            const TCHAR* Thanks[]={TEXT("%s、君がつないでくれた。皆、続こう！"),TEXT("%s、見事だ。その役目、確かに届いた。"),TEXT("%s、助かった！ こっちも任せとけ！")};
            const TCHAR* Address[]={TEXT("兄者"),TEXT("雲長"),TEXT("翼徳")};
            const FString Line=R.ReplyActor==1 && Id==0 ? FString(TEXT("兄者、後ろは私にお任せください。")):FString(Thanks[R.ReplyActor]).Replace(TEXT("%s"),Address[Id]);
            R.Reply=B->After[R.ReplyActor].Name+TEXT("「")+Line+TEXT("」");
        }
        return R;
    }
}
