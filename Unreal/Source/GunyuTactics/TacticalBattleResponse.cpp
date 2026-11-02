#include "TacticalBattleResponse.h"
#include "TacticalCinematicStyle.h"
namespace gunyuresponse
{
    FRead Read(const FBattleBeat* B,const TArray<FVisualUnit>& U,float P,int Next)
    {
        FRead R;
        if(!B || !FMath::IsFinite(P) || P<.48f || P>=1 || !gunyucinema::IsTactic(B->Kind)) return R;
        auto Known=[](const TArray<FVisualUnit>& Units,int Id) { return Units.IsValidIndex(Id) && Units[Id].Id==Id; };
        const int Id=B->Actor;
        if(Id<0 || Id>2 || !Known(B->Before,Id) || !Known(U,Id) || U[Id].bEnemy || B->Before[Id].Troops<=0) return R;
        if(B->Kind!=EBattleCue::Stop && (!Known(B->After,Id) || B->After[Id].Troops<=0)) return R;
        R.Actor=Id;R.Speaker=U[Id].Name;
        bool Meaningful=false;
        auto Say=[&](const TCHAR* A,const TCHAR* C,const TCHAR* D) { const TCHAR* Lines[]={A,C,D};R.Line=Lines[Id]; };
        if(B->Kind==EBattleCue::Stop)
        {
            R.Caption=TEXT("策を立て直そう");R.Sound=ESound::Stop;
            Say(TEXT("まだ終わりじゃない。次を頼む！"),TEXT("届かぬか。軍師、次の手を。"),TEXT("焦った！ 軍師、立て直そうぜ！"));
        }
        else if(B->Kind==EBattleCue::Rally)
        {
            for(int I=0;I<B->Before.Num();++I)
                if(Known(B->After,I) && Known(U,I) && B->Before[I].Id==I && !B->Before[I].bEnemy && B->Before[I].Troops>0 && gunyucinema::SupportChanged(B->Before[I],B->After[I])) Meaningful=true;
            R.Caption=Meaningful ? TEXT("仲間へ届いた鼓舞"):TEXT("士気を保って進もう");
            R.Sound=Meaningful ? ESound::Support:ESound::None;
            if(Meaningful) Say(TEXT("皆が顔を上げた。君のおかげだ！"),TEXT("声は届いた。皆、顔を上げよ！"),TEXT("おう！ まだまだ行けるぜ！"));
            else Say(TEXT("この士気を保とう。皆と共に！"),TEXT("今の勢いを失うな。皆、続け。"),TEXT("元気なら負けねえ！ 行くぞ！"));
        }
        else
        {
            const int T=B->Target;
            if(!Known(B->Before,T) || !Known(B->After,T) || !Known(U,T) || !B->Before[T].bEnemy || B->Before[T].Troops<=0) return {};
            const auto& A=B->Before[T];const auto& Z=B->After[T];
            const int Loss=FMath::Max(0,A.Troops-Z.Troops);
            const bool Moved=!A.Position.Equals(Z.Position),Disordered=!A.bDisordered && Z.bDisordered;
            const bool Opened=A.bGuarded && !Z.bGuarded,Pressured=!A.bPressured && Z.bPressured;
            Meaningful=Loss>0 || Moved || Disordered || Opened || Pressured || Z.Morale<A.Morale;
            if(!Meaningful)
            {
                R.Caption=TEXT("一手では崩せなかった");
                Say(TEXT("まだ踏ん張るか。次の策を！"),TEXT("守りは厚い。別の手で崩そう。"),TEXT("硬えな！ なら、攻め方を変える！"));
            }
            else if(B->Kind==EBattleCue::Lure && Moved)
            {
                R.Caption=TEXT("敵を誘い出した");
                Say(TEXT("追ってきた！ 軍師、今だ！"),TEXT("敵が動いた。軍師、合図を！"),TEXT("よし、釣れた！ あとは頼むぜ！"));
            }
            else if(B->Kind==EBattleCue::Ambush && B->Before[Id].bConcealed && Loss>0)
            {
                R.Caption=TEXT("仕込んだ伏兵が成功");
                Say(TEXT("待った甲斐があった。続こう！"),TEXT("軍師、君の仕込みが決まったな！"),TEXT("隠れた甲斐があったぜ、軍師！"));
            }
            else if(Opened || Disordered || Pressured)
            {
                R.Caption=Opened ? TEXT("敵の固守を崩した"):Disordered ? TEXT("敵陣が混乱した"):TEXT("敵に隙を作った");
                Say(TEXT("守りが乱れた。君の次の一手を！"),TEXT("敵陣が崩れた。この機を逃すな！"),TEXT("崩れたぞ！ 軍師、畳みかけようぜ！"));
            }
            else if(Moved)
            {
                R.Caption=TEXT("敵を押し返した");
                Say(TEXT("道が開いた。皆、続こう！"),TEXT("押し返した。軍師、道を使え！"),TEXT("どいたな！ 軍師、この道だ！"));
            }
            else
            {
                R.Caption=TEXT("武将の一撃が届いた");
                Say(TEXT("届いた！ この機をつなごう！"),TEXT("一撃は届いた。軍師、次を！"),TEXT("どうだ！ 軍師、まだ行けるぜ！"));
            }
            if(Meaningful) R.Sound=B->Combo>=3 ? ESound::Combo3:B->Combo==2 ? ESound::Combo2:ESound::Connect;
        }
        if(Meaningful && B->Combo>=2) R.Caption=FString::Printf(TEXT("%d連鎖成立 / "),B->Combo)+R.Caption;
        R.Weight=FMath::SmoothStep(.48f,.55f,P)*(1-FMath::SmoothStep(.92f,.995f,P));
        // A queued ally acknowledges the CURRENT result; their own outcome is unknown.
        if(Meaningful && P>.81f && Next>=0 && Next<3 && Next!=Id && Known(U,Next) && !U[Next].bEnemy && U[Next].Troops>0)
        {
            const TCHAR* Replies[]={TEXT("この機、受け取った！"),TEXT("うむ、次は私が！"),TEXT("次は俺だ！")};
            R.ReplyActor=Next;R.Reply=U[Next].Name+TEXT("「")+Replies[Next]+TEXT("」");
        }
        return R;
    }
    const TCHAR* SoundPath(ESound S)
    {
        switch(S)
        {
        case ESound::Connect:return TEXT("/Game/Audio/S_TacticConnectV1.S_TacticConnectV1");
        case ESound::Combo2:return TEXT("/Game/Audio/S_TacticCombo2V1.S_TacticCombo2V1");
        case ESound::Combo3:return TEXT("/Game/Audio/S_TacticCombo3V1.S_TacticCombo3V1");
        case ESound::Support:return TEXT("/Game/Audio/S_TacticSupportV1.S_TacticSupportV1");
        case ESound::Stop:return TEXT("/Game/Audio/S_TacticStopV1.S_TacticStopV1");
        default:return nullptr;
        }
    }
}
