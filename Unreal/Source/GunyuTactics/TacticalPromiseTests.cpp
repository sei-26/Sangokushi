#include "TacticalTrial.h"
#include "TacticalState.h"

using namespace frontline;

bool ATacticalField::StoryPromiseSmokeTest()
{
    const int SavedMission = FirstMission, SavedPolicy = EnemyPolicy;
    const bool SavedCouncil = bStartWithCouncil, SavedPlayback = State->bNoPlayback;
    FirstMission = 0; EnemyPolicy = 0; bStartWithCouncil = true; State->bNoPlayback = true;
    auto Check = [](bool Value, const TCHAR* Reason)
    {
        if (!Value) UE_LOG(LogTemp, Error, TEXT("GUNYU_PROMISE: FAIL: %s"), Reason);
        return Value;
    };
    Issue(ETacticalCommand::Restart);
    for (int I = 0; I < 3; ++I) Issue(ETacticalCommand::Dialogue);
    const int Food = State->Journey.world.cities[0].food;
    Issue(ETacticalCommand::PromiseRoad); Issue(ETacticalCommand::PromiseRescue);
    if (!Check(State->Journey.IsRescue() && State->Journey.world.cities[0].food == Food && State->Journey.PreparationCosts(0).TotalFood() == 3900, TEXT("promise previews actual escort bill without spending"))) return false;
    const int Bond = State->Journey.world.Affinity(0, 1);
    Issue(ETacticalCommand::TalkLiu); const FString Liu = Presentation().Journey.Dialogue;
    Issue(ETacticalCommand::TalkGuan); const FString Guan = Presentation().Journey.Dialogue;
    Issue(ETacticalCommand::TalkZhang); const FString Zhang = Presentation().Journey.Dialogue;
    if (!Check(Liu != Guan && Guan != Zhang && State->Journey.world.Affinity(0, 1) == Bond, TEXT("different voices do not farm relationships"))) return false;
    Issue(ETacticalCommand::Deploy);
    auto& J = State->Journey;
    if (!Check(J.world.armies.size() == 6 && J.world.armies[5].troops == 600 && J.world.cities[0].food == Food - 3900 && Presentation().Objective.Contains(TEXT("避難車")), TEXT("real escort, real provisions and visible objective"))) return false;
    Issue(ETacticalCommand::Sample); Issue(ETacticalCommand::Execute);
    if (!Check(J.stage == JourneyStage::Battle && J.lastChain.completed == 3, TEXT("head-on chain does not complete rescue"))) return false;
    const int CampFood = J.world.cities[0].food;
    Issue(ETacticalCommand::Withdraw);
    if (!Check(!J.withdrawn && J.stage == JourneyStage::Battle && J.world.cities[0].food == CampFood, TEXT("withdrawal confirmation changes no simulation"))) return false;
    Issue(ETacticalCommand::Cancel);
    Issue(ETacticalCommand::Withdraw);
    if (!Check(J.stage == JourneyStage::Battle, TEXT("cancel cancels withdrawal confirmation"))) return false;
    Issue(ETacticalCommand::Withdraw);
    if (!Check(J.withdrawn && !J.won && J.trust == 0 && J.history[0].withdrawn && !J.history[0].rescued, TEXT("withdrawal is not victory and settles actual survivors"))) return false;
    const auto Settled = J.world.cities[0];
    Issue(ETacticalCommand::Withdraw);
    if (!Check(J.world.cities[0].food == Settled.food && J.world.cities[0].troops == Settled.troops, TEXT("withdrawal cannot duplicate settlement"))) return false;
    Issue(ETacticalCommand::TalkLiu);
    if (!Check(Presentation().Journey.Dialogue.Contains(TEXT("引く")), TEXT("dialogue recognizes withdrawal"))) return false;
    Issue(ETacticalCommand::Retry); Issue(ETacticalCommand::Deploy);
    if (!Check(J.IsRescue() && J.world.cities[0].food == Food - 3900 && !J.history[0].completed, TEXT("retry restores pledge and clears unearned results"))) return false;
    auto Attack = [this, &J](int Id)
    {
        const auto Contacts = J.intelligence[0].contacts;
        for (const auto& Contact : Contacts)
            if (Contact.current && J.world.CanStrike(Id, Contact.observed.tile))
            {
                SelectOfficer(Id); SelectAt(Contact.observed.tile);
                if (CanIssue(ETacticalCommand::Strike)) { Issue(ETacticalCommand::Strike); return true; }
            }
        return false;
    };
    for (int Phase = 1; Phase <= J.TurnBudget() && J.stage == JourneyStage::Battle; ++Phase)
    {
        if (Phase == 1) { SelectOfficer(0); CommandAt(Campaign::At(9, 9)); SelectOfficer(2); CommandAt(Campaign::At(9, 10)); }
        if (Phase == 2) { SelectOfficer(0); CommandAt(Campaign::At(10, 9)); SelectOfficer(1); CommandAt(Campaign::At(9, 9)); }
        for (int Id = 0; Id < 3 && J.stage == JourneyStage::Battle; ++Id)
        {
            if (Attack(Id) || J.chain.moved[Id]) continue;
            int Target = -1, Distance = 100;
            for (const auto& Contact : J.intelligence[0].contacts)
                if (Contact.current && J.world.MapDistance(J.world.armies[Id].tile, Contact.observed.tile) < Distance)
                { Distance = J.world.MapDistance(J.world.armies[Id].tile, Contact.observed.tile); Target = Contact.observed.tile; }
            int Next = -1;
            for (int Tile : J.world.MapNeighbors(J.world.armies[Id].tile))
                if (StrategistJourney::OnMap(Tile) && Tile / Width != 11 && J.intelligence[0].visible[Tile] && TacticChain::Free(J.world, Tile) && J.world.MapDistance(Tile, Target) < Distance)
                { Distance = J.world.MapDistance(Tile, Target); Next = Tile; }
            if (Next >= 0) { SelectOfficer(Id); CommandAt(Next); }
            Attack(Id);
        }
        if (J.stage == JourneyStage::Battle) Issue(ETacticalCommand::EndTurn);
        UE_LOG(LogTemp, Display, TEXT("GUNYU_PROMISE_PHASE: %d cart=%d enemies=%d,%d"), Phase, J.convoyStep, J.world.armies[3].troops, J.world.armies[4].troops);
    }
    if (!Check(J.won && J.history[0].rescued && J.convoyStep == 6, TEXT("rescue completed through public UI orders"))) return false;
    Issue(ETacticalCommand::TalkGuan);
    if (!Check(Presentation().Journey.Dialogue.Contains(TEXT("車")), TEXT("officer remembers what was protected"))) return false;
    Issue(ETacticalCommand::Continue);
    if (!Check(Presentation().Journey.History.Contains(TEXT("季安")), TEXT("promise remains in the next council record"))) return false;
    Issue(ETacticalCommand::Reinforce); // Replenish the longer rescue's losses from real reserves.
    Issue(ETacticalCommand::Deploy);
    if (!Check(J.world.generals[5].name == U"季安の糧車", TEXT("rescued person returns as the next mission's carrier"))) return false;
    auto Approach = [this, &J](int Id, int Target, bool AvoidRoad)
    {
        if (Target < 0 || J.chain.moved[Id]) return;
        int Next = -1, Distance = J.world.MapDistance(J.world.armies[Id].tile, Target);
        for (int Tile : J.world.MapNeighbors(J.world.armies[Id].tile))
            if (StrategistJourney::OnMap(Tile) && (!AvoidRoad || Tile / Width != 11) && J.intelligence[0].visible[Tile] && TacticChain::Free(J.world, Tile) && J.world.MapDistance(Tile, Target) < Distance)
            { Distance = J.world.MapDistance(Tile, Target); Next = Tile; }
        if (Next >= 0) { SelectOfficer(Id); CommandAt(Next); }
    };
    auto Fight = [&J, &Attack, &Approach](int Id, bool AvoidRoad)
    {
        if (J.stage != JourneyStage::Battle || !TacticChain::Living(J.world, Id) || Attack(Id)) return;
        int Target = -1, Distance = 100;
        for (const auto& Contact : J.intelligence[0].contacts)
            if (Contact.current && J.world.MapDistance(J.world.armies[Id].tile, Contact.observed.tile) < Distance)
            { Distance = J.world.MapDistance(J.world.armies[Id].tile, Contact.observed.tile); Target = Contact.observed.tile; }
        Approach(Id, Target, AvoidRoad); Attack(Id);
    };
    for (int Phase = 1; Phase <= J.TurnBudget() && J.stage == JourneyStage::Battle; ++Phase)
    {
        if (Phase == 1) { SelectOfficer(0); CommandAt(Campaign::At(9, 9)); SelectOfficer(2); CommandAt(Campaign::At(9, 10)); }
        if (Phase == 2) { SelectOfficer(0); CommandAt(Campaign::At(10, 9)); SelectOfficer(1); CommandAt(Campaign::At(9, 9)); }
        for (int Id = 0; Id < 3 && J.stage == JourneyStage::Battle; ++Id) Fight(Id, true);
        if (J.stage == JourneyStage::Battle) Issue(ETacticalCommand::EndTurn);
    }
    if (!Check(J.won && J.mission == 1, TEXT("rescued carrier's next mission remains winnable"))) return false;
    Issue(ETacticalCommand::Continue); Issue(ETacticalCommand::Deploy);
    for (int Phase = 0; Phase < J.TurnBudget() && J.stage == JourneyStage::Battle; ++Phase)
    {
        for (int Id = 0; Id < 3 && J.stage == JourneyStage::Battle; ++Id)
        {
            const int Zone = Id == 0 ? J.goal : J.Definition().secondGoal;
            bool EnemyNearby = false;
            for (const auto& Contact : J.intelligence[0].contacts)
                if (Contact.current && J.world.MapDistance(Contact.observed.tile, Zone) <= 2) EnemyNearby = true;
            if (Id == 1 || EnemyNearby) Fight(Id, false);
            else Approach(Id, Zone, false);
        }
        if (J.stage == JourneyStage::Battle) Issue(ETacticalCommand::EndTurn);
    }
    if (!Check(J.won && J.mission == 2 && J.history[0].rescued, TEXT("new rescue route reaches third victory with real resources"))) return false;
    Issue(ETacticalCommand::Continue);
    if (!Check(J.stage == JourneyStage::Complete, TEXT("new rescue route reaches conclusion"))) return false;
    FirstMission = SavedMission; EnemyPolicy = SavedPolicy; bStartWithCouncil = SavedCouncil; State->bNoPlayback = SavedPlayback;
    Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp, Display, TEXT("GUNYU_PROMISE: PASS (pledge, real convoy, three voices, withdrawal/confirmation, retry, rescued carrier, three-battle rescue route)"));
    return true;
}
