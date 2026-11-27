#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyStrategicResponse.hpp"
#include "Components/InstancedStaticMeshComponent.h"
using namespace frontline;
bool ATacticalField::PrepareStrategicDepthPreview(int Phase)
{
    if(Phase<0 || Phase>4) return false;
    if(Phase==2 || Phase==4)
    {
        if(!PrepareGrandBattlePreview(YellowTurbanMission,2,1) || GuangzongVeteranOfficer(State->Journey)<0) return false;
        Issue(ETacticalCommand::Continue);Issue(ETacticalCommand::CoalitionProtect);Issue(ETacticalCommand::Deploy);
        if(Phase==4)
        {
            auto& J=State->Journey;const int Officer=GuangzongVeteranOfficer(J);
            J.world.armies[Officer].tile=J.world.armies[Officer].target=Campaign::At(3,8);J.world.armies[Officer].morale=60;
            J.world.armies[7].tile=J.world.armies[7].target=Campaign::At(4,8);J.RefreshIntelligence();Refresh();
            State->bNoPlayback=false;RunRealtimeCycle(true);AdvancePresentation(60);
        }
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
    if(!Check(PrepareStrategicDepthPreview(0),TEXT("reopen the real flank decision"))) return false;
    State->bNoPlayback=false;State->bAutoBattle=true;bPauseOnBattleAlert=true;State->Realtime.paused=false;
    RunRealtimeCycle(false);
    if(!Check(IsPlaying() && State->DecisionPending.Kind==gunyudecision::EKind::GuangzongFlank &&
        State->DecisionPaused.Kind==gunyudecision::EKind::None,TEXT("a real observed flank departure waits for playback before stopping auto"))) return false;
    AdvancePresentation(60);
    if(!Check(RealtimePaused() && State->bAutoBattle && State->DecisionPaused.Kind==gunyudecision::EKind::GuangzongFlank,
        TEXT("the flank decision pauses with auto still available"))) return false;
    auto Decision=gunyudecision::Capture(State->Journey),Later=Decision;Later.VisibleFlankTile-=1;
    if(!Check(gunyudecision::Compare(Decision,Later).Kind==gunyudecision::EKind::None,
        TEXT("an ongoing flank does not stop again on every cell"))) return false;
    for(auto& Contact:State->Journey.intelligence[0].contacts) if(Contact.id==4) Contact.current=false;
    const auto Hidden=gunyudecision::Capture(State->Journey);
    if(!Check(Hidden.VisibleFlankTile<0 && !Hidden.bFlankThreat,TEXT("hidden flank state cannot become a decision report"))) return false;
    if(!Check(PrepareStrategicDepthPreview(0),TEXT("reopen camps for geometry inspection"))) return false;
    if(!ExportSession(Before)) return false;
    BuildMapScenery();
    if(!Check(ExportSession(After) && Before==After && Tents->GetInstanceCount()>=6 &&
        Tents->GetCollisionEnabled()==ECollisionEnabled::NoCollision && SupplyCrates->GetCollisionEnabled()==ECollisionEnabled::NoCollision,
        TEXT("rear encampments add scenery without altering rules or collision"))) return false;
    for(int I=0;I<Tents->GetInstanceCount();++I)
    {
        FTransform T;Tents->GetInstanceTransform(I,T,false);const FVector P=T.GetLocation();
        for(int Tile:{Campaign::At(8,3),Campaign::At(8,8),Campaign::At(8,11),State->Journey.goal})
            if(!Check(FVector::DistSquared2D(P,TileLocation(Tile))>120*120,TEXT("three fighting lanes and their crossings remain clear of tents"))) return false;
    }
    if(!Check(PrepareStrategicDepthPreview(2) && HasGuangzongVeterans(State->Journey) &&
        State->Journey.world.armies[7].troops==3200+State->Journey.history[YellowTurbanMission].review.grand.leftReturned,
        TEXT("an actual saved relief becomes the next battle's allied company"))) return false;
    if(!ExportSession(Before)) return false;
    if(!Check(ImportSession(Before) && HasGuangzongVeterans(State->Journey),TEXT("veteran identity and real survivors persist without a new save version"))) return false;
    auto& J=State->Journey;const int Officer=GuangzongVeteranOfficer(J);
    J.world.armies[Officer].tile=J.world.armies[Officer].target=Campaign::At(3,8);J.world.armies[Officer].morale=60;
    J.world.armies[7].tile=J.world.armies[7].target=Campaign::At(4,8);J.RefreshIntelligence();Refresh();
    State->bNoPlayback=false;State->bAutoBattle=false;
    RunRealtimeCycle(true);
    const auto Response=gunyumission::Read(CurrentBeat());
    if(!Check(IsPlaying() && Response.Actor==Officer && Response.Caption.Contains(TEXT("義兵の鼓舞")) &&
        ReadMissionResponse().Line.IsEmpty(),TEXT("an actual paid rally creates the rescuer's reply, hidden until playback finishes"))) return false;
    AdvancePresentation(60);
    if(!Check(!ReadMissionResponse().Line.IsEmpty() && !Presentation().VeteranStatus.IsEmpty(),TEXT("the earned aid has a character reply and visible company activity"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_STRATEGIC_DEPTH: PASS (reserve comparison, observed flank decision, hidden/no-repeat protection, clear camps, actual relief, paid veteran rally, playback-safe reply and save)"));
    return true;
}
