#include "TacticalTrial.h"
#include "TacticalState.h"

FString ATacticalField::ReadCampaignCarryover() const
{
    using namespace frontline;
    const auto& J=State->Journey;
    if(!J.campaignFlow) return {};
    const bool Planning=J.stage==JourneyStage::Preparation;
    const bool Won=J.won && (J.stage==JourneyStage::Debrief || J.stage==JourneyStage::Complete);
    if(!Planning && !Won) return {};
    if(Won && J.mission==0 && J.shortPrelude)
        return TEXT("帰還兵・残った兵糧・武将の信頼を引き継ぎ、広宗の本戦へ。");

    if((Won && J.mission==0) || (Planning && J.mission==1))
    {
        const auto& Rescue=J.history[0];
        TArray<FString> Changes;
        if(Rescue.rescued) Changes.Add(TEXT("季安の案内で河港の期限＋2指揮"));
        if(Rescue.story.reply==JourneyPledge::Preserve) Changes.Add(TEXT("兵を守る約束で北橋から出陣"));
        if(Rescue.story.reply==JourneyPledge::Coordinate) Changes.Add(TEXT("連携の約束で南橋から出陣"));
        return FString::Join(Changes,TEXT(" ／ "));
    }

    if(((Won && J.mission==1) || (Planning && J.mission==2)) && J.HasPortSignal())
        return TEXT("河港で通した合図が本隊に届き、烽火は3指揮から2指揮に");

    if((Won && J.mission==YellowTurbanMission) || (Planning && J.mission==CoalitionMission && !J.legacyBattlefield))
    {
        const auto& Support=J.GuangzongSupport();
        TArray<FString> Changes;
        const auto& previous=J.history[YellowTurbanMission];const auto& grand=previous.review.grand;
        if(previous.won && grand.relievedOn && grand.leftReturned>0)
            Changes.Add(JourneyText(J.world.generals[grand.reliefOfficer].name)+TEXT("が救った左翼の案内で先行配置・士気支援"));
        if(Support.swift) Changes.Add(TEXT("鄒靖の援兵が張飛の前進を支える"));
        if(Support.roadOfficer>=0) Changes.Add(TEXT("広宗で助けた民の案内で関羽が脇道から出陣"));
        return FString::Join(Changes,TEXT(" ／ "));
    }

    if(((Won && J.mission==CoalitionMission) || (Planning && J.HasSunJian())) && J.HasCoalitionGuide())
        return TEXT("張邈の案内で、孫堅隊が合流地点に近い場所から進軍");

    if(((Won && J.HasSunJian()) || (Planning && J.mission==LuoyangMission)) && J.SunJianTrust()>=SunJianFlankTrust)
        return TEXT("陽人で得た孫堅の信頼で、洛陽西郊の荷駄隊を狙える");

    if(Won && J.mission==LuoyangMission && J.luoyangPlan==LuoyangPlan::WesternDepot)
        return FString::Printf(TEXT("洛陽西郊から持ち帰った兵糧＋%d"),WesternDepotFood);

    return {};
}
