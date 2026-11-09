#include "TacticalMissionResponse.h"

namespace gunyumission
{
    gunyuresponse::FRead RetryDefenseVoices(const FBattleBeat& Beat, int Guard, int Supporter, bool bNorth)
    {
        const TCHAR* GuardLines[] = {
            TEXT("この火は守れた。援護が届いたよ。"),
            TEXT("この火は守った。援護、かたじけない。"),
            TEXT("火は守ったぜ！ 援護、ありがとな！")
        };
        const TCHAR* SupportLines[] = {
            TEXT("火は君が支えてくれた。軍師、私は援護を続けよう。"),
            TEXT("火の守りは任せた。軍師、私は援護を続ける。"),
            TEXT("そっちの火は任せたぞ！ 軍師、俺は援護で走るぜ！")
        };
        const TCHAR* Address[] = {TEXT("兄者"), TEXT("雲長"), TEXT("翼徳")};
        const TCHAR* SupportLine = Supporter == 1 && Guard == 0
            ? TEXT("守りを預かっていただいた。軍師、私は援護を続ける。")
            : SupportLines[Supporter];

        gunyuresponse::FRead Response;
        Response.Actor = Guard;
        Response.ReplyActor = Supporter;
        Response.Speaker = Beat.After[Guard].Name;
        Response.Caption = bNorth ? TEXT("北の烽火 / 援護が届いた") : TEXT("南の烽火 / 援護が届いた");
        Response.Line = Guard == 1 && Supporter == 0
            ? TEXT("兄者、火は守りました。かたじけない。")
            : GuardLines[Guard];
        Response.Reply = Beat.After[Supporter].Name + TEXT("「") + Address[Guard] + TEXT("、") + SupportLine + TEXT("」");
        Response.Weight = 1;
        Response.Sound = gunyuresponse::ESound::Support;
        return Response;
    }
}
