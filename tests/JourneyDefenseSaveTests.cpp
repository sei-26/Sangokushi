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
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=2;s->beforeBattle=j;
    assert(j.Prepare(1,0));
    // Isolated archive fixtures: real defense evidence and narrative attribution are tested separately.
    j.battleReview.confirmedDefenders=5;j.battleReview.spentGuardFailures=3;
    j.history[1].review.confirmedDefenders=2;j.history[1].review.spentGuardFailures=6;
    s->beforeBattle->battleReview.confirmedDefenders=1;s->beforeBattle->battleReview.spentGuardFailures=4;
    s->beforeBattle->history[0].review.confirmedDefenders=7;s->beforeBattle->history[0].review.spentGuardFailures=7;
    const auto saved=Bytes(*s);assert(saved[4]==11 && saved[5]==0 && saved[6]==0 && saved[7]==0);
    auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(saved,*loaded) && Bytes(*loaded)==saved);
    assert(loaded->journey.battleReview.confirmedDefenders==5 && loaded->journey.battleReview.spentGuardFailures==3);
    assert(loaded->journey.history[1].review.confirmedDefenders==2 && loaded->journey.history[1].review.spentGuardFailures==6);
    assert(loaded->beforeBattle->battleReview.confirmedDefenders==1 && loaded->beforeBattle->battleReview.spentGuardFailures==4);
    assert(loaded->beforeBattle->history[0].review.confirmedDefenders==7 && loaded->beforeBattle->history[0].review.spentGuardFailures==7);
    for(unsigned version=1;version<=10;++version)
    {
        auto old=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Raw(*s,version),*old));
        auto Empty=[](const StrategistJourney& v)
        {
            if(v.battleReview.confirmedDefenders || v.battleReview.spentGuardFailures) return false;
            for(const auto& h:v.history) if(h.review.confirmedDefenders || h.review.spentGuardFailures) return false;
            return true;
        };
        assert(Empty(old->journey) && old->beforeBattle && Empty(*old->beforeBattle));
    }
    const auto destination=Bytes(*loaded);
    for(int value:{-1,8,16}) for(int field=0;field<4;++field)
    {
        auto invalid=std::make_unique<JourneySaveData>(*s);
        int* masks[]={&invalid->journey.battleReview.confirmedDefenders,&invalid->journey.battleReview.spentGuardFailures,
            &invalid->journey.history[1].review.confirmedDefenders,&invalid->beforeBattle->battleReview.spentGuardFailures};
        *masks[field]=value;std::vector<std::uint8_t> out{1,2,3};const auto untouched=out;
        assert(!ValidJourneySave(*invalid) && !EncodeJourneySave(*invalid,out) && out==untouched);
        assert(!DecodeJourneySave(Raw(*invalid,11),*loaded) && Bytes(*loaded)==destination);
    }
    auto corrupt=saved;corrupt.back()^=1;assert(!DecodeJourneySave(corrupt,*loaded) && Bytes(*loaded)==destination);
    auto truncated=saved;truncated.pop_back();assert(!DecodeJourneySave(truncated,*loaded) && Bytes(*loaded)==destination);
    std::cout<<"Defense save PASS: v11 active/history/retry snapshot masks, overlapping outcomes, v1-10 unknown history and transactional invalid/corrupt rejection\n";
}
