#include "StrategistJourney.hpp"
#include "JourneyStrategicResponse.hpp"
#include <memory>

namespace frontline
{
    bool CoalitionReturnReached(const Campaign& world,int army)
    {
        if(army<0 || army>=int(world.armies.size()) || world.armies[army].troops<=0) return false;
        const int tile=world.armies[army].tile;
        return tile%Width<=6 && world.MapDistance(tile,CoalitionRetreat)<=2;
    }
    void StrategistJourney::PrepareCoalitionBattle()
    {
        auto& g=battleReview.grand;g.active=g.coalition.active=true;
        auto Add=[&](int faction,const char32_t* name,int x,int y,int troops,int command)
        {
            General general;general.name=name;general.faction=general.home=faction;general.leadership=command;
            general.trait=faction==0 ? Trait::Inspiring:Trait::Resolute;general.tactic=faction==0 ? Tactic::Rally:Tactic::Charge;
            Army army;army.general=int(world.generals.size());army.faction=faction;
            army.tile=army.target=Campaign::At(x,y);army.troops=troops;army.morale=85;army.food=12000;
            world.generals.push_back(general);world.armies.push_back(army);
        };
        Add(0,U"曹操",10,5,5000,92);
        Add(0,U"張邈の援軍",9,9,2200,75);
        Add(0,U"連合軍・後陣",5,4,3200,78);
        if(GuangzongVeteranOfficer(*this)>=0)
        {
            world.generals[world.armies[7].general].name=U"広宗の義兵";
            // The earned survivors join the existing rear; saving them must
            // never replace a larger allied company with a weaker reward.
            world.armies[7].troops+=history[YellowTurbanMission].review.grand.leftReturned;
        }
        const int rate=CombatSettings(difficulty).enemyPercent;
        Add(1,U"徐栄軍・北岸",10,3,1600*rate/100,73);
        Add(1,U"徐栄軍・包囲隊",12,9,1800*rate/100,78);
        Add(1,U"徐栄・本陣",13,5,2600*rate/100,90);
        world.armies[6].morale=65;world.armies[6].stance=battle::Stance::Guard;
        world.armies[10].stance=battle::Stance::Guard;
        world.armies[3].troops=1000*rate/100;world.armies[4].troops=900*rate/100;
        world.generals[3].name=U"徐栄軍・先鋒";world.generals[4].name=U"徐栄軍・騎兵";
        world.armies[4].arm=Arm::Cavalry;world.generals[4].tactic=Tactic::MountedCharge;
        for(int a=5;a<8;++a) g.alliedStarted+=world.armies[a].troops;
    }

    std::vector<JourneyEnemyPlan> StrategistJourney::PlanCoalitionAllies() const
    {
        std::vector<JourneyEnemyPlan> plans;
        auto observed=std::make_unique<Campaign>(world);
        for(auto& a:observed->armies) if(a.faction!=0) a.troops=0;
        for(const auto& c:intelligence[0].contacts) if(c.current) observed->armies[c.id]=c.observed;
        const bool withdraw=coalitionPlan==CoalitionPlan::Protect;
        const auto& g=battleReview.grand;
        for(int a=5;a<8;++a)
        {
            if(!TacticChain::Living(world,a) || chain.disordered[a]) continue;
            JourneyEnemyPlan p;p.army=a;p.from=p.next=world.armies[a].tile;
            int target=p.from;
            if(a==5)
            {
                if(withdraw && g.coalition.rescuedOn)
                    target=world.armies[6].tile%Width>7 ? Campaign::At(10,7):
                        p.from%Width>7 ? CoalitionBridge:Campaign::At(6,9);
                else if(!withdraw && g.openedOn)
                    target=Campaign::At(13,4); // Cao Cao takes the north flank; heroes keep the southern approach clear.
            }
            if(a==6 && g.coalition.rescuedOn) target=withdraw ? (p.from%Width>7 ? CoalitionBridge:Campaign::At(5,10)):Campaign::At(10,8);
            if(a==7) target=withdraw ? Campaign::At(4,8):Campaign::At(9,4);
            if(a==7 && HasGuangzongVeterans(*this))
            {
                const int officer=GuangzongVeteranOfficer(*this);
                const auto& hero=world.armies[officer];
                if(TacticChain::Living(world,officer))
                    target=world.MapDistance(p.from,hero.tile)>2 ? hero.tile:p.from;
                // Keep the crossing and the victory flag free, even while covering a friend.
                if(p.from==goal) target=Campaign::At(5,4);
                else if(world.tiles[p.from].terrain==Terrain::Bridge)
                    target=hero.tile%Width>7 ? Campaign::At(9,4):Campaign::At(5,4);
                int distance=2;
                for(const auto& c:intelligence[0].contacts)
                    if(c.current && c.observed.troops>0 && world.MapDistance(p.from,c.observed.tile)<distance)
                    { distance=world.MapDistance(p.from,c.observed.tile);target=c.observed.tile; }
                // Once the relief returns, heroes need the bridge approaches
                // as yield cells. Rejoin the western rear instead of following
                // a rescuer onto those cells and pinning the whole column.
                if(withdraw && g.coalition.rescuedOn) target=Campaign::At(4,8);
            }
            if((!withdraw || !g.coalition.rescuedOn) && !(a==5 && !withdraw && g.openedOn))
            {
                int best=2;
                for(const auto& c:intelligence[0].contacts) if(c.current && c.observed.troops>0 && c.observed.morale>25)
                {
                    const int distance=world.MapDistance(p.from,c.observed.tile);
                    if(distance<best) { best=distance;target=c.observed.tile; }
                }
            }
            const bool occupy=((withdraw && g.coalition.rescuedOn) || (a==5 && !withdraw && g.openedOn)) && !(a==7 && HasGuangzongVeterans(*this));
            p.occupyTarget=occupy;
            p.target=target;p.next=JourneyAdvanceStep(*observed,p.from,target,occupy);
            plans.push_back(p);observed->armies[a].tile=p.next;
        }
        return plans;
    }

    void StrategistJourney::ApplyCoalitionEnemyRoles(const Campaign& observed,std::vector<JourneyEnemyPlan>& plans) const
    {
        for(auto& p:plans)
        {
            if(p.army!=8 && p.army!=9) continue;
            if(p.intent==JourneyEnemyIntent::Recover) continue;
            const int ally=p.army==9 ? 6:5;
            if(p.targetArmy>=0 && observed.MapDistance(p.from,observed.armies[p.targetArmy].tile)<=1) continue;
            p.targetArmy=observed.armies[ally].troops>0 ? ally:-1;
            p.target=p.targetArmy>=0 ? observed.armies[ally].tile:CoalitionBridge;
            p.next=JourneyAdvanceStep(observed,p.from,p.target,false);
            p.intent=JourneyEnemyIntent::MainAttack;p.role=JourneyEnemyRole::MainAttack;
            auto next=std::make_unique<Campaign>(observed);next->armies[p.army].tile=p.next;p.danger.clear();
            for(int y=1;y<13;++y) for(int x=1;x<15;++x)
            {
                const int tile=Campaign::At(x,y);
                if(tile!=p.next && next->tiles[tile].terrain!=Terrain::Sea && next->CanStrike(p.army,tile)) p.danger.push_back(tile);
            }
        }
    }

    void StrategistJourney::CheckCoalitionBattle()
    {
        auto& g=battleReview.grand;auto& c=g.coalition;
        const bool withdraw=coalitionPlan==CoalitionPlan::Protect;
        if(!TacticChain::Living(world,5) || (withdraw && !TacticChain::Living(world,6)))
        { Finish(false,U"連合の旗が崩れた。合流できなかった兵を残し、軍師たちは汴水から退いた。");return; }
        const auto* southern=intelligence[0].Find(9);
        const bool broken=!TacticChain::Living(world,9) || (southern && southern->current && southern->observed.morale<=25);
        if(!c.rescuedOn && broken && TacticChain::Living(world,6))
            for(int a=0;a<3;++a) if(TacticChain::Living(world,a) && !chain.disordered[a] && world.MapDistance(world.armies[a].tile,world.armies[6].tile)<=1)
            {
                c.rescuedBy=a;c.rescuedOn=turn;++world.revision;
                Report(world.generals[a].name+U"隊が張邈の援軍へ到着。南の包囲が開いた。曹操「その旗が見えた！ ここからは一緒に動くぞ！」");break;
            }
        if(withdraw)
        {
            if(c.rescuedOn && CoalitionReturnReached(world,5) && CoalitionReturnReached(world,6) && PositionSecured(goal))
            { c.returnedOn=turn;Finish(true,U"曹操と張邈の援軍が汴水の西岸へ戻った。徐栄の本陣を追わず、連合の旗と兵を守って会戦を終えた。"); }
            return;
        }
        if(!g.openedOn && PositionSecured(goal))
            for(int a=0;a<3;++a) if(TacticChain::Living(world,a) && world.armies[a].tile==goal)
            {
                g.openedBy=a;g.openedOn=turn;++world.revision;
                Report(U"対岸に義勇軍の旗！ 曹操「待っていたぞ、軍師。孤軍ではないと、徐栄に見せてやろう！」");break;
            }
        if(g.openedOn && !g.ralliedOn && world.MapDistance(world.armies[5].tile,goal)<=1) g.ralliedOn=turn;
        const auto* h=intelligence[0].Find(GrandCommander);
        if(g.openedOn && g.ralliedOn && PositionSecured(goal) && (!TacticChain::Living(world,GrandCommander) || (h && h->current && h->observed.morale<=25)))
            Finish(true,U"曹操と義勇軍の旗が対岸に並び、徐栄軍の本陣が崩れた。史実とは異なる汴水の勝利を、軍師の采配がつかんだ。");
    }
}
