#include "../NewGame/JourneyArchive.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
using Outcome=JourneyReliefOutcome;
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
static std::unique_ptr<JourneySaveData> Round(const JourneySaveData& s)
{ auto p=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Bytes(s),*p));assert(Bytes(*p)==Bytes(s));return p; }
static std::unique_ptr<JourneySaveData> Staged()
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=2;
    s->beforeBattle=j;assert(j.Prepare(0,1));assert(j.Move(1,Campaign::At(7,6)));assert(j.Move(0,Campaign::At(8,8)));assert(j.EndTurn());
    assert(j.turn==2 && j.heldTurns==1);return s;
}
static void Handover(StrategistJourney& j,bool rt=false)
{
    j.RecordReliefOrder(2,0,Campaign::At(8,9),Campaign::At(9,9),rt);
    assert(j.battleReview.relief.size()==1);
}
static std::u32string Talk(const StrategistJourney& j,int who)
{ std::u32string s;for(const auto& line:JourneyConversation(j,who)) s+=line.text;return s; }
static std::vector<std::uint8_t> Old(const JourneySaveData& s,unsigned version)
{
    auto copy=std::make_unique<JourneySaveData>(s);std::vector<std::uint8_t> b;JourneyArchive w{false,b};w.version=version;
    unsigned magic=0x53594e47,checksum=0;w(magic,version,checksum);w(*copy);
    checksum=2166136261u;for(std::size_t i=12;i<b.size();++i) { checksum^=b[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) b[8+i]=static_cast<std::uint8_t>(checksum>>(8*i));return b;
}
int main()
{
    auto s=Staged();auto& j=s->journey;
    const auto clean=Bytes(*s);j.Preview({{2,3,ChainTactic::Feint,-1}});
    assert(Bytes(*s)==clean && j.battleReview.relief.empty());
    const auto trust=j.officerTrust;Handover(j);
    for(int i=0;i<50;++i) Handover(j); // Repeated future edits are one operation, not a reward source.
    assert(j.officerTrust==trust && j.battleReview.relief[0].outcome==Outcome::Ordered);
    auto issued=Round(*s);assert(issued->journey.battleReview.relief[0].outcome==Outcome::Ordered);
    assert(j.Move(2,Campaign::At(9,9)));assert(j.battleReview.relief[0].outcome==Outcome::Departed);Round(*s);
    assert(j.Move(0,Campaign::At(8,9)));assert(j.battleReview.relief[0].outcome==Outcome::Arrived && !j.battleReview.relief[0].checkedTurn);
    auto arrived=Round(*s);assert(arrived->journey.EndTurn());assert(j.EndTurn());
    const auto& r=j.battleReview.relief[0];assert(r.outcome==Outcome::Held && r.checkedTurn==2 && r.arrivedTurn==2 && r.allSecured && j.heldTurns==2);
    assert(j.Hold(0) && j.Hold(1) && j.EndTurn() && j.won);
    assert(j.history[2].review.relief[0].outcome==Outcome::Held);auto settled=Round(*s);
    assert(Talk(j,0).find(U"両方に役目")!=std::u32string::npos && Talk(j,2).find(U"後ろには仲間の旗")!=std::u32string::npos);
    assert(Talk(j,1).find(U"双方の責")==std::u32string::npos); // Guan did not participate in this explicit handover.
    const auto saved=Bytes(*s);for(int i=0;i<20;++i) { Talk(j,0);Talk(j,2);ReliefReviewText(j); }
    assert(Bytes(*s)==saved && ReliefReviewText(j).find(U"交代直後")!=std::u32string::npos);
    // Personality fixtures use the same recorded facts, without awarding anything for dialogue.
    auto voice=std::make_unique<JourneySaveData>(*s);voice->journey.history[2].review.relief[0].outgoing=1;
    assert(Talk(voice->journey,1).find(U"双方の責")!=std::u32string::npos);
    voice->journey.history[2].review.relief[0].incoming=2;voice->journey.history[2].review.relief[0].outgoing=0;
    assert(Talk(voice->journey,2).find(U"俺が残って")!=std::u32string::npos);
    // Council guard assignments outside the beacon mission use ordinary guard language.
    voice->journey.mission=1;voice->journey.history[1]=voice->journey.history[2];
    const auto ordinary=JourneyConversation(voice->journey,2);
    assert(ordinary.size()>=3);
    for(std::size_t k=ordinary.size()-3;k<ordinary.size();++k) assert(ordinary[k].text.find(U"火")==std::u32string::npos);
    // Arrival with an adjacent enemy is a real failure at the next objective check.
    auto failed=Staged();auto& f=failed->journey;f.world.armies[3].tile=f.world.armies[3].target=Campaign::At(9,10);f.RefreshIntelligence();
    Handover(f);assert(f.Move(2,Campaign::At(9,9)) && f.Move(0,Campaign::At(8,9)) && f.EndTurn());
    assert(f.battleReview.relief[0].outcome==Outcome::Lost && f.heldTurns==0);assert(f.Withdraw());Round(*failed);
    assert(Talk(f,0).find(U"人を入れれば守れる")!=std::u32string::npos);
    assert(Talk(f,2).find(U"火が続かなかった")!=std::u32string::npos);
    // A real council duty outside both beacons is checked at that location, not described as a fire.
    auto duty=Staged();auto& d=duty->journey;
    d.world.armies[3].tile=d.world.armies[3].target=Campaign::At(13,12);d.world.armies[4].tile=d.world.armies[4].target=Campaign::At(13,2);d.RefreshIntelligence();
    d.assignments[0]={JourneyDuty::Hold,Campaign::At(8,8),true};
    d.RecordReliefOrder(0,2,Campaign::At(8,8),Campaign::At(9,8),false);
    assert(d.Move(0,Campaign::At(9,8)) && d.Move(2,Campaign::At(8,8)) && d.EndTurn());
    assert(d.battleReview.relief.size()==1 && d.battleReview.relief[0].outcome==Outcome::Held && d.battleReview.relief[0].allSecured && d.heldTurns==0);
    assert(d.Withdraw());Round(*duty);assert(ReliefReviewText(d).find(U"烽火")==std::u32string::npos);
    const auto dutyTalk=JourneyConversation(d,0);
    for(std::size_t k=dutyTalk.size()-3;k<dutyTalk.size();++k) assert(dutyTalk[k].text.find(U"火")==std::u32string::npos);
    // RT wait, save during the gap, actual arrival and later recovery are distinct.
    auto rt=Staged();rt->realtimeMode=true;auto& a=rt->journey;
    a.world.armies[3].tile=a.world.armies[3].target=Campaign::At(13,12);a.world.armies[4].tile=a.world.armies[4].target=Campaign::At(13,2);a.RefreshIntelligence();
    Handover(a,true);assert(rt->realtime.Queue(a,2,JourneyDirective::March,Campaign::At(9,9)));
    assert(rt->realtime.Queue(a,0,JourneyDirective::March,Campaign::At(8,9)));rt->realtime.SetArrival(a,0,JourneyArrival::Hold);
    assert(rt->realtime.Resolve(a));assert(a.battleReview.relief[0].outcome==Outcome::Departed && a.battleReview.relief[0].gapCycles==1);
    auto gap=Round(*rt);assert(gap->realtime.Resolve(gap->journey));assert(rt->realtime.Resolve(a));
    assert(a.battleReview.relief[0].outcome==Outcome::Recovered && a.battleReview.relief[0].checkedTurn==3 && a.heldTurns==1);
    assert(Bytes(*rt)==Bytes(*gap));assert(a.Withdraw());Round(*rt);
    assert(Talk(a,0).find(U"繋ぎ直せた")!=std::u32string::npos && Talk(a,2).find(U"待ってる間")!=std::u32string::npos);
    // Merely issuing, cancelled orders, and ending before a check never become a held result.
    auto pending=Staged();Handover(pending->journey,true);pending->realtime.Cancel(2);assert(pending->realtime.Resolve(pending->journey));
    assert(pending->journey.battleReview.relief[0].outcome==Outcome::Unconfirmed);assert(pending->journey.Withdraw());
    assert(Talk(pending->journey,0).find(U"両方に役目")==std::u32string::npos);
    auto early=Staged();Handover(early->journey);assert(early->journey.Move(2,Campaign::At(9,9)) && early->journey.Move(0,Campaign::At(8,9)) && early->journey.Withdraw());
    assert(early->journey.history[2].review.relief[0].outcome==Outcome::Unconfirmed);
    assert(Talk(early->journey,0).find(U"その後まで守れたとは")!=std::u32string::npos);
    // Old formats do not invent past relief; the upgraded active battle can record a new explicit operation.
    for(unsigned version:{1u,2u,3u,4u,5u,6u,7u,8u})
    {
        auto legacy=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Old(*s,version),*legacy));
        assert(!legacy->journey.history[2].review.reliefTracked && legacy->journey.history[2].review.relief.empty());
        assert(Talk(legacy->journey,0).find(U"両方に役目")==std::u32string::npos);
    }
    auto oldActive=std::make_unique<JourneySaveData>();auto fresh=Staged();assert(DecodeJourneySave(Old(*fresh,8),*oldActive));
    assert(!oldActive->journey.battleReview.reliefTracked);Handover(oldActive->journey);assert(oldActive->journey.battleReview.reliefTracked);Round(*oldActive);
    auto invalid=std::make_unique<JourneySaveData>(*s);invalid->journey.battleReview.relief[0].checkedTurn=0;assert(!ValidJourneySave(*invalid));
    invalid=std::make_unique<JourneySaveData>(*s);invalid->journey.battleReview.relief[0].incoming=2;assert(!ValidJourneySave(*invalid));
    invalid=std::make_unique<JourneySaveData>(*s);invalid->journey.battleReview.reliefTracked=false;assert(!ValidJourneySave(*invalid));
    std::cout<<"Relief story PASS: real staged movement, first guard check, gap and loss, saved pending/review, optional character voices, no click rewards, cancelled plans and v1-8 unknown history\n";
}
