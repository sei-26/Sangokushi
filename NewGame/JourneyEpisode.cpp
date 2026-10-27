#include "JourneyEpisode.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    const char32_t* EpisodeSpeakerName(int speaker)
    {
        const char32_t* names[]={U"劉備",U"関羽",U"張飛",U"主人公",U"季安"};
        return speaker>=0 && speaker<5 ? names[speaker]:U"語り";
    }
    JourneyEpisode BuildJourneyEpisode(const StrategistJourney& j)
    {
        JourneyEpisode e;
        if(j.stage!=JourneyStage::Preparation && j.stage!=JourneyStage::Debrief && j.stage!=JourneyStage::Complete) return e;
        const char32_t* titles[]={U"第一話　泥の道",U"第二話　米と旗",U"第三話　消えない火"};
        e.title=titles[std::clamp(j.mission,0,2)];
        if(j.stage==JourneyStage::Preparation) BuildEpisodeCouncil(j,e);
        else BuildEpisodeAftermath(j,e);
        return e; // Pure reading of settled/public facts; never awards trust or reveals enemy orders.
    }
}
