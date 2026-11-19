#include "StrategistJourney.hpp"
#include <memory>

namespace frontline
{
    bool StrategistJourney::CanChooseGrandApproach() const
    {
        const auto& grand=battleReview.grand;
        return stage==JourneyStage::Battle && grand.ordersEnabled && turn==1 &&
            grand.approach==GrandApproach::None && !grand.openedOn;
    }

    bool StrategistJourney::ChooseGrandApproach(GrandApproach choice)
    {
        if(!CanChooseGrandApproach() || (choice!=GrandApproach::RelieveLeft && choice!=GrandApproach::CentralAssault)) return false;
        auto& grand=battleReview.grand;
        grand.approach=choice;grand.chosenOn=turn;++world.revision;
        Report(choice==GrandApproach::RelieveLeft ?
            U"主人公「皇甫嵩殿、北へ回って左翼を支えてください。俺たちも北陣を崩す。本隊が戻るまで、中央は無理に押さない！」":
            U"主人公「本隊は中央へ！ 前陣の旗に続いて押し込んでください。北の仲間は苦しくなる。救いに回す隊を残すかは、俺が決める！」");
        return true;
    }

    void StrategistJourney::CheckGrandRelief()
    {
        auto& grand=battleReview.grand;
        if(!grand.ordersEnabled || grand.relievedOn || !TacticChain::Living(world,6)) return;
        const auto* north=intelligence[0].Find(8);
        if(TacticChain::Living(world,8) && (!north || !north->current || north->observed.morale>25)) return;
        for(int officer=0;officer<3;++officer)
            if(TacticChain::Living(world,officer) && !chain.disordered[officer] &&
                (world.MapDistance(world.armies[officer].tile,world.armies[6].tile)<=1 ||
                    (world.armies[officer].tile==GrandNorthPass && world.MapDistance(world.armies[6].tile,GrandLeftAnchor)<=1)))
            {
                grand.relievedOn=turn;grand.reliefOfficer=officer;++world.revision;
                Report(world.generals[officer].name+U"隊が左翼へ到着。北陣を退け、道がつながった。皇甫嵩「北の旗は残った！ 全軍、前陣へ続け！」");
                return;
            }
    }

    int StrategistJourney::GrandMainTarget() const
    {
        const auto& grand=battleReview.grand;
        if(grand.approach==GrandApproach::RelieveLeft && !grand.relievedOn && TacticChain::Living(world,6)) return GrandNorthPass;
        if(grand.approach==GrandApproach::CentralAssault && world.armies[5].tile%Width<9)
        {
            const int at=world.armies[5].tile;
            if(at%Width<7 || at/Width!=8) return Campaign::At(7,8);
            for(int army=0;army<int(world.armies.size());++army)
                if(army!=5 && world.armies[army].troops>0 && world.armies[army].tile==Campaign::At(8,8) && world.armies[army].faction==0)
                    return Campaign::At(7,8);
            return Campaign::At(9,8);
        }
        if(grand.ralliedOn)
        {
            const auto* commander=intelligence[0].Find(GrandCommander);
            return commander && commander->current ? commander->observed.tile:Campaign::At(13,6);
        }
        if(grand.openedOn) return GrandRallyTile();
        return grand.relievedOn || grand.approach==GrandApproach::CentralAssault ? goal:Campaign::At(7,8);
    }
    bool StrategistJourney::GrandHeadquartersBroken() const
    {
        if(!HasGrandBattle()) return false;
        const auto& commander=world.armies[GrandCommander];
        return commander.troops<=0 || (HasGrandStrategy() && commander.morale<=10 && world.MapDistance(commander.tile,Campaign::At(13,6))>=2);
    }
    void AdvanceGrandForcedMarch(StrategistJourney& journey)
    {
        auto& grand=journey.battleReview.grand;auto& world=journey.world;
        if(!grand.ordersEnabled || grand.approach!=GrandApproach::CentralAssault || grand.openedOn || !TacticChain::Living(world,5)) return;
        auto& main=world.armies[5];
        if(main.food<100 || main.morale<40 || journey.chain.disordered[5]) return;
        auto observed=std::make_unique<Campaign>(world);
        for(auto& army:observed->armies) if(army.faction!=world.player) army.troops=0;
        for(const auto& contact:journey.intelligence[0].contacts)
            if(contact.current)
            {
                observed->armies[contact.id]=contact.observed;
                if(contact.observed.troops>0 && contact.observed.morale>=25 && world.MapDistance(contact.observed.tile,main.tile)<=1) return;
            }
        const int target=journey.GrandMainTarget();
        const int next=JourneyAdvanceStep(*observed,main.tile,target,target==Campaign::At(7,8) || target==Campaign::At(9,8));
        if(next==main.tile || !TacticChain::Free(world,next)) return;
        main.tile=main.target=next;main.food-=100;main.morale-=3;
        ++grand.forcedSteps;++world.revision;
    }
}
