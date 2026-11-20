#include "StrategistJourney.hpp"
#include "JourneyEpisode.hpp"
#include <memory>

namespace frontline
{
    void StrategistJourney::PrepareGrandBattle()
    {
        if(!PlansGrandBattle()) return;
        if(HasCoalitionBattle()) { PrepareCoalitionBattle();return; }
        auto& review=battleReview.grand;
        review.active=true;
        auto Add=[&](int faction,const std::u32string& name,int tile,int troops,int leadership)
        {
            General general;
            general.name=name;general.faction=faction;general.home=faction;general.leadership=leadership;
            const int id=int(world.generals.size());world.generals.push_back(general);
            Army army;
            army.general=id;army.faction=faction;army.tile=army.target=tile;
            army.troops=troops;army.morale=85;army.food=12000;
            world.armies.push_back(army);
        };
        if(!HasSunJian()) Add(0,EpisodeSpeakerName(HuangfuSong),Campaign::At(3,6),7000,90);
        else world.armies[5].troops=7000;
        world.generals[world.armies[5].general].trait=Trait::Inspiring;
        world.generals[world.armies[5].general].tactic=Tactic::Rally;
        Add(0,HasSunJian() ? U"孫堅軍・左翼":U"官軍・左翼",Campaign::At(5,3),4000,75);
        Add(0,HasSunJian() ? U"孫堅軍・右翼":U"官軍・右翼",Campaign::At(5,12),4000,75);
        Add(1,HasSunJian() ? U"董卓軍・北陣":U"黄巾軍・北陣",Campaign::At(10,3),3200,72);
        Add(1,HasSunJian() ? U"董卓軍・南陣":U"黄巾軍・南陣",Campaign::At(13,12),3200,72);
        Add(1,HasSunJian() ? U"胡軫・本陣":U"黄巾軍・本陣",Campaign::At(13,6),6000,82);
        world.armies[GrandCommander].stance=battle::Stance::Guard;
        if(HasSunJian() && yangrenStrategyCampaign)
        {
            review.yangren.active=true;
            world.armies[6].tile=world.armies[6].target=Campaign::At(7,7);
            world.armies[7].tile=world.armies[7].target=Campaign::At(9,10);
            auto& reserve=world.armies[8];reserve.tile=reserve.target=Campaign::At(13,4);
            reserve.arm=Arm::Cavalry;reserve.troops=2800*CombatSettings(difficulty).enemyPercent/100;
            reserve.stance=battle::Stance::Guard;
            world.generals[8].name=U"呂布の親衛騎兵";world.generals[8].leadership=90;
            world.generals[8].trait=Trait::Valiant;world.generals[8].tactic=Tactic::MountedCharge;
        }
        if(mission==YellowTurbanMission && grandStrategyCampaign)
        {
            review.ordersEnabled=true;
            battleReview.guangzong={}; // This layout's side objective is the northern relief.
            world.armies[5].tile=world.armies[5].target=Campaign::At(4,6);
            world.armies[6].tile=world.armies[6].target=GrandLeftAnchor;
            world.armies[6].troops=1800;world.armies[6].morale=65;world.armies[6].stance=battle::Stance::Guard;
            world.armies[3].troops=1000*CombatSettings(difficulty).enemyPercent/100;
            world.armies[4].troops=900*CombatSettings(difficulty).enemyPercent/100;
            world.armies[GrandCommander].troops=4200*CombatSettings(difficulty).enemyPercent/100;
            world.armies[GrandCommander].stance=battle::Stance::Balanced;
        }
        for(int army=5;army<GrandBattleArmyCount;++army)
            if(world.armies[army].faction==0) review.alliedStarted+=world.armies[army].troops;
    }

    std::vector<JourneyEnemyPlan> StrategistJourney::PlanGrandAllies() const
    {
        std::vector<JourneyEnemyPlan> plans;
        if(!HasGrandBattle()) return plans;
        if(HasCoalitionBattle()) return PlanCoalitionAllies();
        auto observed=std::make_unique<Campaign>(world);
        for(auto& army:observed->armies) if(army.faction!=world.player) army.troops=0;
        for(const auto& contact:intelligence[0].contacts)
            if(contact.current) observed->armies[contact.id]=contact.observed;
        for(int army=5;army<8;++army)
        {
            if((army==5 && HasSunJian() && !battleReview.grand.ralliedOn) || !TacticChain::Living(world,army) || chain.disordered[army]) continue;
            JourneyEnemyPlan plan;
            plan.army=army;plan.from=plan.next=world.armies[army].tile;
            const bool main=army==5;
            int target=main ? (battleReview.grand.openedOn ? GrandRallyTile():Campaign::At(8,6)):
                army==6 ? Campaign::At(10,3):Campaign::At(13,12);
            if(main && battleReview.grand.ralliedOn)
            {
                const auto* commander=intelligence[0].Find(GrandCommander);
                target=commander && commander->current ? commander->observed.tile:Campaign::At(13,6);
            }
            bool occupy=main && !battleReview.grand.openedOn;
            if(HasYangrenStrategy())
            {
                target=army==6 ? Campaign::At(8,7):YangrenRally;
                occupy=false;
                if(main)
                {
                    const auto* first=intelligence[0].Find(3);
                    target=!battleReview.grand.openedOn && first && first->current && first->observed.troops>0 ? first->observed.tile:YangrenRally;
                }
            }
            if(battleReview.grand.ordersEnabled)
            {
                const auto& grand=battleReview.grand;
                if(army==6 && !grand.relievedOn)
                { target=GrandLeftAnchor;occupy=true; }
                else if(!main && grand.ralliedOn)
                { target=GrandMainTarget();occupy=false; }
                if(main)
                {
                    target=GrandMainTarget();
                    occupy=target==Campaign::At(7,8) || target==Campaign::At(9,8);
                }
            }
            int nearest=100000;
            for(const auto& contact:intelligence[0].contacts)
            {
                if(!contact.current || contact.observed.troops<=0 ||
                    ((battleReview.grand.ordersEnabled || HasYangrenStrategy()) && contact.observed.morale<25)) continue;
                const int distance=world.MapDistance(plan.from,contact.observed.tile);
                if(battleReview.grand.ordersEnabled && !main && distance>(battleReview.grand.ralliedOn ? 1:3)) continue;
                const int priority=distance+(contact.id==army+2 ? -3:0);
                if(HasYangrenStrategy() && distance>2) continue;
                if(((!main && !(army==6 && battleReview.grand.ordersEnabled && !battleReview.grand.relievedOn)) || distance<=1) && priority<nearest)
                { nearest=priority;target=contact.observed.tile;occupy=false; }
            }
            plan.target=target;
            plan.next=JourneyAdvanceStep(*observed,plan.from,target,occupy);
            plans.push_back(plan);
            observed->armies[army].tile=plan.next;
        }
        return plans;
    }

    void StrategistJourney::CheckGrandBattle()
    {
        if(HasCoalitionBattle()) { CheckCoalitionBattle();return; }
        auto& review=battleReview.grand;
        CheckGrandRelief();
        if(!TacticChain::Living(world,AlliedArmy))
        { Finish(false,U"味方本隊が崩れ、全軍が退く。軍師の旗は残ったが、会戦を支えきれなかった。");return; }
        if(!review.openedOn && PositionSecured(goal))
            for(int officer=0;officer<3;++officer)
                if(TacticChain::Living(world,officer) && world.armies[officer].tile==goal)
                {
                    review.openedBy=officer;review.openedOn=turn;
                    if(battleReview.guangzong.tracked) battleReview.guangzong.swift=mission==YellowTurbanMission && GuangzongCommandsLeft()>0;
                    world.tiles[goal].owner=world.player;++world.revision;
                    Report(world.generals[officer].name+(HasYangrenStrategy() ?
                        std::u32string(U"の旗が辻に立った。孫堅本隊と旗を合わせ、騎兵の反撃に備えよう。"):
                        std::u32string(U"の旗が前陣に立った。本隊が攻勢へ！ 両翼と合流し、敵の本陣を落とそう。")));
                    break;
                }
        if((review.openedOn || HasYangrenStrategy()) && !review.ralliedOn && world.MapDistance(world.armies[AlliedArmy].tile,GrandRallyTile())<=1)
        {
            review.ralliedOn=turn;++world.revision;
            Report(HasYangrenStrategy() ? U"孫堅「合流したぞ！ ここからは一緒だ。騎兵が戻ってきても、互いの旗を離すな！」":
                HasSunJian() ? U"孫堅「合流したぞ！ ここからは俺たちも一緒だ。本陣まで突っ切る！」":
                U"皇甫嵩本隊が前陣を通過。両翼へ総攻撃の太鼓が渡り、敵本陣へ進み始めた。");
        }
        if(HasYangrenStrategy())
        {
            CheckYangrenCounter();
            if(YangrenCounterRepelled() && PositionSecured(goal))
                Finish(true,U"孫堅本隊と合流し、呂布の騎兵反撃を退けた。董卓軍は攻勢を失い、陽人の会戦は勝利に終わった。");
            return;
        }
        if(review.openedOn && review.ralliedOn && GrandHeadquartersBroken() && PositionSecured(goal))
            Finish(true,HasSunJian() ? U"孫堅本隊と両翼が敵本陣へ進出。董卓軍の戦列を崩し、陽人の会戦に勝利した。":
                U"前陣の突破から官軍全軍が進出。黄巾軍の本陣を落とし、広宗の会戦に勝利した。");
    }
}
