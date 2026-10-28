#include "JourneyArchive.hpp"
#include "JourneyBattlefield.hpp"
#include <cmath>

namespace frontline
{
    namespace
    {
        bool Range(int v,int low,int high) { return v>=low && v<=high; }
        template<class T> bool Enum(T v,int high) { return Range(static_cast<int>(v),0,high); }
        bool Cell(int tile) { return StrategistJourney::OnMap(tile); }
        bool Order(const JourneyStandingOrder& o)
        {
            if(!Enum(o.kind,4) || !Enum(o.arrival,2) || o.waypoints.size()>3) return false;
            if(o.kind==JourneyDirective::March && !Cell(o.destination)) return false;
            if(o.kind==JourneyDirective::Attack && !Range(o.target,3,4)) return false;
            for(int p:o.waypoints) if(!Cell(p)) return false;
            return true;
        }
        bool ChainOrders(const std::vector<ChainOrder>& orders)
        {
            if(orders.size()>3) return false;
            std::array<bool,3> used{};
            for(const auto& o:orders)
            {
                if(!Range(o.army,0,2) || used[o.army] || !Enum(o.tactic,7) ||
                   !Range(o.enemy,-1,4) || (o.destination!=-1 && !Cell(o.destination))) return false;
                used[o.army]=true;
            }
            return true;
        }
        bool ArmyValid(const Army& a,int generals)
        {
            for(int cell:a.path) if(!Cell(cell)) return false;
            return Range(a.faction,0,1) && Range(a.general,0,generals-1) && Cell(a.tile) && Cell(a.target) &&
                Range(a.troops,0,100000) && Range(a.food,0,1000000) && Range(a.cargoFood,0,1000000) &&
                Range(a.morale,0,100) && Enum(a.arm,4) && Enum(a.stance,2) && a.path.size()<=224;
        }
        bool Review(const JourneyBattleReview& r)
        {
            return Enum(r.resolution,8) && Range(r.endedAt,0,100) && Range(r.enemyStops,0,100) &&
                Range(r.trafficStops,0,100) && Range(r.tacticAttempts,0,1000) &&
                Range(r.tacticSteps,0,3000) && Range(r.longestChain,0,3) && Range(r.coordinatedOfficers,0,7);
        }
        bool ChainRecord(const ChainResult& r)
        {
            if(r.prediction || r.events.size()>3) return false;
            int completed=0;
            for(std::size_t i=0;i<r.events.size();++i)
            {
                const auto& e=r.events[i];
                if(!Enum(e.tactic,7) || !Range(e.damage,0,100000) || !Range(e.combo,0,3) ||
                   (e.setupPercent!=100 && e.setupPercent!=125)) return false;
                if(e.success)
                {
                    ++completed;
                    if(e.combo!=0 && (e.combo!=completed || e.comboPercent!=TacticChain::ComboPercent(e.combo))) return false;
                    if(e.combo==0 && (e.comboPercent!=100 || e.setupPercent!=100)) return false;
                }
                else if(i+1!=r.events.size() || e.combo!=0 || e.damage!=0 || e.comboPercent!=100) return false;
            }
            return r.completed==completed;
        }
        bool Story(const JourneyBattleStory& s)
        {
            if(!Enum(s.pledge,2) || !Enum(s.reply,2) || !Range(s.coordinatedOfficers,0,7) || !Range(s.loss,0,100000)) return false;
            for(int d:s.trustDelta) if(!Range(d,-120,120)) return false;
            if(!s.recorded && (s.reply!=JourneyPledge::None || s.pledge!=JourneyPledge::None)) return false;
            return true;
        }
        bool Assignments(const std::array<JourneyAssignment,3>& assignments,int mission,bool rescue,bool preparation=false)
        {
            for(int i=0;i<3;++i)
            {
                const auto& a=assignments[i];if(!Enum(a.duty,2)) return false;
                if(a.duty==JourneyDuty::None) { if(a.tile!=-1 || a.completed) return false; }
                else if(!Cell(a.tile) || (a.duty==JourneyDuty::Ambush && i!=1)) return false;
                if(a.duty!=JourneyDuty::None)
                {
                    const auto terrain=BattlefieldTerrain(mission,a.tile%Width,a.tile/Width,rescue);
                    if(terrain==Terrain::Sea || terrain==Terrain::River || (a.duty==JourneyDuty::Ambush && terrain!=Terrain::Forest) ||
                       a.tile==Campaign::At(14,3) || a.tile==Campaign::At(2,mission==2 ? 6:10) || (preparation && a.completed)) return false;
                }
                for(int k=0;k<i;++k) if(a.duty!=JourneyDuty::None && assignments[k].duty!=JourneyDuty::None && a.tile==assignments[k].tile) return false;
            }
            return true;
        }
        bool Journey(const StrategistJourney& j)
        {
            const auto& w=j.world;const auto n=w.armies.size();
            if(!Enum(j.stage,4) || !Range(j.mission,0,2) || !Enum(j.difficulty,1) || !Range(j.turn,1,j.TurnBudget()+1) || !Range(j.enemyPolicy,0,2) ||
               !ChainRecord(j.lastChain) || !Enum(j.promise,1) || !Enum(j.convoyRoute,1) || !Cell(j.goal) || !Range(j.preparation,0,1) ||
               !Range(j.convoyStep,0,7) || !Range(j.heldTurns,0,12) || !Review(j.battleReview) || !Assignments(j.assignments,j.mission,j.IsRescue(),j.stage==JourneyStage::Preparation) ||
               w.cities.size()!=2 || w.generals.size()<5 || w.generals.size()>6 || n>6 ||
               ((j.stage==JourneyStage::Battle || j.stage==JourneyStage::Debrief) && n!=(j.HasConvoy()?6u:5u)) ||
               !w.hexMap || w.player!=0 || !Range(w.day,0,10000)) return false;
            for(const auto& c:w.cities) if(!Cell(c.tile) || !Range(c.owner,0,1) || !Range(c.troops,0,1000000) ||
                !Range(c.food,0,10000000) || !Range(c.gold,0,10000000)) return false;
            for(const auto& g:w.generals) if(!Range(g.home,0,1) || !Range(g.faction,0,1) ||
                !Enum(g.trait,15) || !Enum(g.tactic,8) || !Enum(g.specialty,3) ||
                !Range(g.leadership,0,100) || !Range(g.intelligence,0,100) || !Range(g.politics,0,100) || !Range(g.charm,0,100)) return false;
            for(const auto& a:w.armies) if(!ArmyValid(a,static_cast<int>(w.generals.size()))) return false;
            for(const auto& t:w.tiles) if(!Enum(t.terrain,5) || !Range(t.owner,-1,1)) return false;
            for(const auto& b:w.bonds) if(!Range(b.a,0,5) || !Range(b.b,0,5) || !Range(b.value,0,100)) return false;
            for(const auto* bits:{&j.chain.concealed,&j.chain.moved,&j.chain.spent,&j.chain.disordered,
                &j.chain.pressured,&j.chain.inspired,&j.chain.signatureUsed}) if(bits->size()!=n) return false;
            for(const auto& known:j.intelligence)
            {
                if(known.contacts.size()>6) return false;
                for(const auto& c:known.contacts) if(!Range(c.id,0,static_cast<int>(n)-1) ||
                    !ArmyValid(c.observed,static_cast<int>(w.generals.size()))) return false;
            }
            for(int t:j.officerTrust) if(!Range(t,-20,100)) return false;
            for(int m=0;m<3;++m)
            {
                const auto& h=j.history[m];
                // Mission 0 history lacks a failed escort's promise; both authored woodland variants are valid there.
                if(!Enum(h.difficulty,1) || !Assignments(h.assignments,m,m==0) || !Review(h.review) || !Enum(h.route,1) || !Story(h.story) ||
                   (h.story.recorded && !h.completed) || (!h.won && h.story.reply!=JourneyPledge::None)) return false;
            }
            return true;
        }
    }
    bool JourneyArchive::SessionValid(const JourneySaveData& s)
    {
        if(!Journey(s.journey) || (s.beforeBattle && (!Journey(*s.beforeBattle) || s.beforeBattle->stage!=JourneyStage::Preparation)) ||
           !Range(s.selected,0,2) || !Range(s.target,-1,4) || !Range(s.dialogue,0,3) ||
           !Range(s.preparation,0,1) || !Range(s.interlocutor,0,2) || !ChainOrders(s.orders) || !ChainOrders(s.signal.sequence)) return false;
        if((s.journey.stage==JourneyStage::Battle || s.journey.stage==JourneyStage::Debrief) &&
            (!s.beforeBattle || s.beforeBattle->mission!=s.journey.mission)) return false;
        const auto& r=s.realtime;
        if(!std::isfinite(r.elapsed) || r.elapsed<0 || r.elapsed>JourneyRealtime::Period ||
            !std::isfinite(r.activeSeconds) || r.activeSeconds<0 || r.activeSeconds>1000000 ||
            (r.speed!=.5 && r.speed!=1 && r.speed!=2) || !Range(r.pulses,0,10000)) return false;
        for(auto t:s.tactics) if(!Enum(t,7)) return false;
        for(const auto& o:r.orders) if(!Order(o)) return false;
        for(const auto& o:s.watch.lastOrders) if(!Order(o)) return false;
        for(int count:s.watch.stalled) if(!Range(count,0,10000)) return false;
        if(s.watch.history.size()>JourneyBattleWatch::HistoryLimit) return false;
        for(const auto& h:s.watch.history) if(!Enum(h.kind,5) || !Range(h.army,0,5) || !Cell(h.tile) || !Range(h.turn,0,16)) return false;
        return true;
    }
    bool ValidJourneySave(const JourneySaveData& s) { return JourneyArchive::SessionValid(s); }
}
