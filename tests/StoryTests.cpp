#include "../NewGame/HeroStory.hpp"
#include <cassert>
#include <iostream>
void Play(hero::Story& s) {
 while(s.phase==2&&!s.failed&&s.turn<=s.TurnLimit()){
  if(s.battleEvent)assert(s.ResolveBattleEvent(0));
  for(int i=0;i<static_cast<int>(s.units.size())&&s.phase==2;++i){
   const auto u=s.units[i];if(u.hero<0||u.hp<=0||u.acted)continue;
   const int objective=s.ObjectiveTile();
   if(s.deepRules && objective>=0 && s.objectiveProgress<2 && u.hero==2) {
    if(u.x==objective%hero::W && u.y==objective/hero::W) {s.Guard(i);continue;}
    int next=s.StepToward(i,objective%hero::W,objective/hero::W);if(next>=0)s.Act(i,next%hero::W,next/hero::W);
   }
   const auto current=s.units[i];
   if((u.hero==0||u.hero==3||u.hero==2)&&s.Skill(i))continue;
   int target=-1,distance=100;
   for(int j=0;j<static_cast<int>(s.units.size());++j)if(s.units[j].enemy&&s.units[j].hp>0&&hero::Story::Distance(current,s.units[j])<distance){target=j;distance=hero::Story::Distance(current,s.units[j]);}
   if(target>=0){auto t=s.units[target];if(s.CanAttack(i,target)){if(!s.Skill(i))s.Act(i,t.x,t.y);}else{int next=s.StepToward(i,t.x,t.y);if(next>=0)s.Act(i,next%hero::W,next/hero::W);}}
   else if(s.chapter==3){const int px[]={6,7,8,9,9},py[]={6,0,0,5,6};int next=s.StepToward(i,px[u.hero],py[u.hero]);if(next>=0)s.Act(i,next%hero::W,next/hero::W);}
  }
  if(s.chapter==4)s.FireSignal();s.EndTurn();
 }
}
int main(){
 hero::Story s;s.Reset();assert(!s.Choose(2));
 for(int chapter=0;chapter<6;++chapter){
  assert(s.chapter==chapter&&s.phase==0);assert(s.Choose(0)&&s.Choose(0));
  if(chapter==1){assert(s.CivilAction(0)&&s.CivilAction(1)&&s.CivilAction(0));}
  else if(chapter==2){for(int i=0;i<3;++i){if(s.food<20)s.CivilAction(1);assert(s.CivilAction(0));if(i<2)assert(s.CivilAction(2));}}
  else Play(s);
  std::cerr<<"Chapter "<<chapter<<" turn "<<s.turn<<" phase "<<s.phase<<" escaped "<<s.escaped<<" lost "<<s.lost<<"\n";
  if(s.failed)for(const auto& u:s.units)std::cerr<<u.hero<<" civ "<<u.civilian<<" hp "<<u.hp<<" @"<<u.x<<","<<u.y<<"\n";assert(s.phase==3&&!s.failed);assert(s.Choose(0));
 }
 assert(s.phase==4&&s.decisions.size()==18&&s.virtue>=80);
 hero::Story failed;failed.Choose(1);failed.Choose(1);failed.units[0].hp=0;failed.CheckBattle();assert(failed.failed);failed.Retry();assert(!failed.failed&&failed.units[0].hp>0);
 hero::Story actions;actions.Choose(0);actions.Choose(0);assert(!actions.Act(0,10,6));assert(actions.Act(0,0,1));assert(!actions.Act(0,0,2));actions.EndTurn();assert(!actions.units[0].acted);
 hero::Story crisis;crisis.chapter=3;crisis.phase=2;crisis.StartMission();
 crisis.EndTurn();crisis.EndTurn();crisis.EndTurn();assert(crisis.battleEvent==1&&crisis.eventMask==1);
 const int crisisTurn=crisis.turn;crisis.EndTurn();assert(crisis.turn==crisisTurn);assert(!crisis.Act(0,0,1)&&!crisis.Skill(0)&&!crisis.Rally());
 assert(!crisis.ResolveBattleEvent(2));assert(crisis.ResolveBattleEvent(0)&&crisis.battleDecisions.back()==30&&crisis.spirit>=45);
 for(const auto& u:crisis.units)if(u.enemy&&u.hp>0)assert(u.stunned==1);
 crisis.EndTurn();assert(crisis.battleEvent==0);crisis.spirit=59;assert(!crisis.Rally());
 crisis.spirit=60;crisis.units[0].acted=true;crisis.units[0].hp-=4;const int hurt=crisis.units[0].hp;
 crisis.units[1].hp=0;assert(crisis.Rally());assert(crisis.spirit==0&&crisis.rallies==1&&!crisis.units[0].acted&&crisis.units[0].hp==hurt+2&&crisis.units[1].hp==0);
 hero::Story rescue;rescue.chapter=3;rescue.phase=2;rescue.StartMission();rescue.battleEvent=rescue.eventMask=1;rescue.units.back().hp=1;assert(rescue.ResolveBattleEvent(1)&&rescue.units.back().hp==5&&rescue.battleDecisions.back()==31);
 std::cout<<"Story progression, combat, escort and retry tests passed\n";
 // A bond with Liu Bei does not automatically create a bond between two others.
 hero::Story pair;pair.chapter=3;pair.phase=2;pair.StartMission();pair.bonds[1]=pair.bonds[3]=100;
 pair.units[0].x=10;pair.units[0].y=6;pair.units[1].x=1;pair.units[1].y=1;pair.units[3].x=2;pair.units[3].y=1;pair.units[2].hp=pair.units[4].hp=0;
 assert(pair.Affinity(1,3)==20&&pair.Formation(1).defense==0);
 pair.ChangeBond(1,3,60);assert(pair.Affinity(1,3)==80&&pair.Affinity(3,1)==80&&officer::StoryDefense(pair.Formation(1))==1);
 pair.units[3].hp=0;assert(pair.Formation(1).defense==0);
 hero::Story support;assert(!support.AssignPlanner(3));support.Choose(0);support.chapter=3;assert(support.AssignPlanner(3));hero::Story warrior=support;assert(warrior.AssignPlanner(1));
 support.Choose(0);warrior.Choose(0);assert(support.units[0].maxHp>warrior.units[0].maxHp&&support.units[3].acted&&!support.units[0].acted);support.EndTurn();assert(!support.units[3].acted);
 hero::Story training;training.chapter=3;training.phase=1;training.AssignPlanner(1);hero::Story guardTraining=training;guardTraining.AssignPlanner(3);training.Choose(1);guardTraining.Choose(1);assert(training.units[0].attack>guardTraining.units[0].attack);
 std::cout<<"Shared role, pair relationship and preparation tradeoff tests passed\n";

 hero::Story vow;vow.Choose(0);vow.Choose(0);
 for(auto& u:vow.units)if(u.enemy)u.hp=0;
 vow.CheckBattle();assert(vow.phase==2);
 const int tile=vow.ObjectiveTile();vow.units[2].x=tile%hero::W;vow.units[2].y=tile/hero::W;
 vow.EndTurn();assert(vow.objectiveProgress==1);vow.EndTurn();assert(vow.phase==3 && vow.outcomes[0]==2);
 assert(vow.Choose(0)&&vow.Choose(0)&&vow.Choose(0) && vow.chapter==1 && vow.order==58);
 hero::Story force;force.Choose(1);force.Choose(1);for(auto& u:force.units)if(u.enemy)u.hp=0;force.CheckBattle();assert(force.phase==3 && force.outcomes[0]==1);
 hero::Story position;position.phase=2;position.StartMission();
 position.units[0].x=4;position.units[0].y=5;position.AdvanceObjective();assert(position.objectiveProgress==1);
 position.units[0].x=3;position.AdvanceObjective();assert(position.objectiveProgress==0);
 position.units[0].x=4;position.units[3].x=5;position.units[3].y=5;position.AdvanceObjective();assert(position.objectiveProgress==0);
 hero::Story duel;duel.phase=2;duel.StartMission();duel.units.resize(2);
 duel.units[0].acted=false;duel.units[0].x=4;duel.units[0].y=3;
 duel.units[1].hero=-1;duel.units[1].enemy=true;duel.units[1].x=5;duel.units[1].y=3;duel.units[1].attack=4;duel.units[1].hp=duel.units[1].maxHp=12;
 hero::Story open=duel;const int hp=duel.units[0].hp;
 assert(duel.Guard(0) && !duel.Guard(0));duel.EndTurn();open.EndTurn();
 assert(duel.units[0].hp>open.units[0].hp && duel.units[1].hp==11 && !duel.units[0].guarding);
 hero::Story advancing=duel;advancing.units[0].x=3;advancing.units[0].y=3;advancing.units[1].x=5;
 assert(advancing.Act(0,4,3) && advancing.units[0].moved && !advancing.units[0].acted);
 assert(advancing.Act(0,5,3) && advancing.units[0].acted);
 hero::Story interview;interview.chapter=2;interview.phase=2;interview.StartMission();
 assert(interview.CivilAction(0) && !interview.CivilAction(0) && interview.CivilAction(2) && interview.CivilAction(0));
 hero::Story shots=duel;shots.units[0].range=3;shots.units[0].x=3;shots.units[1].x=6;shots.terrain[3*hero::W+4]=1;
 assert(!shots.CanAttack(0,1));shots.terrain[3*hero::W+4]=0;assert(shots.CanAttack(0,1));
 std::cout<<"Story vows, consequences, position, guard and move-attack passed"<<std::endl;
}
