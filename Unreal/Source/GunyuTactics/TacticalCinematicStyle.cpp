#include "TacticalCinematicStyle.h"
namespace gunyucinema
{
    bool IsTactic(EBattleCue K)
    { return K==EBattleCue::Strike || K==EBattleCue::Lure || K==EBattleCue::Ambush || K==EBattleCue::Breakthrough || K==EBattleCue::Feint || K==EBattleCue::Disrupt || K==EBattleCue::Rally || K==EBattleCue::AzureDragon || K==EBattleCue::SerpentRoar || K==EBattleCue::Stop; }
    FString Name(EBattleCue K)
    {
        switch(K)
        {
        case EBattleCue::Strike:return TEXT("攻撃");case EBattleCue::Lure:return TEXT("誘導");
        case EBattleCue::Ambush:return TEXT("伏兵");case EBattleCue::Breakthrough:return TEXT("突破");
        case EBattleCue::Feint:return TEXT("牽制");case EBattleCue::Disrupt:return TEXT("崩し");
        case EBattleCue::Rally:return TEXT("義勇の誓い");case EBattleCue::AzureDragon:return TEXT("青龍偃月");
        case EBattleCue::SerpentRoar:return TEXT("燕人の大喝");case EBattleCue::Stop:return TEXT("連鎖停止");
        default:return FString();
        }
    }
    FString Voice(EBattleCue K)
    {
        switch(K)
        {
        case EBattleCue::Lure:return TEXT("退くと見せて、敵を動かす。今だ！");
        case EBattleCue::Ambush:return TEXT("機を逃すな。一斉にかかれ！");
        case EBattleCue::Breakthrough:return TEXT("道を開くぞ。この一撃に続け！");
        case EBattleCue::Feint:return TEXT("こちらを見ろ。守りに隙を作れ！");
        case EBattleCue::Disrupt:return TEXT("乱れを逃すな。敵の構えを崩せ！");
        case EBattleCue::Rally:return TEXT("仲間を信じよ。民の帰る道を守る！");
        case EBattleCue::AzureDragon:return TEXT("この一刀で、勝機を切り開く！");
        case EBattleCue::SerpentRoar:return TEXT("我が前を通すものか！");
        case EBattleCue::Stop:return TEXT("仕掛けが届かぬ。立て直すぞ！");
        default:return TEXT("全軍、かかれ！");
        }
    }
    FLinearColor Color(EBattleCue K)
    {
        if(K==EBattleCue::AzureDragon || K==EBattleCue::Ambush) return FLinearColor(.22f,.95f,.62f);
        if(K==EBattleCue::Lure || K==EBattleCue::Feint) return FLinearColor(.27f,.76f,1.f);
        if(K==EBattleCue::SerpentRoar || K==EBattleCue::Breakthrough) return FLinearColor(1.f,.41f,.16f);
        if(K==EBattleCue::Stop) return FLinearColor(.68f,.70f,.72f);
        return FLinearColor(1.f,.8f,.32f);
    }
}
