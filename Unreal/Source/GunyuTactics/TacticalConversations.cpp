#include "TacticalTrial.h"
#include "TacticalState.h"

// Original dialogue: relationships are earned by actual shared missions, not
// by repeatedly selecting a conversation. See STORY_STRATEGY_IMPROVEMENTS.md.
void ATacticalField::BuildOfficerConversation()
{
    using namespace frontline;
    const auto& J = State->Journey;
    auto& P = State->HUD.Journey;
    if (!P.bPreparation && !P.bDebrief) return;
    const int Who = FMath::Clamp(State->Interlocutor, 0, 2);
    const TCHAR* Names[] = {TEXT("劉備"), TEXT("関羽"), TEXT("張飛")};
    P.Speaker = Names[Who]; P.Portrait = Who;
    const TCHAR* Line = TEXT("");
    if (P.bPreparation)
    {
        if (J.mission == 0 && J.IsRescue())
        {
            const TCHAR* Lines[] = {
                TEXT("季安は、車を押す者まで兵に数えるなと言っていた。戦う者を前へ出すほど、車の後ろは薄くなる。送り届けると決めるなら、私も最後まで見る。"),
                TEXT("車は、敵が近くにいれば止まる。味方が道を塞いでも進めぬ。敵を崩した後こそ、前へ出る隊と、その場を守る隊を分けるのだ。"),
                TEXT("敵の旗が倒れたら、追いたくなるさ。だが、俺が戻る間まで車を守る奴が要る。俺を前へ出すなら、後ろのこともお前が決めてくれ。")};
            Line = Lines[Who];
        }
        else if (J.mission == 0)
        {
            const TCHAR* Lines[] = {
                TEXT("まず道を開こう。すべてを一度に背負う約束はしなくていい。ただ、何を守り切れなかったかは、あとで一緒に確かめたい。"),
                TEXT("戦の終わりは、敵の数だけでは決まらぬ。先鋒を崩すか、出口を押さえるか。退路ができたなら、それ以上の手柄に兵を使うべきではない。"),
                TEXT("俺が前へ出れば、敵もこっちを見る。その隙に出口を取る手もあるんだろ？ 斬り込むばかりが仕事じゃねえのは、分かってる。")};
            Line = Lines[Who];
        }
        else if (!J.campaignFlow && J.mission == 1 && J.history[0].rescued)
        {
            const TCHAR* Lines[] = {
                TEXT("季安が、今度は糧を運ぶ役を引き受けてくれた。あの時送り届けた車が、次に皆を食わせる車になる。今度も、帰るところまで任せたい。"),
                TEXT("季安は、あの時どこで車を止めたか覚えていた。前の戦で見た道を、今度は補給の道にする。護衛にも、あの時の兵を付けよう。"),
                TEXT("季安がまた俺の後ろを走るんだとよ。今度は荷が重いから急がせるなってさ。……先に敵をどけて、道を空けといてやるか。")};
            Line = Lines[Who];
        }
        else
        {
            const bool HeavyLoss = J.history[J.mission - 1].casualties > 900;
            const TCHAR* Lines[] = {
                HeavyLoss ? TEXT("前の戦で減った兵は、ここで数を戻すだけでは帰ってこない。今回は、誰を残し、いつ戦いを終えるかまで決めてから出よう。") : TEXT("前の任務で残した兵と糧が、今ここにある。君が残してくれた選択肢を、次の戦いでも使いたい。"),
                TEXT("守る仕事を、攻め残った者の仕事にするな。先に担当を決めれば、前へ出る者も背中を気にせず働ける。私をどちらへ置くか、聞かせてくれ。"),
                TEXT("前に出る役は嫌いじゃねえ。だが、俺たちだけで片をつけようとするなよ。戻る道が残ってると思えるから、前で踏ん張れるんだ。")};
            Line = Lines[Who];
        }
    }
    else if (J.withdrawn)
    {
        const TCHAR* Lines[] = {
            TEXT("引くと決めたのだな。残った兵と糧は確かに戻った。守れなかった約束は消えないが、ここにいる者を次へ連れていく責任もある。一緒に、原因を見直そう。"),
            TEXT("勝てぬ形を続けなかったことまで、恥じるな。だが、いつ崩れたかは覚えておこう。道が塞がる前か、戦法が止まった時か。次はそこを変える。"),
            TEXT("悔しくねえわけじゃねえよ。それでも戻れって言われたから戻った。次は、前へ出た俺たちを、何のために呼び戻すかも聞かせてくれ。")};
        Line = Lines[Who];
    }
    else if (J.IsRescue() && J.won)
    {
        const TCHAR* Lines[] = {
            TEXT("季安が車から降りて、君を探していた。敵の旗を倒した者ではなく、最後まで道を残した者に礼を言いたいそうだ。次の軍議にも、席を空けておこう。"),
            TEXT("敵が崩れた後も、車の後ろに兵を残したな。策は、最後の者が通るところまで続いていた。その引き際なら、次も兵を預けられる。"),
            TEXT("季安の車、ちゃんと出口を抜けたな。俺が暴れてる間に、あんな所まで行ってやがった。あいつに言っとけ。次も道を空けてやるってな。")};
        Line = Lines[Who];
    }
    else if (!J.won)
    {
        const TCHAR* Lines[] = {
            TEXT("結果だけ見て君に責を負わせるつもりはない。だが、送り届けるはずだった者の顔は忘れたくない。どこまで戻れば、別の道を選べるか考えよう。"),
            TEXT("崩れたのが配置か、順序か、護衛の薄さか。一つずつ分けて見よう。次の策を立てるために話している。君を責めるためではない。"),
            TEXT("倒す相手ばかり見てたかもしれねえな。俺にも言ってくれよ。そこに残れ、追うな、って。次は一人で抱えんな。")};
        Line = Lines[Who];
    }
    else
    {
        const TCHAR* Lines[] = {
            TEXT("敵を追う手を止めて、任された道を残した。その道へ糧が通り、人が戻る。君の采配を、戦った兵だけの手柄にはしたくない。"),
            TEXT("前に出る者と、支える者の働きがつながった。次は同じ手が通るとは限らぬが、今日ともに動いたことは、兵も私も覚えている。"),
            TEXT("あそこでつながった時は、悪くなかったな。お前が呼ぶ場所に、俺たちがちゃんといた。次はどこへ走らせる気だ？")};
        Line = Lines[Who];
    }
    P.Dialogue = Line;
}
