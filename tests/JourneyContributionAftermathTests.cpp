#include "JourneyContributionPlayer.hpp"
#include "../NewGame/JourneyAftermathRole.hpp"
#include "../NewGame/JourneyArchive.hpp"
#include <cassert>
#include <iostream>
using namespace frontline;
static std::u32string Text(const JourneyRoleAftermath& r)
{ std::u32string text;for(const auto& line:r.lines) text+=line.text;return text; }
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
static std::vector<std::uint8_t> OldBytes(JourneySaveData s)
{
    std::vector<std::uint8_t> bytes;
    JourneyArchive writer{false,bytes};
    writer.version=19;
    unsigned magic=0x53594e47,version=19,checksum=0;
    writer(magic,version,checksum);writer(s);
    checksum=2166136261u;
    for(std::size_t i=12;i<bytes.size();++i) { checksum^=bytes[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) bytes[8+i]=static_cast<std::uint8_t>(checksum>>(i*8));
    return bytes;
}
static void PortHandoff()
{
    using namespace contributionqa;
    auto save=PortApproach();
    assert(save);
    auto& j=save->journey;
    assert(j.Move(1,Campaign::At(12,8)) && j.Move(2,PortBridgeExit));
    assert(j.battleReview.port.crossedBy[0]==1 && j.battleReview.port.crossedBy[1]==2);
    const std::vector<ChainOrder> push{{2,3,ChainTactic::Breakthrough,-1}};
    const auto before=Bytes(*save);
    assert(j.Preview(push).completed==1 && Bytes(*save)==before);
    assert(j.Execute(push).completed==1 && j.world.armies[3].troops>0);
    assert(j.battleReview.port.clearedBy==2 && j.battleReview.port.clearedOn==j.turn &&
        j.battleReview.port.capturedBy<0 && j.stage==JourneyStage::Battle);
    const auto opening=Bytes(*save);
    auto restored=std::make_unique<JourneySaveData>();
    assert(DecodeJourneySave(opening,*restored) && restored->journey.battleReview.port==j.battleReview.port);
    assert(restored->journey.Move(0,j.goal) && restored->journey.won);
    const auto won=Bytes(*restored);
    const auto result=ReadRoleAftermath(restored->journey);
    assert(result.summary.find(U"道を開いた：張飛")!=std::u32string::npos &&
        Text(result).find(U"北橋を最初に越えたのは関羽")!=std::u32string::npos &&
        result.lines[1].speaker==0 && result.lines[2].speaker==2);
    assert(restored->journey.battleReview.coordinatedOfficers==0 &&
        restored->journey.history[1].review.port.JointCapture());
    auto legacy=std::make_unique<JourneySaveData>();
    assert(DecodeJourneySave(OldBytes(*restored),*legacy) &&
        ReadRoleAftermath(legacy->journey).summary.find(U"河港に旗を立てた：劉備")!=std::u32string::npos &&
        !legacy->journey.history[1].review.port.JointCapture());
    assert(legacy->journey.Continue() && legacy->journey.RequiredHoldTurns()==3);
    assert(DecodeJourneySave(won,*restored) && restored->journey.Continue() &&
        restored->journey.RequiredHoldTurns()==2 && !Bytes(*restored).empty());

    assert(DecodeJourneySave(opening,*restored));
    auto& blocked=restored->journey;
    blocked.world.armies[4].troops=900;
    blocked.world.armies[4].tile=blocked.world.armies[4].target=Campaign::At(13,12);
    blocked.RefreshIntelligence();
    assert(blocked.Move(0,blocked.goal) && !blocked.won && blocked.battleReview.port.clearedBy<0);
    blocked.world.armies[4].troops=0;
    blocked.world.armies[3].troops=0;
    blocked.RefreshIntelligence();
    assert(blocked.EndTurn() && blocked.won && !blocked.history[1].review.port.JointCapture());

    assert(DecodeJourneySave(opening,*restored));
    auto& late=restored->journey;
    late.world.armies[3].troops=0;
    late.RefreshIntelligence();
    for(int cycle=0;cycle<=StrategistJourney::PortOpeningCycles;++cycle) assert(late.EndTurn());
    assert(late.battleReview.port.clearedBy<0 && late.Move(0,late.goal) && late.won &&
        !late.history[1].review.port.JointCapture());

    assert(DecodeJourneySave(before,*restored));
    auto& strike=restored->journey;
    strike.world.armies[3].troops=1;
    strike.RefreshIntelligence();
    assert(strike.Strike(2,3) && strike.battleReview.port.clearedBy==2);
    assert(strike.Move(0,strike.goal) && strike.history[1].review.port.JointCapture());

    auto realtime=PortApproach();
    assert(realtime->realtime.Queue(realtime->journey,2,JourneyDirective::March,PortBridgeExit) &&
        realtime->realtime.Resolve(realtime->journey) && realtime->journey.battleReview.port.crossedBy[1]==2);
}
int main()
{
    std::cout<<std::unitbuf;
    PortHandoff();
    for(int variant=0;variant<4;++variant)
    {
        auto save=contributionqa::Play(variant);if(!save) std::cout<<"public playthrough failed variant="<<variant<<"\n";assert(save);
        auto& j=save->journey;const auto before=Bytes(*save);const auto role=ReadRoleAftermath(j);const auto scene=BuildJourneyEpisode(j);
        std::cout<<"variant="<<variant<<" won="<<j.won<<" turn="<<j.turn<<" mask="<<j.history[j.mission].review.coordinatedOfficers<<" loss="<<j.casualties<<"\n";
        assert(role.lines.size()>=3 && role.lines.size()<=4 && scene.lines.size()==role.lines.size());
        if(variant<3) assert(role.summary.find(std::u32string(U"河港に旗を立てた：")+EpisodeSpeakerName(variant))!=std::u32string::npos && role.lines[1].speaker==variant);
        if(variant==2) assert(role.summary.find(U"連鎖に参加")==std::u32string::npos && Text(role).find(U"私も一手")==std::u32string::npos);
        else assert(role.summary.find(U"連鎖に参加")!=std::u32string::npos);
        for(int n=0;n<12;++n) { ReadRoleAftermath(j);BuildJourneyEpisode(j); }assert(Bytes(*save)==before);
        auto restored=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(before,*restored) && Bytes(*restored)==before);
        assert(Text(ReadRoleAftermath(restored->journey))==Text(role));
        restored->journey.enemyPolicy=2;restored->journey.world.armies[4].tile=Campaign::At(13,3);
        assert(Text(ReadRoleAftermath(restored->journey))==Text(role));
        if(variant==0)
        {
            // Explicit display-history cases: do not turn a local feat into an unearned celebration.
            auto fixture=std::make_unique<JourneySaveData>(*save);auto& v=fixture->journey;auto& h=v.history[1];
            h.story.costly=true;auto costly=ReadRoleAftermath(v);assert(Text(costly).find(U"大きな兵の損耗")!=std::u32string::npos && costly.lines[1].warmMask==0);
            h.story.costly=false;h.story.pledge=JourneyPledge::Preserve;h.story.pledgeKept=false;
            assert(Text(ReadRoleAftermath(v)).find(U"約束は、果たせなかった")!=std::u32string::npos);
            h.story.pledge=JourneyPledge::None;h.review.longestChain=2;h.review.coordinatedOfficers=7;
            assert(ReadRoleAftermath(v).summary.find(U"劉備・関羽・張飛")!=std::u32string::npos && Text(ReadRoleAftermath(v)).find(U"三連鎖")==std::u32string::npos);
            h.won=v.won=false;h.withdrawn=v.withdrawn=true;h.review.resolution=JourneyResolution::Withdrawn;
            const auto withdrawn=ReadRoleAftermath(v);assert(withdrawn.summary.find(U"河港に旗")==std::u32string::npos && Text(withdrawn).find(U"任務は果たせなかった")!=std::u32string::npos);
            v.world.armies[0].troops=0;for(const auto& line:ReadRoleAftermath(v).lines) assert(line.speaker!=0);
            h.review.coordinatedOfficers=0;assert(ReadContributionAftermath(v).lines.empty());
        }
        if(variant==1)
        {
            auto fixture=std::make_unique<JourneySaveData>(*save);auto& v=fixture->journey;v.history[1].review.coordinatedOfficers=0;
            v.world.armies[0].tile=v.goal;assert(ReadContributionAftermath(v).lines.empty()); // Ambiguous occupancy never picks an arbitrary captor.
            v.world.armies[0].tile=Campaign::At(2,2);v.world.tiles[v.goal].owner=1;assert(ReadContributionAftermath(v).lines.empty());
            v.world.tiles[v.goal].owner=0;v.history[1].review.endedAt=0;assert(ReadContributionAftermath(v).lines.empty());
        }
        assert(j.Continue());assert(ReadContributionAftermath(j).lines.empty()); // No past-board reconstruction after chapter advance.
    }
    std::cout<<"Contribution aftermath PASS: real port captors, actual coordination and ordinary attacks, immutable save/reload, unknown/ambiguous evidence, costs and no invented chain order\n";
}
