#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyStrategicResponse.hpp"
using namespace frontline;
bool ATacticalField::PrepareStrategicDepthPreview(int Phase)
{
    if(Phase<0 || Phase>3) return false;
    if(Phase==2)
    {
        if(!PrepareGrandBattlePreview(YellowTurbanMission,2,1) || GuangzongVeteranOfficer(State->Journey)<0) return false;
        Issue(ETacticalCommand::Continue);Issue(ETacticalCommand::CoalitionProtect);Issue(ETacticalCommand::Deploy);
        return HasGuangzongVeterans(State->Journey) && State->Journey.stage==JourneyStage::Battle;
    }
    if(!PrepareGrandBattlePreview(YellowTurbanMission,0,2)) return false;
    auto& J=State->Journey;
    J.world.armies[0].tile=J.world.armies[0].target=Campaign::At(11,8);
    J.world.armies[1].tile=J.world.armies[1].target=Campaign::At(3,5);
    J.world.armies[2].tile=J.world.armies[2].target=Campaign::At(11,7);
    State->Selected=0;State->Target=3;J.RefreshIntelligence();Refresh();
    if(Phase==1)
    {
        Issue(ETacticalCommand::RealtimeStep);AdvancePresentation(60);
        if(const auto* Seen=J.intelligence[0].Find(4);Seen && Seen->current) SelectAt(Seen->observed.tile);
    }
    if(Phase==3)
    {
        State->bNoPlayback=false;PlaybackSpeed=1;SelectOfficer(2);SelectTactic(int(ChainTactic::SerpentRoar));
        Issue(ETacticalCommand::QueueTactic);
        if(!FireManualTactics() || !IsPlaying()) return false;
        AdvancePresentation(CurrentBeat()->Duration*.74f);
    }
    return State->Journey.stage==JourneyStage::Battle;
}
bool ATacticalField::StrategicDepthSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why){if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_STRATEGIC_DEPTH: FAIL %s"),Why);return V;};
    if(!Check(PrepareStrategicDepthPreview(0),TEXT("real grand battlefield can compare a concentrated attack"))) return false;
    std::vector<std::uint8_t> Before,After;if(!ExportSession(Before)) return false;Refresh();
    if(!Check(ExportSession(After) && Before==After,TEXT("reading remaining officers never commits a tactic or time"))) return false;
    bool Solo=false,Team=false;
    for(int I=0;I<int(State->BattleChoices.size());++I)
    {
        const auto& Plan=State->BattleChoices[I];const auto& Card=Presentation().BattleChoices[I];
        if(!Check(Card.Detail.Contains(JourneyText(RemainingOfficers(State->Journey,Plan.orders))) && Card.Cost.Contains(TEXT("温存")),TEXT("comparison uses the exact remaining action budget"))) return false;
        Solo|=Plan.orders.size()==1;Team|=Plan.orders.size()>1;
    }
    if(!Check(Solo && Team,TEXT("the same visible opponent offers solo and combo with different officers reserved"))) return false;
    const int Origin=State->Journey.world.armies[4].tile;
    Issue(ETacticalCommand::RealtimeStep);AdvancePresentation(60);
    if(!Check(State->Journey.world.armies[4].tile!=Origin && RealtimePaused(),TEXT("a real command cycle moves the enemy flank and returns paused"))) return false;
    if(!Check(PrepareStrategicDepthPreview(2) && HasGuangzongVeterans(State->Journey) &&
        State->Journey.world.armies[7].troops==3200+State->Journey.history[YellowTurbanMission].review.grand.leftReturned,
        TEXT("an actual saved relief becomes the next battle's allied company"))) return false;
    if(!ExportSession(Before)) return false;
    if(!Check(ImportSession(Before) && HasGuangzongVeterans(State->Journey),TEXT("veteran identity and real survivors persist without a new save version"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_STRATEGIC_DEPTH: PASS (real reserve comparison, enemy flank movement, actual relief, chapter transition, veteran company, immutable reading and save)"));
    return true;
}
