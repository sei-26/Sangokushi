#include "StrategistJourney.hpp"

namespace frontline
{
    bool StrategistJourney::CanChooseYangrenApproach() const
    {
        return stage==JourneyStage::Battle && HasYangrenStrategy() && turn==1 &&
            battleReview.grand.yangren.approach==YangrenApproach::None;
    }
    bool StrategistJourney::ChooseYangrenApproach(YangrenApproach choice)
    {
        if(!CanChooseYangrenApproach() || (choice!=YangrenApproach::Forest && choice!=YangrenApproach::Advance)) return false;
        auto& y=battleReview.grand.yangren;y.approach=choice;y.orderedOn=turn;++world.revision;
        Report(choice==YangrenApproach::Forest ? U"主人公「先に森で呂布を崩す。孫堅殿、南道で待ってください。合図が来たら、一緒に前へ！」":
            U"主人公「孫堅殿と足並みを合わせて押す！ 騎兵に追われても散るな。辻で本隊を迎え、反撃を受け止める！」");
        return true;
    }
    bool StrategistJourney::YangrenFirstChargeBroken() const
    {
        if(battleReview.yangren.counter!=YangrenCounter::None || !TacticChain::Living(world,3)) return true;
        const auto* seen=intelligence[0].Find(3);
        return seen && seen->current && seen->observed.morale<=35;
    }
    bool StrategistJourney::YangrenCounterRepelled() const
    { return HasYangrenStrategy() && battleReview.grand.yangren.repelledOn>0; }
    void StrategistJourney::CheckYangrenCounter()
    {
        if(!HasYangrenStrategy()) return;
        auto& y=battleReview.grand.yangren;auto& g=battleReview.grand;
        if(g.openedOn && g.ralliedOn && !y.counterOn)
        {
            y.counterOn=turn;++world.revision;
            Report(TacticChain::Living(world,8) ? U"呂布「合流しただけで勝ったつもりか！ 騎兵、南へ回れ！」本陣から親衛騎兵が出る。孫堅を孤立させず、反撃を退けよう。":
                U"孫堅「親衛の騎兵も先に崩していたか！ 軍師、反撃の隙を与えなかったな！」");
        }
        const auto* seen=intelligence[0].Find(8);
        if(y.counterOn && !y.repelledOn && (!TacticChain::Living(world,8) ||
            (seen && seen->current && seen->observed.morale<=25)))
        {
            y.repelledOn=turn;++world.revision;
            Report(U"呂布の親衛騎兵が退く。孫堅「よく踏ん張った！ 俺たちの旗は、まだ隣にあるぞ！」");
        }
    }
}
