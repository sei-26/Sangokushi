#include "JourneyCampaign.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    void StrategistJourney::ResetCampaign() { Reset();campaignFlow=true; }
    bool StrategistJourney::UpgradeCampaignAtCouncil()
    { if(stage!=JourneyStage::Preparation) return false;campaignFlow=true;return true; }
    std::u32string StrategistJourney::ObjectiveBrief() const
    {
        if(campaignFlow && mission==2) return RequiredHoldTurns()==2 ? U"両翼の烽火を２指揮維持し、本隊の包囲を完成させる":U"両翼の烽火を３指揮維持し、本隊の包囲を完成させる";
        return Definition().objective;
    }
    JourneyCampaignBeat CampaignBeat(const StrategistJourney& j)
    {
        JourneyCampaignBeat b;
        if(!j.campaignFlow) return b;
        const bool finished=j.stage==JourneyStage::Debrief && j.won;
        if(j.stage==JourneyStage::Complete)
            return {U"黄巾の戦役を越えて",j.history[0].won && j.history[1].won ? U"民の退路、河港、決戦の両翼。三つの采配が、一人の軍師の実績になった。":U"任された戦いを越え、主人公は義勇軍の正式な軍師として迎えられた。",U"次章以降の歴史戦役は今後の制作。",U"義勇軍の正式な軍師"};
        if(j.mission==0)
        {
            b.phase=U"第一幕 ／ 民の退路から、義勇軍へ";
            b.change=finished ? (j.history[0].rescued ? U"避難車が到着し、生き延びた季安たちが後方を支える。主人公は一戦の作戦を任される。":U"退路が開き、義勇軍が集結。主人公は次の一戦の作戦を任される。"):
                U"黄巾の乱で道を失った民と出会う。退路を作る初陣。";
            b.next=U"次の局面：数日後、河港を急襲。敵の指揮所を奪い、官軍への合流路を開く。";
            if(finished && j.history[0].story.reply==JourneyPledge::Preserve) b.next+=U" 返答に従い、北橋へ回る配置で出陣する。";
            if(finished && j.history[0].story.reply==JourneyPledge::Coordinate) b.next+=U" 返答に従い、南橋で連鎖突破を狙う配置で出陣する。";
            b.standing=finished ? U"一戦の作戦を任される軍師":U"民と出会った、名もない若者";
        }
        else if(j.mission==1)
        {
            b.phase=U"第二幕 ／ 数日後、河港から戦役の主戦線へ";
            b.change=finished ? U"河港を奪取して敵の連絡路を断った。義勇軍は官軍の共同作戦に加わり、主人公は決戦の一方面を任される。":
                U"初陣の働きが認められ、今度は攻める一戦を任される。橋を越え、敵の指揮所へ。";
            b.next=U"次の局面：戦役は数週間先の決戦へ。二つの烽火を守り、本隊の包囲を完成させる。";
            b.standing=finished ? U"共同作戦の一方面を預かる軍師":U"河港急襲の作戦責任者";
        }
        else
        {
            b.phase=U"第三幕 ／ 数週間後、本隊の決戦を支える";
            b.change=finished ? U"両翼の烽火が消えず、本隊の包囲が完成した。主人公は劉備から正式に軍師として迎えられる。":
                (j.history[1].campaignFlow ? U"河港の制圧を足場に決戦へ進んだ。左右の守備を分担し、本隊の攻撃が決まる時間を作る。":U"前の任務を足場に決戦へ進む。左右の守備を分担し、本隊の攻撃が決まる時間を作る。");
            b.next=U"この戦いの先：黄巾の戦役に決着をつけ、義勇軍の正式な軍師として次の時代を見据える。";
            b.standing=finished ? U"義勇軍の正式な軍師":U"決戦の両翼を預かる軍師";
        }
        if(j.mission==1)
        {
            if(j.CurrentPledge()==JourneyPledge::Preserve) b.change+=U" 初陣後の返答で兵を守ると決め、北橋へ回る出陣配置を選んだ。";
            else if(j.CurrentPledge()==JourneyPledge::Coordinate) b.change+=U" 初陣後に皆をつなぐと約束し、南橋からの連鎖突破を狙う配置を選んだ。";
        }
        if(finished && j.history[j.mission].story.pledge!=JourneyPledge::None)
            b.change+=j.history[j.mission].story.pledgeKept ? U" 出陣前の約束も果たし、武将たちの信頼に応えた。":U" だが出陣前の約束は果たしていない。勝利とは別に、その代償が武将との会話に残る。";
        if(j.mission==1 && j.history[0].rescued)
            b.change+=U" 季安たちの先導が急襲を支え、河港戦の期限に２指揮の余裕が生まれた。";
        if(j.mission==2 && j.RequiredHoldTurns()==2)
            b.change+=U" 前戦での二隊連鎖をもとに本隊との合図が整い、包囲完成までの保持が３指揮から２指揮へ短縮された。";
        if(j.stage==JourneyStage::Debrief && !j.won)
        { b.change=U"任務はまだ達成していない。戦役の先へ進まず、直前の軍議から立て直せる。";b.next=U"次の一手：配置と準備を見直し、同じ戦役へ再挑戦。"; }
        return b;
    }
}
