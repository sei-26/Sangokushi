#include "StrategistJourney.hpp"
#include "JourneyPlanExample.hpp"
#include <memory>
namespace frontline
{
    bool StrategistJourney::SelectDifficulty(JourneyDifficulty value)
    {
        if((stage!=JourneyStage::Preparation && stage!=JourneyStage::Encounter) ||
           (value!=JourneyDifficulty::Standard && value!=JourneyDifficulty::Story)) return false;
        difficulty=value;
        message=std::u32string(U"難易度：")+DifficultyName(value)+U"。兵力と期限は出陣時に適用する。";
        return true;
    }
    JourneyPlanExample PlanExample(const StrategistJourney& j)
    {
        JourneyPlanExample p;
        if(j.mission==2)
        {
            p.name=U"二つの烽火を分担して守る";
            p.purpose=U"劉備を北の烽火、張飛を南の烽火へ。関羽は北の森林で援護の準備をする。";
            p.assignments={JourneyAssignment{JourneyDuty::Hold,j.Definition().goal},
                JourneyAssignment{JourneyDuty::Ambush,j.Definition().positions[1]},
                JourneyAssignment{JourneyDuty::Hold,j.Definition().secondGoal}};
            p.manualTask=U"まず両拠点の配置を待つ。敵が隣接したら、関羽の戦法や他隊の単独戦法で守りを助ける。";
            p.caution=U"両方を同時に３指揮維持する。固守でも隣接敵が２隊になると維持できない。救援に出れば元の拠点が空く。";
        }
        else if(j.mission==1)
        {
            p.name=U"車道を空け、南橋を支える";
            p.purpose=U"劉備は南橋手前の陸地で守備。関羽は北側の森林へ。張飛は固定配置せず、橋の敵を崩す役として自分で動かす。";
            p.assignments={JourneyAssignment{JourneyDuty::Hold,Campaign::At(10,10)},
                JourneyAssignment{JourneyDuty::Ambush,Campaign::At(9,9)},JourneyAssignment{}};
            p.manualTask=U"張飛を南橋へ進め、見えた敵に単独の突破を使う。道が開いたら車道から退き、糧車を出口まで通す。";
            p.caution=U"見本は橋を自動で奪わない。川へは進入できず、敵の出方で進路を直す必要がある。積荷は1,200以上残す。";
        }
        else
        {
            p.name=j.IsRescue() ? U"退路の脇を支え、避難車を通す":U"森林を足場に、戦法をつなぐ";
            p.purpose=U"劉備は今の前線で守備。関羽は出陣位置の森で伏兵準備。張飛は固定せず、前へ出す時機を自分で選ぶ。";
            p.assignments={JourneyAssignment{JourneyDuty::Hold,j.Definition().positions[0]},
                JourneyAssignment{JourneyDuty::Ambush,j.Definition().positions[1]},JourneyAssignment{}};
            p.manualTask=j.IsRescue() ? U"張飛の単独戦法で前路の敵を崩し、味方も車道を空ける。敵将を倒しても、避難車が出口へ着くまで続く。":
                U"まず張飛の単独戦法を使ってよい。連鎖を狙う時は守備の指示をXで解除し、使いたい戦法と配置を決め直す。";
            p.caution=U"守備は攻撃の行動権を使う。敵の斥候や接近で伏兵が失敗することもある。固守中の隊を攻めに使う時は指示を変える。";
        }
        return p;
    }
    bool StrategistJourney::ApplyPlanExample()
    {
        if(stage!=JourneyStage::Preparation) return false;
        const auto plan=PlanExample(*this);
        auto candidate=std::make_unique<StrategistJourney>(*this);candidate->assignments={};
        for(int i=0;i<3;++i)
            if(!candidate->AssignDuty(i,plan.assignments[i].tile,plan.assignments[i].duty)) return false;
        assignments=candidate->assignments;
        message=U"見本を配置："+plan.name+U"。戦場の進行・戦法発動は、自分で指揮する。";
        return true;
    }
}
