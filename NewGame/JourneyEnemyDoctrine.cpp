#include "JourneyEnemyDoctrine.hpp"
namespace frontline
{
    namespace
    {
        bool HostileCombat(const Campaign& w,int side,const Army& a)
        { return a.troops>0 && a.arm!=Arm::Transport && w.Hostile(side,a.faction); }
        int Nearest(const Campaign& w,int army)
        {
            int result=-1,best=100000;
            for(int i=0;i<int(w.armies.size());++i)
                if(HostileCombat(w,w.armies[army].faction,w.armies[i]))
                {
                    const int d=w.MapDistance(w.armies[army].tile,w.armies[i].tile);
                    if(d<best) { best=d;result=i; }
                }
            return result;
        }
        bool FrontPressed(const Campaign& w)
        {
            // Do not interpret a redacted/absent vanguard as a defeated one.
            return w.armies.size()>3 && w.armies[3].troops>0 &&
                (w.armies[3].troops<=900 || w.armies[3].morale<=60);
        }
        int Defense(const Campaign& w,int side,int goal)
        {
            int score=0;
            for(const auto& a:w.armies)
                if(HostileCombat(w,side,a) && w.MapDistance(a.tile,goal)<=1)
                    score+=a.troops*std::max(0,a.morale)/100*(a.stance==battle::Stance::Guard ? 4:3)/3;
            return score;
        }
        void Target(const Campaign& w,JourneyEnemyPlan& p,int army,JourneyEnemyIntent intent)
        { p.targetArmy=army;p.target=army>=0 ? w.armies[army].tile:-1;p.intent=intent; }
    }
    void ApplyJourneyEnemyDoctrine(const Campaign& w,const JourneyEnemyContext& c,JourneyEnemyPlan& p)
    {
        if(!c.campaign || p.army<3 || p.army>=int(w.armies.size())) return;
        const auto& unit=w.armies[p.army];
        const int nearest=Nearest(w,p.army);
        const int distance=nearest<0 ? 100000:w.MapDistance(unit.tile,w.armies[nearest].tile);
        if(c.objective==JourneyObjective::Breakout || c.objective==JourneyObjective::Escort)
        {
            p.role=p.army==3 ? JourneyEnemyRole::Vanguard:JourneyEnemyRole::Reserve;
            // Scouts retain their proactive search and convoy interception.
            if(p.army!=4 || unit.scouting || p.intent==JourneyEnemyIntent::Intercept) return;
            if(distance<=2 || (nearest>=0 && FrontPressed(w))) Target(w,p,nearest,JourneyEnemyIntent::Counterattack);
            else Target(w,p,-1,JourneyEnemyIntent::Reserve);
        }
        else if(c.objective==JourneyObjective::CaptureCamp)
        {
            if(p.army==3)
            {
                p.role=JourneyEnemyRole::Anchor;
                if(distance>1)
                { p.targetArmy=-1;p.target=c.goal;p.intent=JourneyEnemyIntent::HoldCamp;p.occupyTarget=true; }
            }
            else
            {
                p.role=JourneyEnemyRole::Counterattack;
                int intruder=-1,best=100000;
                for(int i=0;i<int(w.armies.size());++i)
                    if(HostileCombat(w,unit.faction,w.armies[i]) && w.armies[i].tile%Width>=11)
                    {
                        const int score=w.MapDistance(w.armies[i].tile,c.goal)*100+w.MapDistance(unit.tile,w.armies[i].tile);
                        if(score<best) { best=score;intruder=i; }
                    }
                if(distance<=1) intruder=nearest;
                if(intruder<0 && nearest>=0 && FrontPressed(w)) intruder=nearest;
                Target(w,p,intruder,intruder<0 ? JourneyEnemyIntent::Reserve:JourneyEnemyIntent::Counterattack);
            }
        }
        else if(c.objective==JourneyObjective::HoldPositions)
        {
            if(p.army==3)
            { p.role=JourneyEnemyRole::Fixing;p.intent=JourneyEnemyIntent::Fixing;p.targetArmy=-1;p.target=c.secondGoal; }
            else
            {
                p.role=JourneyEnemyRole::MainAttack;p.intent=JourneyEnemyIntent::MainAttack;p.targetArmy=-1;
                const int north=Defense(w,unit.faction,c.goal),south=Defense(w,unit.faction,c.secondGoal);
                p.target=south<north ? c.secondGoal:c.goal;
            }
            // A blocking screen must be fought, not passed through to a weak wing.
            if(distance<=1) Target(w,p,nearest,JourneyEnemyIntent::Engage);
        }
    }
    std::u32string EnemyDoctrineHint(const JourneyEnemyPlan& p,const JourneyEnemyContext& c)
    {
        if(!c.campaign || p.intent==JourneyEnemyIntent::Recover) return {};
        if(p.intent==JourneyEnemyIntent::Reserve)
            return p.role==JourneyEnemyRole::Counterattack ? U"反撃隊は渡河を待つ見込み。\n突入役と、迎え撃つ役を分けよう。":U"後詰はまだ待機する見込み。\n先鋒を崩す時は、その次にも備えよう。";
        if(p.role==JourneyEnemyRole::Anchor)
            return U"指揮所を離れにくい守備隊。\n崩して、旗を立てる隊を通そう。";
        if(p.intent==JourneyEnemyIntent::Counterattack)
            return p.role==JourneyEnemyRole::Counterattack ? U"橋を渡る部隊へ反撃する見込み。\n橋の出口で孤立しないように。":U"後詰が戦列へ向かう見込み。\n足を止め、先鋒と分断する手もある。";
        if(p.role==JourneyEnemyRole::Fixing)
            return U"一翼の守備を引きつける隊。\n追いすぎず、もう一翼も見よう。";
        if(p.role==JourneyEnemyRole::MainAttack)
            return p.intent==JourneyEnemyIntent::Engage ? U"主攻は目の前の部隊と交戦中。\n受け止める間に、両翼を整えよう。":
                p.target==c.goal ? U"手薄と見た北の烽火へ進む見込み。\n移動を妨げ、守備を間に合わせよう。":U"手薄と見た南の烽火へ進む見込み。\n移動を妨げ、守備を間に合わせよう。";
        return {};
    }
}
