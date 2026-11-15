#include "JourneyCouncilStage.hpp"
#include "JourneyBattlefield.hpp"
#include "StrategistJourney.hpp"

namespace frontline
{
    CouncilBattleBrief BuildCouncilBattleBrief(const StrategistJourney& j)
    {
        if(j.stage!=JourneyStage::Preparation || !j.campaignFlow || j.mission>=YellowTurbanMission)
            return {};

        const auto& mission=j.Definition();
        if(j.mission==0)
        {
            if(j.IsRescue())
            {
                const auto& route=j.ConvoyPath();
                const bool south=j.convoyRoute==JourneyConvoyRoute::SouthernWood;
                return {south ? U"南の林道で、季安の車を通す":U"街道を開けて、季安の車を通す",
                    U"季安が車を出す。道を開ける隊と、出口で車を待つ隊を分けよう。",
                    {{{route.back(),U"護送の出口を守る",U"車が着くのを出口の脇で待つよ。味方の兵でも、車の行く手に立てば通れないからね。"},
                      {mission.positions[1],U"林から敵を引き離す",U"私が林で敵の気を引く。敵が車から離れたら好機だ。斬り切るまで待つことはない。"},
                      {route[3],south ? U"林道の曲がり角を空ける":U"街道の先鋒を押し返す",
                          south ? U"曲がり角まで俺が先に行く。車が追いつく前に、俺も道の脇にどくぞ！":
                              U"車の前に敵がいたら俺が押し返す。で、俺が道の真ん中に立ってたら……軍師、遠慮なくどけって言ってくれ！"}}}};
            }
            return {U"林を使って、民の退路を開く",
                U"先鋒を崩すか、退路の出口を取れば民が通れる。どっちで道を開くか、三人の役を決めよう。",
                {{{mission.goal,U"退路の出口を確保する",U"敵が林へ向いたら、私を出口へ送ってくれ。先鋒が残っていても、出口を押さえられれば民を通せる。"},
                  {mission.positions[1],U"伏兵の森へ先に入る",U"この森に敵を寄せろ。私が伏せて待ち、翼徳が飛び出す隙をつくる。"},
                  {mission.positions[3],U"街道の先鋒を崩す",U"先鋒は俺にやらせろ！ 兄者が林から崩したら追い打ちだ。急ぐなら俺ひとりでも一発いけるぜ。"}}}};
        }

        if(j.mission==1)
        {
            const bool north=j.Battlefield()==JourneyMap::PortNorth;
            return {north ? U"北橋を回り、河港へ":U"橋を渡って、河港を取る",
                U"向こう岸の指揮所を取れば、本隊が渡れる。橋で敵を受ける隊と、最後に旗を立てる隊を分けよう。",
                {{{mission.goal,U"指揮所に旗を立てる",
                      U"橋の先で敵が翼徳を追ったら、私を指揮所へ。周りを空けて旗を立てれば、本隊に合図できる。"},
                  {Campaign::At(9,north ? 8:9),U"西岸の林から渡河を援護",
                      U"私は西岸の林に入る。橋が詰まってから三隊で押すより、渡る一隊が狙われた時に横から助けよう。"},
                  {Campaign::At(11,north ? 8:11),north ? U"北橋を切り開く":U"南橋を切り開く",
                      north ? U"北橋は俺が抜く！ 敵が俺を追ったら、兄貴を港へ走らせてくれ！":
                          U"南橋の先で反撃が来るなら、俺が迎える！ その間に兄貴を指揮所へ走らせてくれ。"}}}};
        }

        return {U"追う前に、二つの烽火を守る",
            j.RequiredHoldTurns()==2 ? U"河港で合わせた合図がある。南北の火を同時に二指揮守れば、本隊が包囲に入れる。":
                U"本隊は二つの火を見ている。南北で持ち場を分け、どちらも消さずに三指揮をしのごう。",
            {{{mission.goal,U"北の烽火を守る",U"北は私が預かる。敵一隊なら固守で時間を稼げる。二隊目が来る前に、雲長の援護を頼むよ。"},
              {mission.positions[1],U"林から手薄な翼を助ける",U"私は林から両翼を見る。敵を追う隊を増やすより、手薄な方へ私の一手を回してくれ。"},
              {mission.secondGoal,U"南の烽火に残る",U"南は俺に任せろ！ 敵が退いても追わねえ。もう片方の火が揃うまで、ここを空けねえぞ。"}}}};
    }
}
