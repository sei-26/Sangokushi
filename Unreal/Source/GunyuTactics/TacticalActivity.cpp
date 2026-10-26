#include "TacticalActivity.h"
#include "TacticalSoldierMotion.h"
namespace gunyuactivity
{
    namespace
    {
        bool Present(const TArray<FVisualUnit>& U,int I) { return I>=0 && U.IsValidIndex(I) && U[I].Id==I; }
        bool Attack(EBattleCue K)
        { return K==EBattleCue::Strike || K==EBattleCue::Ambush || K==EBattleCue::Breakthrough || K==EBattleCue::Feint || K==EBattleCue::Disrupt || K==EBattleCue::AzureDragon || K==EBattleCue::SerpentRoar; }
        FString Name(EBattleCue K)
        {
            switch(K)
            {
            case EBattleCue::Lure:return TEXT("誘導");case EBattleCue::Ambush:return TEXT("伏兵");
            case EBattleCue::Breakthrough:return TEXT("突破");case EBattleCue::Feint:return TEXT("牽制");
            case EBattleCue::Disrupt:return TEXT("崩し");case EBattleCue::AzureDragon:return TEXT("青龍偃月");
            case EBattleCue::SerpentRoar:return TEXT("咆哮");case EBattleCue::Rally:return TEXT("鼓舞");
            case EBattleCue::Guard:return TEXT("固守");case EBattleCue::Conceal:return TEXT("潜伏準備");
            case EBattleCue::Withdraw:return TEXT("撤収");default:return TEXT("攻撃");
            }
        }
        FString Rest(const FVisualUnit& U)
        {
            if(U.Troops<=0) return TEXT("戦闘不能");if(U.bDisordered) return TEXT("混乱");
            if(U.bPressured) return TEXT("隙あり");if(U.bConcealed) return TEXT("潜伏中");
            if(U.bGuarded) return TEXT("固守");if(U.bInspired) return TEXT("鼓舞");
            return U.Id==5 ? TEXT("護送待機"):TEXT("待機");
        }
    }
    FUnitActivity Read(int Id,const TArray<FVisualUnit>& U,const FBattleBeat* B,float P)
    {
        FUnitActivity R;if(!Present(U,Id)) return R;
        const auto& V=U[Id];R.Caption=Rest(V);R.Progress=FMath::Clamp(P,0.f,1.f);
        if(!B || !Present(B->Before,Id)) return R;
        const auto M=gunyumotion::Read(V,U,B,P);R.Direction=M.Facing;
        const bool PartnerVisible=Present(U,B->Target) && Present(B->Before,B->Target);
        R.bActor=Id==B->Actor;
        if(B->Kind==EBattleCue::Stop)
        { if(R.bActor) { R.bActive=R.bStopped=true;R.Caption=TEXT("連鎖停止");R.Color=FLinearColor(1,.4f,.18f); }return R; }
        if(R.bActor && Attack(B->Kind) && !PartnerVisible) return R;
        const bool Target=Id==B->Target && Present(U,B->Actor) && Present(B->Before,B->Actor) && (Attack(B->Kind) || B->Kind==EBattleCue::Lure);
        R.bActive=R.bActor || Target || V.Motion>.01f || (B->Kind==EBattleCue::Enemy && M.bAttack);
        if(!R.bActive || V.Troops<=0) return R;
        R.Color=R.bActor ? FLinearColor(1,.81f,.32f):FLinearColor(.27f,.86f,.94f);
        R.bImpact=P>=.48f && V.Loss>0;
        if(R.bActor)
        {
            if(PartnerVisible && (Attack(B->Kind) || B->Kind==EBattleCue::Lure)) R.Partner=B->Target;
            if(B->Kind==EBattleCue::March) R.Caption=P<.18f ? TEXT("進軍準備"):V.Motion>.01f ? TEXT("進軍中"):TEXT("到着・整列");
            else if(B->Kind==EBattleCue::Lure) R.Caption=P<.82f ? TEXT("誘導・後退"):TEXT("誘導・整列");
            else R.Caption=Name(B->Kind)+(P<.32f ? TEXT("・構え"):P<.48f ? TEXT("・発動"):P<.82f ? TEXT("・実行"):TEXT("・復帰"));
        }
        else if(Target && B->Kind==EBattleCue::Lure) { R.Caption=P<.38f ? TEXT("追撃準備"):P<.86f ? TEXT("追撃中"):TEXT("追撃・整列");R.Partner=B->Actor; }
        else if(R.bImpact)
        { R.Caption=V.bDisordered ? TEXT("被弾・混乱"):P<.82f ? TEXT("被弾"):TEXT("立て直し");R.Color=FLinearColor(1,.39f,.19f); }
        else if(V.Motion>.01f) R.Caption=Id==5 ? TEXT("護送進行"):TEXT("進軍中");
        else if(Target && P<.48f) R.Caption=TEXT("攻撃対象");
        else if(B->Kind==EBattleCue::Enemy && M.bAttack) R.Caption=P<.48f ? TEXT("交戦・構え"):P<.82f ? TEXT("交戦中"):TEXT("立て直し");
        else R.Caption=Rest(V);
        return R;
    }
}
