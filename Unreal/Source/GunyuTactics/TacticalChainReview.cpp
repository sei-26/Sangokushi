#include "TacticalChainReview.h"
#include "TacticalState.h"
namespace gunyureview
{
    namespace
    {
        FString Role(frontline::ChainTactic Tactic)
        {
            using T=frontline::ChainTactic;
            switch(Tactic)
            {
            case T::Lure:return TEXT("敵を動かす役");case T::Ambush:return TEXT("伏兵で乱す役");
            case T::Breakthrough:return TEXT("突破する役");case T::Feint:return TEXT("隙を作る役");
            case T::Disrupt:return TEXT("構えを崩す役");case T::Oath:return TEXT("仲間を支える役");
            case T::AzureDragon:return TEXT("主攻を担う役");case T::SerpentRoar:return TEXT("先に陣を割る役");
            default:return FString();
            }
        }
        FString Basis(frontline::ChainTactic Tactic)
        {
            using T=frontline::ChainTactic;
            switch(Tactic)
            {
            case T::Lure:return TEXT("敵が追撃し、位置が動いた。次の担当との隣接を作戦に組み込もう。");
            case T::Ambush:return TEXT("森林での伏兵準備と、対象への隣接が成立した。");
            case T::Breakthrough:return TEXT("敵の混乱、または士気60以下と、押し出す先の空きから成立した。");
            case T::Feint:return TEXT("敵に隙を作った。この指揮の崩しや青龍偃月につなげられる。");
            case T::Disrupt:return TEXT("敵の隙から成立。固守を解除し、混乱へつなげた。");
            case T::AzureDragon:return TEXT("敵の隙・混乱、または士気60以下から成立。押し出す空きがなくても斬り込める。");
            case T::SerpentRoar:return TEXT("事前の隙を必要とせず、敵の固守を崩して混乱へつなげた。");
            case T::Oath:return TEXT("周囲の味方を支援。士気回復は上限まで、鼓舞は次の攻撃につながる。");
            default:return FString();
            }
        }
        bool State(const frontline::ChainEvent& E,bool bPressure)
        { for(const auto& C:E.changes) if(C.army==E.enemy) return bPressure ? C.pressured:C.disordered;return false; }
    }
    FChainReview Build(const frontline::ChainResult& Result,const TArray<FString>& Names)
    {
        FChainReview R;if(Result.prediction || Result.events.empty()) return R;
        using T=frontline::ChainTactic;
        for(int I=0;I<FMath::Min(3,int(Result.events.size()));++I)
        {
            const auto& E=Result.events[I];FChainReviewRow Row;
            Row.Actor=E.army;Row.bSuccess=E.success;Row.Damage=E.success ? FMath::Max(0,E.damage):0;
            Row.Officer=Names.IsValidIndex(E.army) && !Names[E.army].IsEmpty() ? Names[E.army]:TEXT("担当武将");
            Row.Tactic=JourneyText(frontline::TacticChain::Name(E.tactic));Row.Role=Role(E.tactic);
            Row.Subject=E.tactic==T::Oath ? TEXT("周囲の味方") : Names.IsValidIndex(E.enemy) && !Names[E.enemy].IsEmpty() ? Names[E.enemy]:TEXT("対象の敵");
            Row.Reason=E.tactic==T::Oath && E.success ? TEXT("士気上限の味方にも鼓舞は付く。実際に変化した味方を記録している。"):JourneyText(E.reason);
            Row.Link=E.success ? Basis(E.tactic):TEXT("ここで停止。残った行動権と敵の現在の状態を確認し、次の命令を選び直そう。");
            if(!E.success) { Row.Outcome=TEXT("この手で停止");R.bStopped=true; }
            else
            {
                ++R.Completed;R.Damage+=Row.Damage;
                Row.Outcome=Row.Damage>0 ? FString::Printf(TEXT("敵兵 −%d"),Row.Damage):TEXT("成立");
                if(E.tactic==T::Lure && E.from>=0 && E.to>=0 && E.from!=E.to) Row.Outcome=TEXT("敵を移動させた");
                if(E.tactic==T::Oath)
                {
                    int Count=0;for(const auto& C:E.changes) if(C.army>=0 && C.army<3 && C.army!=E.army) ++Count;
                    Row.Outcome=FString::Printf(TEXT("味方の状態変化 %d隊"),Count);
                }
                for(int P=I-1;P>=0;--P)
                {
                    const auto& Earlier=Result.events[P];if(!Earlier.success || Earlier.enemy!=E.enemy || E.enemy<0) continue;
                    if(E.tactic==T::Ambush && Earlier.tactic==T::Lure && Earlier.to==E.from && Earlier.from!=Earlier.to)
                        Row.Link=FString::Printf(TEXT("第%d手で動いた同じ敵へ、伏兵を重ねた。"),P+1);
                    if(E.tactic==T::Breakthrough && State(Earlier,false))
                        Row.Link=FString::Printf(TEXT("第%d手の混乱に続いて突破。空いた場所へ進んだ。"),P+1);
                    if(E.tactic==T::AzureDragon && State(Earlier,false))
                        Row.Link=FString::Printf(TEXT("第%d手で混乱した敵へ、主攻を重ねた。"),P+1);
                    if((E.tactic==T::Disrupt || E.tactic==T::AzureDragon) && State(Earlier,true))
                        Row.Link=FString::Printf(TEXT("第%d手で隙を作った相手へ、さらに攻撃を重ねた。"),P+1);
                    break;
                }
                if(Row.Damage>0)
                    for(int P=0;P<I;++P)
                    {
                        const auto& Earlier=Result.events[P];if(!Earlier.success || Earlier.tactic!=T::Oath) continue;
                        bool Inspired=false,Consumed=false;
                        for(const auto& C:Earlier.changes) if(C.army==E.army && C.inspired) Inspired=true;
                        for(const auto& C:E.changes) if(C.army==E.army && !C.inspired) Consumed=true;
                        if(Inspired && Consumed) { Row.Link+=FString::Printf(TEXT(" 第%d手の鼓舞も攻撃に活用した。"),P+1);break; }
                    }
            }
            R.Rows.Add(MoveTemp(Row));if(!E.success) break;
        }
        R.bAvailable=!R.Rows.IsEmpty();
        R.Summary=FString::Printf(TEXT("%d手成立  /  敵兵の実損耗 %d%s"),R.Completed,R.Damage,R.bStopped ? TEXT("  /  途中停止"):TEXT(""));
        R.Lesson=R.bStopped ? TEXT("停止後の手に戦果は付かない。理由を確かめ、残った行動権で立て直そう。") : TEXT("次は戦場の目標と残った行動権を確認し、連鎖の成果を任務へつなげよう。");
        return R;
    }
}
