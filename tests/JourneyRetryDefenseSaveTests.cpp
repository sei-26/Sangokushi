#include "../NewGame/JourneyArchive.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
static std::vector<std::uint8_t> Raw(const JourneySaveData& s,unsigned version)
{
    auto copy=std::make_unique<JourneySaveData>(s);std::vector<std::uint8_t> b;JourneyArchive w{false,b};w.version=version;
    unsigned magic=0x53594e47,checksum=0;w(magic,version,checksum);w(*copy);
    checksum=2166136261u;for(std::size_t i=12;i<b.size();++i) { checksum^=b[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) b[8+i]=static_cast<std::uint8_t>(checksum>>(8*i));return b;
}
int main()
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=2;
    // Archive-only fixtures. Actual failed-battle evidence and rollback are tested by the retry rules.
    j.retryDefense=JourneyDefenseLesson{2,2,5,JourneyResolution::Withdrawn};s->beforeBattle=j;
    s->beforeBattle->retryDefense=JourneyDefenseLesson{2,5,7,JourneyResolution::Deadline};
    const auto saved=Bytes(*s);assert(saved[4]==12 && saved[5]==0 && saved[6]==0 && saved[7]==0);
    auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(saved,*loaded) && Bytes(*loaded)==saved);
    const auto& live=*loaded->journey.retryDefense;const auto& before=*loaded->beforeBattle->retryDefense;
    assert(live.mission==2 && live.failedGuards==2 && live.endedAt==5 && live.resolution==JourneyResolution::Withdrawn);
    assert(before.failedGuards==5 && before.endedAt==7 && before.resolution==JourneyResolution::Deadline);
    assert(loaded->journey.battleReview.spentGuardFailures==0 && !loaded->journey.history[2].completed);
    for(unsigned version=1;version<=11;++version)
    {
        auto old=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Raw(*s,version),*old));
        assert(!old->journey.retryDefense && old->beforeBattle && !old->beforeBattle->retryDefense);
    }
    const auto destination=Bytes(*loaded);
    for(int fault=0;fault<13;++fault)
    {
        auto bad=std::make_unique<JourneySaveData>(*s);auto& current=bad->journey;auto& lesson=*current.retryDefense;
        if(fault==0) lesson.failedGuards=0;if(fault==1) lesson.failedGuards=8;if(fault==2) lesson.failedGuards=-1;
        if(fault==3) lesson.endedAt=0;if(fault==4) lesson.endedAt=17;if(fault==5) lesson.mission=1;
        if(fault==6) lesson.resolution=JourneyResolution::Held;if(fault==7) lesson.resolution=JourneyResolution::None;
        if(fault==8) current.campaignFlow=false;if(fault==9) current.stage=JourneyStage::Encounter;
        if(fault==10) { current.mission=1;lesson.mission=1; }
        if(fault==11) bad->beforeBattle->retryDefense->failedGuards=16;
        if(fault==12) current.mission=99;
        std::vector<std::uint8_t> out{4,5,6};const auto untouched=out;
        assert(!ValidJourneySave(*bad) && !EncodeJourneySave(*bad,out) && out==untouched);
        assert(!DecodeJourneySave(Raw(*bad,12),*loaded) && Bytes(*loaded)==destination);
    }
    auto corrupt=saved;corrupt.back()^=1;assert(!DecodeJourneySave(corrupt,*loaded) && Bytes(*loaded)==destination);
    auto truncated=saved;truncated.pop_back();assert(!DecodeJourneySave(truncated,*loaded) && Bytes(*loaded)==destination);
    for(auto resolution:{JourneyResolution::Withdrawn,JourneyResolution::ForceLost,JourneyResolution::Deadline})
    { j.retryDefense->resolution=resolution;assert(DecodeJourneySave(Bytes(*s),*loaded)); }
    std::cout<<"Retry defense save PASS: separate nested lesson, no settled reward, old 1-11 unknown, context/mask/cycle/outcome bounds and transactional rejection\n";
}
