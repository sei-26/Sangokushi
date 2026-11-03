#include "../NewGame/JourneyAftermathRole.hpp"
#include "../NewGame/JourneySave.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
static std::unique_ptr<JourneySaveData> Staged()
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=2;
    s->beforeBattle=j;assert(j.Prepare(0,1));assert(j.Move(1,Campaign::At(7,6)));assert(j.Move(0,Campaign::At(8,8)));assert(j.EndTurn());
    assert(j.heldTurns==1);j.RecordReliefOrder(2,0,Campaign::At(8,9),Campaign::At(9,9),false);return s;
}
static void MovePair(StrategistJourney& j)
{ assert(j.Move(2,Campaign::At(9,9)) && j.Move(0,Campaign::At(8,9))); }
static std::u32string Text(const JourneyRoleAftermath& r)
{ std::u32string t;for(const auto& l:r.lines) t+=l.text;return t; }
int main()
{
    auto s=Staged();auto& j=s->journey;assert(ReadRoleAftermath(j).lines.empty());
    MovePair(j);assert(ReadRoleAftermath(j).lines.empty()); // Arrival cannot invent settled history.
    assert(j.EndTurn() && j.Hold(0) && j.Hold(1) && j.EndTurn() && j.won);
    const auto before=Bytes(*s);const auto role=ReadRoleAftermath(j);const auto scene=BuildJourneyEpisode(j);
    assert(role.lines.size()==4 && scene.lines.size()==4 && role.summary.find(U"張飛 → 劉備")!=std::u32string::npos);
    assert(scene.lines[1].speaker==0 && scene.lines[2].speaker==2 && scene.lines[3].speaker==3);
    assert(scene.lines[1].text.find(U"翼徳")!=std::u32string::npos && scene.lines[2].text.find(U"兄者の旗")!=std::u32string::npos);
    assert(role.lines[1].warmMask==1 && role.lines[2].warmMask==4 && Text(role).find(U"二人の名")!=std::u32string::npos);
    for(int i=0;i<20;++i) { BuildJourneyEpisode(j);ReadRoleAftermath(j); }assert(Bytes(*s)==before);
    auto restored=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(before,*restored));
    assert(Text(ReadRoleAftermath(restored->journey))==Text(role));
    // Hidden future enemy choices cannot rewrite a settled conversation.
    restored->journey.enemyPolicy=2;restored->journey.world.armies[3].tile=Campaign::At(13,3);
    assert(Text(ReadRoleAftermath(restored->journey))==Text(role));
    auto failed=Staged();auto& f=failed->journey;
    f.world.armies[3].tile=f.world.armies[3].target=Campaign::At(9,10);f.RefreshIntelligence();MovePair(f);
    assert(f.EndTurn() && f.heldTurns==0 && f.Withdraw());const auto lost=ReadRoleAftermath(f);
    assert(lost.summary.find(U"成立せず")!=std::u32string::npos && lost.lines[1].warmMask==0 && Text(lost).find(U"任務を果たしたことにはならなかった")!=std::u32string::npos);
    assert(Text(lost).find(U"二人の名")==std::u32string::npos);
    auto early=Staged();MovePair(early->journey);assert(early->journey.Withdraw());
    assert(ReadRoleAftermath(early->journey).summary.find(U"未確認")!=std::u32string::npos);
    auto order=Staged();assert(order->journey.Withdraw());assert(ReadRoleAftermath(order->journey).lines.empty());
    auto recovered=Staged();auto& a=recovered->journey;
    // Isolate the actual RT occupied-tile delay from enemy collision, as in the existing relief suite.
    for(int enemy:{3,4}) a.world.armies[enemy].tile=a.world.armies[enemy].target=Campaign::At(13,enemy==3 ? 12:2);
    a.RefreshIntelligence();a.battleReview.relief.clear();a.RecordReliefOrder(2,0,Campaign::At(8,9),Campaign::At(9,9),true);
    assert(recovered->realtime.Queue(a,2,JourneyDirective::March,Campaign::At(9,9)) && recovered->realtime.Queue(a,0,JourneyDirective::March,Campaign::At(8,9)));
    recovered->realtime.SetArrival(a,0,JourneyArrival::Hold);
    assert(recovered->realtime.Resolve(a) && recovered->realtime.Resolve(a) && a.Withdraw());
    const auto recovery=ReadRoleAftermath(a);assert(recovery.summary.find(U"空白のあと")!=std::u32string::npos && Text(recovery).find(U"待ってる間")!=std::u32string::npos);
    // The same gap remains part of the story after a real subsequent victory.
    auto returned=Staged();auto& w=returned->journey;
    for(int enemy:{3,4}) w.world.armies[enemy].tile=w.world.armies[enemy].target=Campaign::At(13,enemy==3 ? 12:2);
    w.RefreshIntelligence();w.battleReview.relief.clear();w.RecordReliefOrder(2,0,Campaign::At(8,9),Campaign::At(9,9),true);
    assert(returned->realtime.Queue(w,2,JourneyDirective::March,Campaign::At(9,9)) && returned->realtime.Queue(w,0,JourneyDirective::March,Campaign::At(8,9)));
    returned->realtime.SetArrival(w,0,JourneyArrival::Hold);assert(returned->realtime.Resolve(w) && returned->realtime.Resolve(w));
    for(int i=0;i<2 && w.stage==JourneyStage::Battle;++i) assert(w.Hold(0) && w.Hold(1) && w.EndTurn());
    assert(w.won && ReadRoleAftermath(w).summary.find(U"空白のあと")!=std::u32string::npos && Text(ReadRoleAftermath(w)).find(U"任務を果たしたことにはならなかった")==std::u32string::npos);
    // Settled-history voice fixtures isolate local success from the rest of a battle.
    auto fixture=std::make_unique<JourneySaveData>(*s);auto& v=fixture->journey;
    v.history[2].review.relief.back().allSecured=false;
    assert(ReadRoleAftermath(v).summary.find(U"両翼の維持は未成立")!=std::u32string::npos && Text(ReadRoleAftermath(v)).find(U"もう一方")!=std::u32string::npos);
    v.history[2].review.relief.back().allSecured=true;v.history[2].story.costly=true;
    assert(Text(ReadRoleAftermath(v)).find(U"大きな兵の損耗")!=std::u32string::npos && ReadRoleAftermath(v).lines[1].warmMask==0);
    v.history[2].story.costly=false;v.history[2].story.pledge=JourneyPledge::Preserve;v.history[2].story.pledgeKept=false;
    assert(Text(ReadRoleAftermath(v)).find(U"約束も、果たせていない")!=std::u32string::npos);
    v.history[2].story.pledge=JourneyPledge::None;v.history[2].review.relief.back().outgoing=1;
    assert(ReadRoleAftermath(v).lines[2].text.find(U"兄者が守りを預かってくださった")!=std::u32string::npos);
    v.world.armies[0].troops=0;const auto quiet=ReadRoleAftermath(v);for(const auto& l:quiet.lines) assert(l.speaker!=0);
    v.history[2].review.relief.clear();assert(ReadRoleAftermath(v).lines.empty()); // Old/unknown records get no invented handover.
    j.campaignFlow=false;assert(BuildJourneyEpisode(j).lines[2].text==role.lines[2].text);
    j.campaignFlow=true;assert(j.Continue());assert(ReadRoleAftermath(j).lines.empty() && BuildJourneyEpisode(j).lines.size()==4);
    std::cout<<"Role aftermath PASS: actual checked/failed/recovered/unchecked handovers, precise participants, partial guard and costs, immutable save/load, unknown history and finale\n";
}
