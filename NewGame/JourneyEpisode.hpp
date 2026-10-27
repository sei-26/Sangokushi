#pragma once
#include <string>
#include <vector>
namespace frontline
{
    class StrategistJourney;
    // 0 Liu, 1 Guan, 2 Zhang, 3 protagonist, 4 Ji An, -1 narration.
    struct JourneyEpisodeLine { int speaker=-1;std::u32string text,direction; };
    struct JourneyEpisode
    {
        std::u32string title,place,setting,question;
        std::vector<JourneyEpisodeLine> lines;
    };
    const char32_t* EpisodeSpeakerName(int speaker);
    JourneyEpisode BuildJourneyEpisode(const StrategistJourney& journey);
    void BuildEpisodeCouncil(const StrategistJourney&,JourneyEpisode&);
    void BuildEpisodeAftermath(const StrategistJourney&,JourneyEpisode&);
}
