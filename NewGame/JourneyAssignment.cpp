#include "StrategistJourney.hpp"
#include "JourneyBattlefield.hpp"
namespace frontline
{
    const char32_t* DutyName(JourneyDuty duty)
    { return duty==JourneyDuty::Hold ? U"要所守備":duty==JourneyDuty::Ambush ? U"伏兵配置":U"指定なし"; }
    std::u32string StrategistJourney::AssignmentProblem(int officer,int tile,JourneyDuty duty) const
    {
        if(stage!=JourneyStage::Preparation || officer<0 || officer>=3) return U"担当の指定は軍議で行う。";
        if(duty==JourneyDuty::None) return {};
        if(duty!=JourneyDuty::Hold && duty!=JourneyDuty::Ambush) return U"指定できない役目。";
        if(!OnMap(tile)) return U"地図のマスを選ぼう。";
        const auto terrain=BattlefieldTerrain(mission,tile%Width,tile/Width,IsRescue());
        if(terrain==Terrain::River || terrain==Terrain::Sea) return U"川には進入できない。橋か陸地を選ぼう。";
        if(tile==Campaign::At(2,mission==2 ? 6:10) || tile==Campaign::At(14,3)) return U"城の敷地には配置できない。";
        if(duty==JourneyDuty::Ambush && (world.generals[officer].tactic!=Tactic::Ambush || terrain!=Terrain::Forest)) return U"伏兵配置は、関羽と森林の組み合わせで指定する。";
        for(int i=0;i<3;++i)
            if(i!=officer && assignments[i].duty!=JourneyDuty::None && assignments[i].tile==tile) return U"その地点は別の武将が担当している。";
        return {};
    }
    bool StrategistJourney::AssignDuty(int officer,int tile,JourneyDuty duty)
    {
        const auto problem=AssignmentProblem(officer,tile,duty);
        if(!problem.empty()) { message=problem;return false; }
        assignments[officer]={duty,duty==JourneyDuty::None ? -1:tile,false};
        message=world.generals[officer].name+U"の担当："+DutyName(duty)+U"。出陣後、到着して準備するまで配置成立ではない。";
        return true;
    }
    std::u32string StrategistJourney::AssignmentStatus(int officer) const
    {
        if(officer<0 || officer>=3) return {};
        const auto& a=assignments[officer];if(a.duty==JourneyDuty::None) return U"担当の指定なし";
        auto number=[](int v) { const auto s=std::to_string(v);return std::u32string(s.begin(),s.end()); };
        return std::u32string(DutyName(a.duty))+U" ("+number(a.tile%Width)+U","+number(a.tile/Width)+U") / "+
            (a.completed ? U"配置成立":stage==JourneyStage::Preparation ? U"出陣後に実行":U"未成立");
    }
    void StrategistJourney::RecordAssignment(int officer)
    {
        if(officer<0 || officer>=3 || !TacticChain::Living(world,officer)) return;
        auto& a=assignments[officer];if(a.completed || a.duty==JourneyDuty::None || world.armies[officer].tile!=a.tile) return;
        if((a.duty==JourneyDuty::Hold && guarding[officer]) || (a.duty==JourneyDuty::Ambush && chain.concealed[officer]))
        {
            a.completed=true;
            const char32_t* words[]={U"「皆、ここで支えよう。軍師、次の合図を待っている。」",U"「任された場所についた。軍師、ここから先をつなげ。」",U"「着いたぞ、軍師！ 次の手は、お前が決めろよ！」"};
            Report(world.generals[officer].name+words[officer]+U" / "+AssignmentStatus(officer));
        }
    }
}
