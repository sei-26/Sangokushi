#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalPortLandscape.h"

void ATacticalField::BuildPortAssaultPresentation()
{
    using namespace frontline;
    const auto& J=State->Journey;
    auto& H=State->HUD;
    const auto& Port=J.battleReview.port;
    auto Name=[&J](int Officer) { return JourneyText(J.world.generals[J.world.armies[Officer].general].name); };
    const auto Crossings=gunyuportlandscape::Bridges(J);
    for(int Bridge=0;Bridge<2 && Crossings.IsValidIndex(Bridge);++Bridge)
    {
        const int Officer=Port.crossedBy[Bridge];
        H.PortCrossings[Bridge].Tile=Crossings[Bridge];
        H.PortCrossings[Bridge].Caption=Officer>=0 ? FString(Bridge==0 ? TEXT("北橋を突破"):TEXT("南橋を突破"))+TEXT(" ／ ")+Name(Officer):
            FString(Bridge==0 ? TEXT("北橋 ／ 側面へ回る"):TEXT("南橋 ／ 正面へ進む"));
    }
    H.MissionDetail+=TEXT("\n北橋は側面への道、南橋は指揮所に近い道。渡る隊と、最後に旗を立てる隊を決めよう。占領は同じ隊でも勝利になる。");

    if(!gunyudecision::CanSeePortOpening(J)) return;
    const int Remaining=StrategistJourney::PortOpeningCycles-(J.turn-Port.clearedOn);
    H.bPortOpening=true;
    H.MissionAdvice=TEXT("指揮所へ旗を立てよう");
    const FString Limit=Remaining==0 ? FString(TEXT("この指揮まで")):FString::Printf(TEXT("あと%d指揮"),Remaining);
    H.MissionStep=TEXT("河港占領 ｜ ")+Name(Port.clearedBy)+TEXT("隊が道を開いた。別隊の占領で連携 ／ ")+Limit;
    H.MissionDetail+=TEXT("\n連携の機会：別の隊がこの間に占領すると、次の烽火戦の守備が一指揮短くなる。同じ隊でも、期限後でも、安全に占領すれば勝利。");
}
