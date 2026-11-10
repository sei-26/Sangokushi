#include "TacticalMissionResponse.h"

namespace gunyumission
{
    gunyuresponse::FRead ReadDuty(const FBattleBeat& Beat)
    {
        const auto& Before=Beat.MissionBefore;
        const auto& After=Beat.MissionAfter;
        const bool Cycle=Beat.Kind==EBattleCue::Enemy;
        if(!Before.bBattle || !After.bBattle || Before.CommandTurn<1 || After.CommandTurn!=Before.CommandTurn+(Cycle ? 1:0)) return {};
        if(!Cycle && Beat.Kind!=EBattleCue::Guard && Beat.Kind!=EBattleCue::Conceal) return {};
        const int NewDuties=After.CompletedDuties & After.PresentDuties & ~Before.CompletedDuties;
        const TCHAR* GuardLines[]={
            TEXT("任された場所は固めたよ。次の合図を待つ。"),
            TEXT("持ち場を固めた。軍師、次の手を。"),
            TEXT("ここは固めたぜ！ 軍師、次を頼む！")};
        const TCHAR* AmbushLine=TEXT("森に伏した。軍師、合図を待つ。");
        gunyuresponse::FRead Result;
        for(int Id=0;Id<3;++Id)
        {
            if(!(NewDuties&(1<<Id)) || (!Cycle && Beat.Actor!=Id) || !Beat.After.IsValidIndex(Id)) continue;
            const auto& Unit=Beat.After[Id];
            if(Unit.Id!=Id || Unit.bEnemy || Unit.Troops<=0) continue;
            const bool Ambush=(After.AmbushDuties&(1<<Id))!=0;
            if(!Cycle && Ambush!=(Beat.Kind==EBattleCue::Conceal)) continue;
            const TCHAR* Line=Ambush ? AmbushLine:GuardLines[Id];
            if(Result.Actor>=0)
            {
                Result.ReplyActor=Id;Result.Reply=Unit.Name+TEXT("「")+Line+TEXT("」");
                break;
            }
            Result.Actor=Id;Result.Speaker=Unit.Name;Result.Line=Line;
            Result.Caption=Ambush ? TEXT("軍議の担当 / 伏兵を準備"):TEXT("軍議の担当 / 固守を実行");
            Result.Weight=1;Result.Sound=gunyuresponse::ESound::Support;
        }
        return Result;
    }
}
