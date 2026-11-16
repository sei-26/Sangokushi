#include "TacticalTrial.h"
#include "TacticalState.h"

void ATacticalField::BuildPortAssaultPresentation()
{
    using namespace frontline;
    const auto& J=State->Journey;
    auto& H=State->HUD;
    const auto& Port=J.battleReview.port;
    auto Name=[&J](int Officer) { return JourneyText(J.world.generals[J.world.armies[Officer].general].name); };
    for(int Bridge=0;Bridge<2;++Bridge)
    {
        const int Officer=Port.crossedBy[Bridge];
        if(Officer<0) continue;
        H.PortCrossings[Bridge].Tile=Campaign::At(11,Bridge==0 ? 8:11);
        H.PortCrossings[Bridge].Caption=FString(Bridge==0 ? TEXT("北橋を突破"):TEXT("南橋を突破"))+TEXT(" ／ ")+Name(Officer);
    }

    if(!gunyudecision::CanSeePortOpening(J)) return;
    const int Remaining=StrategistJourney::PortOpeningCycles-(J.turn-Port.clearedOn);
    H.bPortOpening=true;
    H.MissionAdvice=TEXT("別の隊で旗を立てよう");
    const FString Limit=Remaining==0 ? FString(TEXT("この指揮まで")):FString::Printf(TEXT("あと%d指揮"),Remaining);
    H.MissionStep=TEXT("河港占領 ｜ ")+Name(Port.clearedBy)+TEXT("隊が道を開いた。別の隊を指揮所へ ／ ")+Limit;
    H.MissionDetail+=TEXT("\n別の隊がこの間に占領すると、本隊との合図が整い、次の烽火戦で必要な守備が一指揮短くなります。");
}
