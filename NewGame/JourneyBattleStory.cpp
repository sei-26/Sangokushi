#include "StrategistJourney.hpp"
namespace frontline
{
    namespace
    {
        std::u32string Number(int value) { const auto s=std::to_string(value);return {s.begin(),s.end()}; }
        int Count(int mask) { return (mask&1)+((mask>>1)&1)+((mask>>2)&1); }
    }
    const char32_t* PledgeName(JourneyPledge pledge)
    {
        if(pledge==JourneyPledge::Preserve) return U"出陣兵の損失を一割以内に抑えて勝つ";
        if(pledge==JourneyPledge::Coordinate) return U"二隊以上の戦法をつなぎ、任務を果たす";
        return U"次の戦いの約束はまだ交わしていない";
    }
    const char32_t* BattleStoryName(const JourneyBattleStory& story,bool won,bool withdrawn)
    {
        if(!story.recorded) return U"これまでの戦いの記録";
        if(withdrawn) return U"引くと決めた軍師";
        if(!won) return U"果たせなかった約束";
        if(story.pledge!=JourneyPledge::None && !story.pledgeKept) return U"勝利の陰に残した言葉";
        if(story.costly) return U"勝った。それでも戻らない兵がいる";
        if(story.protectedPeople) return U"最後の一台まで、帰る道を";
        if(story.pledgeKept) return U"言葉を、戦場で果たした";
        if(story.coordinatedOfficers) return U"軍師の合図が、仲間をつなぐ";
        return story.lowLoss ? U"兵を連れ帰った軍師":U"次の一策へ残したもの";
    }
    int StrategistJourney::StoryMoraleBonus(int officer) const
    { return officer>=0 && officer<3 ? std::clamp(officerTrust[officer]/2,-5,8):0; }
    JourneyPledge StrategistJourney::CurrentPledge() const
    { return mission>0 && history[mission-1].story.recorded ? history[mission-1].story.reply:JourneyPledge::None; }
    std::u32string StrategistJourney::PledgeProgress() const
    {
        const auto pledge=CurrentPledge();if(pledge==JourneyPledge::None) return {};
        if(pledge==JourneyPledge::Preserve)
        {
            int returned=0;for(const auto& unit:world.armies) if(unit.faction==world.player) returned+=unit.troops;
            const int initial=deployed+convoyEscort;
            return U"主人公の約束：兵の損失 "+Number(std::max(0,initial-returned))+U" / "+Number(initial/10)+U"以内で勝利";
        }
        return U"主人公の約束：二隊連鎖 "+std::u32string(Count(battleReview.coordinatedOfficers)>=2 ? U"成立済み":U"未成立")+U"・任務達成まで";
    }
    bool StrategistJourney::NeedsStoryReply() const
    { return stage==JourneyStage::Debrief && won && mission<2 && history[mission].story.recorded && history[mission].story.reply==JourneyPledge::None; }
    bool StrategistJourney::CanContinue() const
    { return stage==JourneyStage::Debrief && won && !NeedsStoryReply(); }
    bool StrategistJourney::ChooseStoryReply(JourneyPledge pledge)
    {
        if(!NeedsStoryReply() || (pledge!=JourneyPledge::Preserve && pledge!=JourneyPledge::Coordinate)) return false;
        history[mission].story.reply=pledge;
        Report(pledge==JourneyPledge::Preserve ? U"主人公「次は、みんなを連れて帰る。俺がそう決めて戦う。」":U"主人公「次は一人で片づけさせない。俺が合図を出して、皆をつなぐ。」");
        return true; // Words alone earn no trust. The next battle settles this pledge.
    }
    void StrategistJourney::RecordCooperation(const ChainResult& result)
    {
        int mask=0,damage=0,steps=0;
        for(const auto& e:result.events)
        {
            if(!e.success) break;
            if(e.army>=0 && e.army<3) mask|=1<<e.army;
            damage+=e.damage;++steps;
        }
        if(steps>=2 && damage>0 && Count(mask)>=2) battleReview.coordinatedOfficers|=mask;
    }
    void StrategistJourney::CompleteBattleStory(bool success)
    {
        auto& story=history[mission].story;
        story={};story.recorded=true;story.pledge=CurrentPledge();
        const int initial=deployed+convoyEscort;
        story.loss=casualties;
        story.lowLoss=initial>0 && story.loss*10<=initial;
        story.costly=initial>0 && story.loss*4>=initial;
        story.protectedPeople=success && IsRescue();
        story.coordinatedOfficers=battleReview.coordinatedOfficers;
        story.pledgeKept=success && (story.pledge==JourneyPledge::Preserve ? story.lowLoss:
            story.pledge==JourneyPledge::Coordinate && Count(story.coordinatedOfficers)>=2);
        std::array<int,3> delta{};
        if(success)
        {
            delta={2,2,2};
            if(story.protectedPeople) { delta[0]+=4;delta[1]+=1;delta[2]+=1; }
            if(story.lowLoss) { delta[0]+=1;delta[1]+=4; }
            if(story.costly) { delta[0]-=2;delta[1]-=4;delta[2]-=2; }
            for(int i=0;i<3;++i) if(story.coordinatedOfficers&(1<<i)) delta[i]+=2;
            if(story.coordinatedOfficers&4) delta[2]+=2;
        }
        else if(story.costly) { delta[0]-=1;delta[1]-=2;delta[2]-=1; }
        if(story.pledge!=JourneyPledge::None)
        {
            if(story.pledgeKept && story.pledge==JourneyPledge::Preserve) { delta[0]+=2;delta[1]+=3;delta[2]+=1; }
            else if(story.pledgeKept)
            { for(int i=0;i<3;++i) if(story.coordinatedOfficers&(1<<i)) delta[i]+=2; }
            else if(story.pledge==JourneyPledge::Preserve) { delta[0]-=1;delta[1]-=3; }
            else delta[2]-=3;
        }
        for(int i=0;i<3;++i)
        {
            const int old=officerTrust[i];officerTrust[i]=std::clamp(old+delta[i],-20,100);
            story.trustDelta[i]=officerTrust[i]-old;
        }
    }
}
