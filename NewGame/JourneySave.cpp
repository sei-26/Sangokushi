#include "JourneyArchive.hpp"
#include <memory>

namespace frontline
{
    namespace
    {
        unsigned Checksum(const std::vector<std::uint8_t>& bytes,std::size_t start)
        { unsigned hash=2166136261u;for(auto i=start;i<bytes.size();++i) { hash^=bytes[i];hash*=16777619u; }return hash; }
    }
    bool EncodeJourneySave(const JourneySaveData& source,std::vector<std::uint8_t>& output)
    {
        try
        {
            if(!ValidJourneySave(source)) return false;
            auto copy=std::make_unique<JourneySaveData>(source);
            std::vector<std::uint8_t> bytes;
            JourneyArchive writer{false,bytes};
            unsigned magic=0x53594e47,version=5,checksum=0;
            writer(magic,version,checksum);writer(*copy);
            checksum=Checksum(bytes,12);
            for(int i=0;i<4;++i) bytes[8+i]=static_cast<std::uint8_t>(checksum>>(i*8));
            output=std::move(bytes);return true;
        }
        catch(const std::exception&) { return false; }
    }
    bool DecodeJourneySave(const std::vector<std::uint8_t>& input,JourneySaveData& output)
    {
        if(input.size()<12 || input.size()>JourneySaveLimit) return false;
        try
        {
            auto bytes=input;JourneyArchive reader{true,bytes};
            unsigned magic=0,version=0,checksum=0;reader(magic,version,checksum);
            if(magic!=0x53594e47 || (version<1 || version>5) || checksum!=Checksum(bytes,12)) return false;
            reader.version=version;
            auto candidate=std::make_unique<JourneySaveData>();reader(*candidate);
            if(reader.offset!=bytes.size() || !ValidJourneySave(*candidate)) return false;
            output=std::move(*candidate);output.realtime.paused=true;return true;
        }
        catch(const std::exception&) { return false; }
    }
}
