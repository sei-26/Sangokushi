#include "JourneyDutyAftermathPlayer.hpp"
#include "../NewGame/JourneyAftermathRole.hpp"
#include <cassert>
#include <iostream>

int main()
{
    for(int variant=0;variant<4;++variant)
    {
        auto save=dutyaftermathqa::Play(variant);
        if(!save) std::cerr<<"Public duty playthrough failed for variant "<<variant<<'\n';
        assert(save);
        auto& j=save->journey;
        const auto scene=ReadDutyAftermath(j);
        const auto episode=BuildJourneyEpisode(j);
        assert(scene.lines.size()==1 && scene.lines.front().speaker==(variant==2 ? 0:1));
        const auto& voice=scene.lines.front().text;
        assert(std::count_if(episode.lines.begin(),episode.lines.end(),[&](const auto& line) { return line.text==voice; })==1);
        if(variant==0)
        {
            assert(j.won && ReadRoleAftermath(j).summary.find(U"河港に旗を立てた：張飛")!=std::u32string::npos);
            assert(scene.summary==U"軍議の担当　劉備：固守実行 ／ 関羽：伏兵未成立" && voice.find(U"仕込めなかった")!=std::u32string::npos);
            assert(episode.lines[1].speaker==2);
        }
        else if(variant==2)
            assert(!j.won && j.withdrawn && scene.summary==U"軍議の担当　劉備：固守未成立");
        else
            assert(j.won==(variant==1) && scene.summary==U"軍議の担当　関羽：伏兵準備");

        std::vector<std::uint8_t> before,after;
        assert(EncodeJourneySave(*save,before));
        for(int n=0;n<3;++n) { ReadDutyAftermath(j);BuildJourneyEpisode(j); }
        assert(EncodeJourneySave(*save,after) && before==after);
        auto reload=std::make_unique<JourneySaveData>();
        assert(DecodeJourneySave(before,*reload));
        assert(ReadDutyAftermath(reload->journey).lines.front().text==voice);
        reload->journey.assignments={}; // The active order is not evidence for the settled result.
        assert(ReadDutyAftermath(reload->journey).summary==scene.summary);
        const auto assignments=reload->journey.history[j.mission].assignments;
        reload->journey.history[j.mission].assignments={}; // A save that never recorded duties.
        assert(ReadDutyAftermath(reload->journey).summary.empty());
        assert(BuildJourneyEpisode(reload->journey).lines.size()+1==episode.lines.size());
        reload->journey.history[j.mission].assignments=assignments;
        for(int id=0;id<3;++id) reload->journey.world.armies[id].troops=0;
        assert(ReadDutyAftermath(reload->journey).lines.empty() && ReadDutyAftermath(reload->journey).summary==scene.summary);
        assert(ReadDutyAftermath(*save->beforeBattle).summary.empty());
    }
    std::cout<<"Duty aftermath PASS: genuine port capture, convoy delivery and withdrawal, only settled duties, one optional line, immutable save/load and no voice for lost units\n";
}
