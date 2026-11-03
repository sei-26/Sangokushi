#include "JourneyContributionPlayer.hpp"
#include "../NewGame/JourneyAftermathRole.hpp"
#include <cassert>
#include <iostream>
using namespace frontline;
static std::u32string Text(const JourneyRoleAftermath& r)
{ std::u32string text;for(const auto& line:r.lines) text+=line.text;return text; }
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
int main()
{
    std::cout<<std::unitbuf;
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
