#include "../NewGame/JourneyAftermathRole.hpp"
#include "../NewGame/JourneySave.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::u32string Text(const JourneyRoleAftermath& r)
{ std::u32string out;for(const auto& line:r.lines) out+=line.text;return out; }
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> out;assert(EncodeJourneySave(s,out));return out; }
static void Place(StrategistJourney& j,int a,int x,int y)
{ j.world.armies[a].tile=j.world.armies[a].target=Campaign::At(x,y); }
static std::unique_ptr<JourneySaveData> Play()
{
    auto save=std::make_unique<JourneySaveData>();auto& j=save->journey;j.ResetCampaign();j.Meet();j.mission=2;save->beforeBattle=j;save->realtimeMode=true;
    assert(j.Prepare(1,0)); // Authored contact boundary; tactics, updates and victory are ordinary shared rules.
    for(int y=1;y<StrategistJourney::Rows-1;++y) for(int x=1;x<StrategistJourney::Columns-1;++x) j.world.tiles[Campaign::At(x,y)].terrain=Terrain::Plain;
    Place(j,0,6,6);Place(j,1,7,6);Place(j,2,8,9);Place(j,3,6,5);j.world.armies[3].troops=3000;j.world.armies[3].morale=90;j.world.armies[4].troops=0;
    j.assignments[0]={};j.assignments[1]={JourneyDuty::Hold,j.goal,true};j.assignments[2]={JourneyDuty::Hold,j.Definition().secondGoal,true};
    j.chain.Reset(j.world);j.RefreshIntelligence();for(int a=0;a<3;++a) assert(save->realtime.Queue(j,a,JourneyDirective::Hold));
    assert(j.Execute({{0,3,ChainTactic::Feint,-1},{1,3,ChainTactic::Disrupt,-1}}).completed==2);
    for(int cycle=0;cycle<10 && j.stage==JourneyStage::Battle;++cycle) assert(save->realtime.Resolve(j));
    assert(j.stage==JourneyStage::Debrief && j.won && j.history[2].review.confirmedDefenders==6 && j.history[2].review.spentGuardFailures==2);return save;
}
int main()
{
    auto save=Play();auto& j=save->journey;const auto before=Bytes(*save);const auto role=ReadDefenseAftermath(j);const auto text=Text(role);
    assert(role.lines.size()==4 && role.lines[1].speaker==1 && role.lines[2].speaker==2 && role.lines.back().speaker==3);
    assert(role.summary.find(U"連鎖：劉備・関羽")!=std::u32string::npos && role.summary.find(U"烽火の守備：関羽・張飛")!=std::u32string::npos && role.summary.find(U"固守できず：関羽")!=std::u32string::npos);
    assert(text.find(U"火を支えた隊として")!=std::u32string::npos && text.find(U"構えられない時もあった")!=std::u32string::npos && text.find(U"立て直した")==std::u32string::npos && text.find(U"三連鎖")==std::u32string::npos);
    for(int i=0;i<10;++i) ReadDefenseAftermath(j);assert(Bytes(*save)==before);
    auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(before,*loaded) && Text(ReadDefenseAftermath(loaded->journey))==text && Bytes(*loaded)==before);
    loaded->journey.defenseChecks={};loaded->journey.assignments={};loaded->journey.enemyPolicy=2;loaded->journey.world.armies[4].tile=Campaign::At(13,3);
    assert(Text(ReadDefenseAftermath(loaded->journey))==text); // No current assignment, transient report or hidden enemy supplies story credit.
    for(int id=0;id<3;++id)
    {
        auto display=std::make_unique<JourneySaveData>(*save);auto& v=display->journey;v.world.armies[id].troops=0;
        for(const auto& line:ReadDefenseAftermath(v).lines) assert(line.speaker!=id);
    }
    auto display=std::make_unique<JourneySaveData>(*save);auto& v=display->journey;auto& h=v.history[2];
    h.story.costly=true;auto costly=ReadDefenseAftermath(v);assert(Text(costly).find(U"大きな兵の損耗")!=std::u32string::npos);for(const auto& line:costly.lines) assert(!line.warmMask);
    h.story.costly=false;h.story.pledge=JourneyPledge::Preserve;h.story.pledgeKept=false;assert(Text(ReadDefenseAftermath(v)).find(U"約束は、果たせなかった")!=std::u32string::npos);
    h.story.pledge=JourneyPledge::None;h.won=v.won=false;h.withdrawn=v.withdrawn=true;h.review.resolution=JourneyResolution::Withdrawn;
    assert(Text(ReadDefenseAftermath(v)).find(U"任務は果たせなかった")!=std::u32string::npos);
    h.review.confirmedDefenders=h.review.spentGuardFailures=0;assert(ReadDefenseAftermath(v).lines.empty());
    for(int invalid:{-1,8}) { h.review.confirmedDefenders=invalid;assert(ReadDefenseAftermath(v).lines.empty()); }
    h.review.confirmedDefenders=6;h.review.spentGuardFailures=0;h.review.coordinatedOfficers=7;h.review.longestChain=0;
    assert(ReadDefenseAftermath(v).summary.find(U"連鎖：")==std::u32string::npos);
    h.review.endedAt=0;assert(ReadDefenseAftermath(v).lines.empty());h.review.endedAt=v.turn;
    v.stage=JourneyStage::Battle;assert(ReadDefenseAftermath(v).lines.empty());v.stage=JourneyStage::Complete;assert(ReadDefenseAftermath(v).lines.empty());
    std::cout<<"Defense aftermath PASS: real local updates and battle outcome, exact recorded roles, costs, living voices, immutable save/reload and no invented timing or current-board credit\n";
}
