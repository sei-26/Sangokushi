#include "TacticalTrial.h"
#include "TacticalState.h"

using namespace frontline;

bool ATacticalField::BuildDutyTraffic()
{
    const auto& J=State->Journey;
    auto NextTile=[&](int Id)
    {
        if(!TacticChain::Living(J.world,Id) || J.chain.moved[Id]) return -1;
        const auto& Order=State->Realtime.orders[Id];
        // Reaching a waypoint advances the standing order this cycle. It
        // does not also move to the first tile of the next leg.
        if(Order.kind==JourneyDirective::March && Order.destination==J.world.armies[Id].tile) return -1;
        const auto& Route=State->OrderReads[Id].route;
        return Route.size()>1 ? Route[1]:-1;
    };
    const int Id=State->Selected,Next=NextTile(Id);
    if(Next<0) return false;
    TArray<FString> Others;
    for(int Other=0;Other<3;++Other)
        if(Other!=Id && NextTile(Other)==Next)
            Others.Add(JourneyText(J.world.generals[J.world.armies[Other].general].name));
    if(Others.IsEmpty()) return false;
    auto& P=State->HUD.DutyProgress;
    P.Caption=TEXT("行軍が重なる・")+(Others.Num()==1 ? Others[0]:FString(TEXT("味方２隊")));
    P.Detail=FString::Join(Others,TEXT("・"))+TEXT("と次の行軍先が重なっています。先に進める隊を選び、ほかの行き先を変えるか、Xで行軍指示を止めましょう。");
    P.InspectTile=Next;P.InspectLabel=TEXT("行軍が重なる地点を見る");P.bTraffic=true;
    return true;
}
