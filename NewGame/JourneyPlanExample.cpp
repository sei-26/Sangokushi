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
    void AdaptPlanExample(const StrategistJourney&,JourneyPlanExample&);
    void AdaptReliefPlanExample(const StrategistJourney&,JourneyPlanExample&);
    void AdaptContributionPlanExample(const StrategistJourney&,JourneyPlanExample&);
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
            p.caution=j.RequiredHoldTurns()==2 ? U"河港での連携が効き、両方を２指揮維持すれば包囲が完成する。敵２隊の隣接や救援で守りを空ける判断には注意。":U"両方を同時に３指揮維持する。固守でも隣接敵が２隊になると維持できない。救援に出れば元の拠点が空く。";
        }
        else if(j.HasSunJian())
        {
            p.name=U"林で呂布を止め、南道を開ける";
            p.purpose=U"関羽が林で騎兵を止め、劉備が辻を押さえる。張飛は南の孫堅を援護する。";
            p.assignments={JourneyAssignment{},JourneyAssignment{JourneyDuty::Ambush,j.Definition().positions[1]},JourneyAssignment{}};
            p.manualTask=U"一隊を陽人の辻に入れ、もう一隊で南の敵を退ける。孫堅が合流地点へ来れば勝利。";
            p.caution=U"呂布が平地で騎兵突撃を使う。森林に引き込むか、混乱させて発動を止めよう。";
        }
        else if(j.mission==CoalitionMission)
        {
            if(j.coalitionPlan==CoalitionPlan::Protect)
            {
                p.name=U"街道に二隊を残し、関羽が援護";
                p.purpose=U"劉備と張飛は北と南の烽火へ。関羽は中央の森から手薄な側へ向かう。";
                p.assignments={JourneyAssignment{JourneyDuty::Hold,j.Definition().goal},JourneyAssignment{},
                    JourneyAssignment{JourneyDuty::Hold,j.Definition().secondGoal}};
                p.manualTask=j.legacyBattlefield ? U"関羽で渡河してきた敵を足止めし、二つの烽火を同時に２指揮維持する。":
                    U"関羽で東の林から来る敵を足止めし、二つの烽火を同時に２指揮維持する。";
                p.caution=U"守備役が戦法を使うと、その指揮の固守を使えなくなる。";
            }
            else
            {
                p.name=U"北橋へ突破役を通し、南で反撃を受ける";
                p.purpose=U"関羽は北橋手前の林で伏兵。劉備と張飛は前哨を取る隊と南橋へ向かう隊に分ける。";
                p.assignments[1]={JourneyDuty::Ambush,j.Definition().positions[1]};
                p.manualTask=U"敵の前衛を前哨から引き離し、一隊を占領地点に入れる。";
                p.caution=U"橋の上に味方が固まると後続が渡れない。必要なら命令を出し直す。";
            }
        }
        else if(j.mission==YellowTurbanMission)
        {
            p.name=U"北の林で引きつけ、南から前陣へ";
            p.purpose=U"劉備は林の南端を固め、関羽は北の林で伏兵。張飛は南の平地から前陣へ。";
            p.assignments={JourneyAssignment{JourneyDuty::Hold,Campaign::At(8,9)},
                JourneyAssignment{JourneyDuty::Ambush,Campaign::At(8,8)},JourneyAssignment{}};
            p.manualTask=U"張飛を南へ進め、反撃隊を退ける。劉備か関羽で守備隊を前陣から引き離し、一隊を入れる。";
            p.caution=U"敵が前陣に隣接したままでは占領できない。先に引き離すか押し返そう。";
        }
        else if(j.mission==LuoyangMission)
        {
            const bool west=j.luoyangPlan==LuoyangPlan::WesternDepot;
            p.name=west ? U"林から荷駄陣の片翼を崩す":U"城壁沿いに南門を開く";
            p.purpose=west ? U"関羽は北の林から攻め、劉備か張飛を荷駄陣に通す。":U"関羽を林に残し、劉備と張飛は正面と南から門前へ。";
            p.assignments[1]={JourneyDuty::Ambush,j.Definition().positions[1]};
            p.manualTask=west ? U"敵を挟んで陣から離し、撤収前に一隊を入れる。":U"南門を守る後衛を引き離し、門前に一隊を入れて本隊に合図する。";
            p.caution=U"隣に敵が残っている間は旗を立てられない。追撃役と占領役を分ける。";
        }
        else if(j.Definition().kind==JourneyObjective::CaptureCamp)
        {
            p.name=U"南橋から突破し、河港を奪う";
            p.purpose=U"劉備が南橋手前を支え、関羽は森林で援護。張飛を自分で橋へ進め、敵を崩した後に指揮所を占領する。";
            p.assignments={JourneyAssignment{JourneyDuty::Hold,Campaign::At(10,10)},JourneyAssignment{JourneyDuty::Ambush,Campaign::At(9,9)},JourneyAssignment{}};
            if(j.CurrentPledge()==JourneyPledge::Preserve)
            {
                p.name=U"北橋を回り、兵を守って河港へ";
                p.assignments[0].tile=Campaign::At(10,8);p.assignments[1].tile=Campaign::At(9,8);
                p.purpose=U"兵を守るという返答に沿い、北橋近くから迂回する配置。劉備が橋手前を支え、関羽は森、張飛は突破役として自分で指揮する。";
            }
            p.manualTask=U"見えた敵へ単独戦法か連鎖で突破し、指揮所へ一隊を入れる。二隊連鎖も成立させて勝てば、本隊との連携が次の決戦を早める。";
            p.caution=U"敵を倒すだけでは指揮所を占領したことにならない。安全を確かめて入城する。車の護送と残敵掃討は不要。";
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
        AdaptPlanExample(j,p);AdaptReliefPlanExample(j,p);AdaptContributionPlanExample(j,p);AdaptDefenseRetryPlanExample(j,p);return p;
    }
    bool StrategistJourney::ApplyPlanExample()
    {
        if(stage!=JourneyStage::Preparation) return false;
        const auto plan=PlanExample(*this);
        auto candidate=std::make_unique<StrategistJourney>(*this);candidate->assignments={};
        for(int i=0;i<3;++i)
            if(!candidate->AssignDuty(i,plan.assignments[i].tile,plan.assignments[i].duty)) return false;
        assignments=candidate->assignments;
        message=std::u32string(plan.reason.empty() ? U"見本を配置：":U"提案を採用：")+plan.name+U"。戦場の進行・戦法発動は、自分で指揮する。";
        return true;
    }
}
