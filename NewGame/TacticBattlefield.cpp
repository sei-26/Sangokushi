#include "TacticBattlefield.hpp"
namespace frontline
{
    TacticBattlefieldImpact ReadTacticBattlefield(const Campaign& w,const std::vector<bool>& concealed,int actor,int target)
    {
        TacticBattlefieldImpact p;
        if(actor<0 || target<0 || actor>=int(w.armies.size()) || target>=int(w.armies.size()) ||
            w.armies[actor].troops<=0 || w.armies[target].troops<=0) return p;
        const auto& victim=w.armies[target];
        const auto ground=w.tiles[victim.tile].terrain;
        p.terrain=ground==Terrain::Forest ? 80:ground==Terrain::Mountain ? 67:100;
        p.stance=battle::DamagePercent(victim.stance);
        std::vector<int> fronts;
        for(int id=0;id<int(w.armies.size());++id)
        {
            const auto& a=w.armies[id];
            if(a.faction!=w.armies[actor].faction || a.arm==Arm::Transport || a.troops<1200 || a.morale<30 || a.retreat ||
                w.MapDistance(a.tile,victim.tile)!=1 || (id!=actor && id<int(concealed.size()) && concealed[id])) continue;
            if(std::find(fronts.begin(),fronts.end(),a.tile)==fronts.end()) fronts.push_back(a.tile);
        }
        // Adjacent friendly fronts are the same side of the fight. At least two
        // separated sides are needed; piling two officers together is not a pincer.
        for(int cell:fronts)
            if(std::any_of(fronts.begin(),fronts.end(),[&](int other) { return w.MapDistance(cell,other)>=2; })) ++p.directions;
        p.directions=std::min(p.directions,3);return p;
    }
    std::u32string TacticBattlefieldImpact::Describe() const
    {
        std::u32string text;
        if(terrain==80) text+=U" 森林が被害を抑えた。";
        if(terrain==67) text+=U" 山地が被害を抑えた。";
        if(stance==75) text+=U" 固守が攻撃を受け止めた。";
        if(stance==120) text+=U" 攻勢の隙を突いた。";
        if(directions>=2) text+=directions>=3 ? U" 三方向から挟撃、威力が上がった！":U" 離れた二方向から挟撃、威力が上がった！";
        return text;
    }
    std::u32string TacticBattlefieldImpact::Hint() const
    {
        if(directions>=2) return terrain<100 || stance<100 ? U"挟撃の好機！\n敵の守りはまだ残る。":U"挟撃の好機！\n戦法をつないで押し切ろう。";
        if(stance<100) return U"敵は固守している。\n崩してから、大きな一撃を。";
        if(terrain<100) return U"地形に守られた敵だ。\n誘い出すか、回り込もう。";
        if(stance>100) return U"敵は攻勢で前のめり。\n大きな一撃を合わせよう。";
        return {};
    }
}
