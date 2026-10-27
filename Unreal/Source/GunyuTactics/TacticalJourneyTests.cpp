#include "TacticalTrial.h"
#include "TacticalState.h"
#include <queue>

using namespace frontline;

bool ATacticalField::JourneySmokeTest()
{
    const int SavedMission = FirstMission, SavedPolicy = EnemyPolicy;
    const bool SavedCouncil = bStartWithCouncil, SavedPlayback = State->bNoPlayback;
    FirstMission = 0; EnemyPolicy = 0; bStartWithCouncil = true; State->bNoPlayback = true;
    auto Check = [](bool Value, const TCHAR* Reason)
    {
        if (!Value) UE_LOG(LogTemp, Error, TEXT("GUNYU_JOURNEY: FAIL: %s"), Reason);
        return Value;
    };
    Issue(ETacticalCommand::Restart);
    auto& J = State->Journey;
    if (!Check(J.stage == JourneyStage::Encounter && J.world.armies.empty(), TEXT("encounter before any deployment"))) return false;
    SelectAt(Campaign::At(9, 10)); SelectOfficer(0); // No armies exist yet: these must be harmless.
    Issue(ETacticalCommand::Dialogue); Issue(ETacticalCommand::Dialogue); Issue(ETacticalCommand::Dialogue);
    Issue(ETacticalCommand::PromiseRoad); // Existing three-mission route remains playable.
    if (!Check(J.stage == JourneyStage::Preparation && !Presentation().Journey.Plans.IsEmpty(), TEXT("dialogue opens real council"))) return false;
    const auto StartingCamp = J.world.cities[0];
    Issue(ETacticalCommand::PlanSupplies);
    if (!Check(J.world.cities[0].food == StartingCamp.food && J.world.armies.empty(), TEXT("selecting a plan spends nothing"))) return false;
    Issue(ETacticalCommand::Reinforce); Issue(ETacticalCommand::Reinforce);
    const auto CouncilCamp = J.world.cities[0];
    if (!Check(CouncilCamp.troops == StartingCamp.troops + 1200 && CouncilCamp.gold == StartingCamp.gold - 250 && CouncilCamp.food == StartingCamp.food - 500, TEXT("recruitment is paid exactly once"))) return false;
    Issue(ETacticalCommand::PlanAmbush); Issue(ETacticalCommand::Deploy);
    const int ChargedFood = J.world.cities[0].food;
    Issue(ETacticalCommand::Deploy);
    if (!Check(J.chain.concealed[1] && J.world.cities[0].food == ChargedFood && ChargedFood == CouncilCamp.food - 3000, TEXT("deploy uses selected plan once"))) return false;
    for (int Turn = 0; Turn < 12 && J.stage == JourneyStage::Battle; ++Turn) Issue(ETacticalCommand::EndTurn);
    if (!Check(J.stage == JourneyStage::Debrief && !J.won && CanIssue(ETacticalCommand::Retry), TEXT("loss offers retry"))) return false;
    Issue(ETacticalCommand::Retry);
    if (!Check(J.stage == JourneyStage::Preparation && J.world.cities[0].troops == CouncilCamp.troops && J.world.cities[0].food == CouncilCamp.food && J.world.cities[0].gold == CouncilCamp.gold && !CanIssue(ETacticalCommand::Reinforce), TEXT("retry restores pre-battle resources without duplicating recruits"))) return false;
    Issue(ETacticalCommand::PlanSupplies); Issue(ETacticalCommand::Deploy);
    if (!Check(J.world.armies[0].morale == 100 && !J.chain.concealed[1] && J.world.cities[0].food == CouncilCamp.food - 4800, TEXT("supplies change real morale and provisions"))) return false;
    SelectOfficer(1); Issue(ETacticalCommand::PrepareAmbush);
    Issue(ETacticalCommand::Sample); Issue(ETacticalCommand::Execute); Issue(ETacticalCommand::EndTurn);
    SelectOfficer(2); Issue(ETacticalCommand::QueueTactic); Issue(ETacticalCommand::Execute);
    if (!Check(J.won && Presentation().Journey.bDebrief && J.world.Affinity(0, 1) == 43, TEXT("first battle settles survivors, relationships and dialogue"))) return false;
    const auto ReturnedCamp = J.world.cities[0];
    if(State->Journey.NeedsStoryReply()) Issue(ETacticalCommand::StoryReplyPreserve);
    Issue(ETacticalCommand::Continue);
    if (!Check(J.mission == 1 && J.stage == JourneyStage::Preparation && J.world.cities[0].troops == ReturnedCamp.troops && J.world.cities[0].food == ReturnedCamp.food && J.world.cities[0].gold == ReturnedCamp.gold && J.trust == 10, TEXT("next council retains actual resources and trust"))) return false;
    Issue(ETacticalCommand::PlanAmbush);
    const auto EscortCost = J.PreparationCosts(0);
    Issue(ETacticalCommand::Deploy);
    if (!Check(J.world.armies.size() == 6 && J.world.cities[0].food == ReturnedCamp.food - EscortCost.TotalFood(), TEXT("escort charges cart cargo and provisions"))) return false;
    auto Approach = [this, &J](int Army, int Goal, bool AvoidRoad)
    {
        if (Goal < 0 || J.chain.moved[Army]) return;
        const int From=J.world.armies[Army].tile;const bool Occupied=!TacticChain::Free(J.world,Goal);
        std::array<int,TileCount> First;First.fill(-1);First[From]=From;
        std::queue<int> Open;Open.push(From);int Destination=-1;
        while(!Open.empty() && Destination<0)
        {
            const int At=Open.front();Open.pop();
            if(At==Goal || (Occupied && J.world.MapDistance(At,Goal)<=1)) { Destination=First[At];break; }
            for(int Next:J.world.MapNeighbors(At))
                if(StrategistJourney::OnMap(Next) && First[Next]<0 && J.intelligence[0].visible[Next] &&
                   TacticChain::Free(J.world,Next) && (!AvoidRoad || Next/Width!=11))
                { First[Next]=At==From ? Next:First[At];Open.push(Next); }
        }
        if(Destination>=0 && Destination!=From) { SelectOfficer(Army);CommandAt(Destination); }
    };
    auto Attack = [this, &J](int Army)
    {
        const auto Contacts = J.intelligence[0].contacts;
        for (const auto& Contact : Contacts)
            if (Contact.current && J.world.CanStrike(Army, Contact.observed.tile))
            {
                SelectOfficer(Army); SelectAt(Contact.observed.tile);
                if (CanIssue(ETacticalCommand::Strike)) { Issue(ETacticalCommand::Strike); return true; }
            }
        return false;
    };
    auto Fight = [&J, &Approach, &Attack](int Army, bool AvoidRoad)
    {
        if (J.stage != JourneyStage::Battle || !TacticChain::Living(J.world, Army) || Attack(Army)) return;
        int Goal = -1, Distance = 100;
        for (const auto& Contact : J.intelligence[0].contacts)
            if (Contact.current && J.world.MapDistance(J.world.armies[Army].tile, Contact.observed.tile) < Distance)
            { Distance = J.world.MapDistance(J.world.armies[Army].tile, Contact.observed.tile); Goal = Contact.observed.tile; }
        Approach(Army, Goal, AvoidRoad); Attack(Army);
    };
    for (int Phase = 1; Phase <= J.Definition().limit && J.stage == JourneyStage::Battle; ++Phase)
    {
        if (Phase == 1) { SelectOfficer(0); CommandAt(Campaign::At(9, 9)); SelectOfficer(2); CommandAt(Campaign::At(9, 10)); }
        if (Phase == 2) { SelectOfficer(0); CommandAt(Campaign::At(10, 9)); SelectOfficer(1); CommandAt(Campaign::At(9, 9)); }
        Fight(0,true);Fight(1,true);
        const auto* Vanguard=J.intelligence[0].Find(3);
        if(Vanguard && Vanguard->current && J.world.armies[3].troops>0)
        {
            if(!J.world.CanStrike(2,Vanguard->observed.tile)) Approach(2,Vanguard->observed.tile,false);
            if(J.Preview({{2,3,ChainTactic::Breakthrough,-1}}).completed==1)
            {
                SelectOfficer(2);SelectAt(J.world.armies[3].tile);
                Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);
            }
        }
        else if(J.world.armies[3].troops>0) Approach(2,J.goal,false);
        if(J.world.armies[3].troops<=0 && J.world.armies[2].tile/Width==11) Approach(2,Campaign::At(12,10),false);
        if (J.stage == JourneyStage::Battle) Issue(ETacticalCommand::EndTurn);
        UE_LOG(LogTemp,Display,TEXT("GUNYU_ESCORT_PLAY: phase=%d turn=%d cart=%d vanguard=%d reserve=%d"),Phase,J.turn,J.convoyStep,J.world.armies[3].troops,J.world.armies[4].troops);
    }
    if (!Check(J.won && J.recoveredCargo >= 1200, TEXT("escort completed with UI commands"))) return false;
    if(State->Journey.NeedsStoryReply()) Issue(ETacticalCommand::StoryReplyPreserve);
    Issue(ETacticalCommand::Continue); Issue(ETacticalCommand::Deploy);
    for (int Phase = 0; Phase < 12 && J.stage == JourneyStage::Battle; ++Phase)
    {
        for (int Army = 0; Army < 3 && J.stage == JourneyStage::Battle; ++Army)
        {
            const int Zone = Army == 0 ? J.goal : J.Definition().secondGoal;
            bool EnemyNearby = false;
            for (const auto& Contact : J.intelligence[0].contacts)
                if (Contact.current && J.world.MapDistance(Contact.observed.tile, Zone) <= 2) EnemyNearby = true;
            if (Army == 1 || EnemyNearby) Fight(Army, false);
            else Approach(Army, Zone, false);
        }
        if (J.stage == JourneyStage::Battle) Issue(ETacticalCommand::EndTurn);
    }
    if (!Check(J.won && J.mission == 2 && J.trust == 30, TEXT("three councils and three battles preserve the journey"))) return false;
    if(State->Journey.NeedsStoryReply()) Issue(ETacticalCommand::StoryReplyPreserve);
    Issue(ETacticalCommand::Continue);
    if (!Check(J.stage == JourneyStage::Complete && Presentation().Journey.bComplete, TEXT("final scene appears"))) return false;
    FirstMission = SavedMission; EnemyPolicy = SavedPolicy; bStartWithCouncil = SavedCouncil; State->bNoPlayback = SavedPlayback;
    Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp, Display, TEXT("GUNYU_JOURNEY: PASS (encounter, council, resource costs, defeat/retry, three UI playthroughs, relationships, conclusion)"));
    return true;
}
