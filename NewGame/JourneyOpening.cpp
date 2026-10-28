#include "StrategistJourney.hpp"

namespace frontline
{
    const JourneyConvoyPlan& ConvoyPlan(JourneyConvoyRoute route)
    {
        static const JourneyConvoyPlan road{U"街道",U"６区間。案内料なし。先鋒の正面を通るため、押し返すか引きつけて道を開く。",0,
            {Campaign::At(6,11),Campaign::At(7,11),Campaign::At(8,11),Campaign::At(9,11),Campaign::At(10,11),Campaign::At(11,11),Campaign::At(12,11)}};
        static const JourneyConvoyPlan south{U"南の林道",U"７区間。案内料120。南の森を経由して正面を避ける。出口の護衛と、長い護送時間が必要。",120,
            {Campaign::At(6,11),Campaign::At(7,12),Campaign::At(8,12),Campaign::At(9,12),Campaign::At(10,12),Campaign::At(11,12),Campaign::At(12,12),Campaign::At(12,11)}};
        return route==JourneyConvoyRoute::SouthernWood ? south : road;
    }
    bool StrategistJourney::SelectConvoyRoute(JourneyConvoyRoute route)
    {
        if (stage!=JourneyStage::Preparation || !IsRescue() || (route!=JourneyConvoyRoute::Road && route!=JourneyConvoyRoute::SouthernWood)) return false;
        convoyRoute=route;message=ConvoyPlan(route).description;return true;
    }
    const std::vector<int>& StrategistJourney::ConvoyPath() const
    { return ConvoyPlan(IsRescue() ? convoyRoute : JourneyConvoyRoute::Road).tiles; }
    void StrategistJourney::CompleteBattleReview(bool success)
    {
        auto& r=battleReview;r.endedAt=turn;
        if (withdrawn) r.resolution=JourneyResolution::Withdrawn;
        else if (success) r.resolution=Definition().kind==JourneyObjective::CaptureCamp ? JourneyResolution::Captured:HasConvoy() ? JourneyResolution::Delivered :
            Definition().kind==JourneyObjective::HoldPositions ? JourneyResolution::Held :
            world.armies[3].troops<=450 || world.armies[3].morale<=30 ? JourneyResolution::EnemyBroken : JourneyResolution::ExitSecured;
        else r.resolution=world.armies[0].troops+world.armies[1].troops+world.armies[2].troops<1800 ? JourneyResolution::ForceLost :
            HasConvoy() && (world.armies[ConvoyArmy].troops<=0 || (!IsRescue() && recoveredCargo<Definition().requiredProgress)) ? JourneyResolution::ConvoyLost : JourneyResolution::Deadline;
    }
    std::u32string StrategistJourney::BattleReviewLesson() const
    {
        if(battleReview.resolution==JourneyResolution::Captured) return U"敵の撃破から指揮所の占領へつなげた。戦役の目的が済んだ所で追撃を止め、生存兵を次の決戦へ回す。";
        if (battleReview.resolution==JourneyResolution::ConvoyLost)
            return U"車の護衛か積荷を失った。追撃する隊に加え、出口や車の近くを守る隊を残してみよう。";
        if (battleReview.resolution==JourneyResolution::ForceLost || casualties>900)
            return U"兵の損耗が大きかった。勝負を決める場所と、交戦を続けず離れる隊を選び直そう。";
        if (battleReview.enemyStops>0 && battleReview.enemyStops>=battleReview.trafficStops)
            return U"前路の敵で護送が止まった。敵を引き受ける隊を先に置くか、軍議で経路を変えてみよう。";
        if (battleReview.trafficStops>0)
            return U"通路が塞がって車が止まった。護送予定路と味方の配置を確かめ、道を空ける担当を決めよう。";
        if (won && battleReview.tacticSteps==0)
            return U"戦法を温存して任務を達成した。残った兵と糧が、次の軍議で使える選択肢になる。";
        if (won) return U"任務の目的まで策をつなげた。どの隊を前へ出し、誰を残したかを次の配置にも生かそう。";
        return U"次は何を守り、どこで戦いを終えるかを軍議から考え直そう。";
    }
    const char32_t* StrategistJourney::ResolutionName() const
    {
        switch (battleReview.resolution)
        {
        case JourneyResolution::Captured:return U"河港を奪取し、官軍の共同作戦へ";
        case JourneyResolution::EnemyBroken:return U"先鋒を崩して退路を確保";
        case JourneyResolution::ExitSecured:return U"出口を押さえて退路を確保";
        case JourneyResolution::Delivered:return U"護送の約束を達成";
        case JourneyResolution::Held:return U"二つの烽火を維持";
        case JourneyResolution::Withdrawn:return U"生存兵を連れて撤収";
        case JourneyResolution::ForceLost:return U"主力の損耗で任務中止";
        case JourneyResolution::ConvoyLost:return U"護送の必要条件を失った";
        case JourneyResolution::Deadline:return U"任務の期限を超過";
        default:return U"軍議で次の策を考える";
        }
    }
}
