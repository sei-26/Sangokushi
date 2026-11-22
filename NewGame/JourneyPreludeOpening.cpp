#include "JourneyPreludeOpening.hpp"
#include "StrategistJourney.hpp"
#include <memory>
namespace frontline
{
    bool CanChoosePreludeOpening(const StrategistJourney& j)
    { return j.campaignFlow && j.mission==0 && j.stage==JourneyStage::Preparation && !j.IsRescue(); }
    const PreludeOpeningPlan& PreludeOpening(int choice)
    {
        static const PreludeOpeningPlan ambush{U"関羽の伏兵で、先鋒を崩す",
            U"関羽は森で待つ。張飛は追い打ち、劉備は自由に援護。",
            U"敵を森の間合いへ。伏兵の配置だけでは攻撃しない。",
            {JourneyAssignment{},JourneyAssignment{JourneyDuty::Ambush,Campaign::At(8,9)},JourneyAssignment{}}};
        static const PreludeOpeningPlan exit{U"劉備を出口へ、二人で援護",
            U"劉備は出口へ行軍・固守。関羽と張飛は自分で援護。",
            U"敵を引き離す援護が必要。出口に敵が隣接している間は確保できない。",
            {JourneyAssignment{JourneyDuty::Hold,Campaign::At(12,10)},JourneyAssignment{},JourneyAssignment{}}};
        return choice==1 ? exit:ambush;
    }
    bool MatchesPreludeOpening(const StrategistJourney& j,int choice)
    {
        if(!CanChoosePreludeOpening(j) || choice<0 || choice>1) return false;
        const auto& a=PreludeOpening(choice).assignments;
        for(int i=0;i<3;++i) if(j.assignments[i].duty!=a[i].duty || j.assignments[i].tile!=a[i].tile) return false;
        return true;
    }
    bool AdoptPreludeOpening(StrategistJourney& j,int choice)
    {
        if(!CanChoosePreludeOpening(j) || choice<0 || choice>1 || !j.CanPrepare(choice)) return false;
        auto candidate=std::make_unique<StrategistJourney>(j);candidate->assignments={};
        const auto& plan=PreludeOpening(choice);
        for(int i=0;i<3;++i) if(!candidate->AssignDuty(i,plan.assignments[i].tile,plan.assignments[i].duty)) return false;
        j.assignments=candidate->assignments;
        j.message=std::u32string(U"初陣の策を採用：")+plan.name+U"。配置・物資は出陣前に変更できる。兵と糧はまだ使っていない。";
        return true;
    }
    CouncilExchange PreludeOpeningCouncil(const StrategistJourney& j,int accepted)
    {
        CouncilExchange e;if(!CanChoosePreludeOpening(j)) return e;
        if(accepted<0)
        {
            e.subject=U"初陣の一策 ／ あの道に、明日をつなぐ";
            e.beats={{{2,U"先鋒は目の前だ！ 俺がぶち抜く。……おい軍師、合図はお前が出せよ！",U"街道の向こうで黄色い旗が迫る。季安の車から、子どもが顔をのぞかせた。",4},{U"頼もしいな",U"待て、翼徳",U""}},
                {{1,U"先鋒を崩すか、出口を先に取るか。戦いを終える道は二つある。私を林に置くなら、その先の一手も任せる。",U"関羽は林に、劉備は出口に駒を置いた。",1},{}},
                {{3,U"三人とも、力を貸してくれ。敵を全部倒す必要はない。民が通れる道を、俺たちでつくる！",U"二つの策から選べる。採用後も作戦図で直せる。出陣するまでは兵と糧を使わない。",5},{U"共に行こう",U"合図を待とう",U"任せろ！"}}};
        }
        else if(accepted<=1)
        {
            e.subject=PreludeOpening(accepted).name;
            if(accepted==0) e.beats={{{3,U"関羽、林で待ってくれ。張飛はその一撃につなげる。劉備殿、二人が孤立しないよう力を貸してほしい。",U"関羽に伏兵の担当を置いた。他の二人は自由指揮。",5},{}},
                {{2,U"待つぜ。だけど、呼ぶ時はでっけえ声でな！ 俺まで林に隠れたら、木が足りねえ！",U"関羽が息を漏らす。主人公は、二つの駒をつなぐ線を引いた。",7},{U"では、出よう",U"木より先に口を隠せ",U""}}};
            else e.beats={{{3,U"劉備殿を出口へ。二人は敵を引き受けてくれ。倒し切るより、先に道を取る！ 立て直す糧も積んでおこう。",U"劉備に出口の守備担当を置いた。関羽と張飛は自由指揮。",5},{}},
                {{0,U"引き受けた。途中で敵が来たら、君を呼ぶよ。道の先で、皆を待っている。",U"行軍は出陣後に進める。接敵した時の援護は、軍師の采配にかかっている。",1},{U"",U"援護は任せよ",U"出口で会おうぜ！"}}};
        }
        return e;
    }
}
