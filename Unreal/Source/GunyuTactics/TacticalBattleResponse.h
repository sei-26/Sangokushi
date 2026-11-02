#pragma once
#include "TacticalPresentation.h"
namespace gunyuresponse
{
    enum class ESound { None, Connect, Combo2, Combo3, Support, Stop };
    struct FRead
    {
        int Actor=-1,ReplyActor=-1;
        FString Speaker,Caption,Line,Reply;
        float Weight=0;
        ESound Sound=ESound::None;
    };
    // Only this beat's observed snapshots and the already queued next officer.
    FRead Read(const FBattleBeat* Beat,const TArray<FVisualUnit>& Visible,float Progress,int NextActor=-1);
    const TCHAR* SoundPath(ESound Sound);
    bool SmokeTest();
}
