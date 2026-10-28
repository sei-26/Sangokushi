#pragma once
#include "JourneySave.hpp"
#include <cstring>
#include <stdexcept>
#include <type_traits>

namespace frontline
{
    struct JourneyArchive
    {
        static bool SessionValid(const JourneySaveData&);
        bool reading;
        std::vector<std::uint8_t>& bytes;
        std::size_t offset=0;
        unsigned version=8;
        void Require(bool value) { if (!value) throw std::runtime_error("Invalid journey archive"); }
        template<class... T> void operator()(T&... values) { (IO(values),...); }
        template<class T> void IO(T& value)
        {
            if constexpr (std::is_enum_v<T>)
            { int raw=static_cast<int>(value);IO(raw);if(reading) value=static_cast<T>(raw); }
            else if constexpr (std::is_same_v<T,bool>)
            { unsigned raw=value ? 1:0;IO(raw);Require(raw<=1);if(reading) value=raw!=0; }
            else
            {
                static_assert(std::is_integral_v<T> || std::is_same_v<T,double>);
                constexpr int Count=std::is_same_v<T,double> ? 8:4;
                std::uint64_t raw=0;
                if (!reading)
                {
                    if constexpr(std::is_same_v<T,double>) std::memcpy(&raw,&value,8);
                    else raw=static_cast<std::uint32_t>(value);
                }
                Require(reading ? offset+Count<=bytes.size() : bytes.size()+Count<=JourneySaveLimit);
                for(int i=0;i<Count;++i)
                    if(reading) raw|=std::uint64_t(bytes[offset++])<<(i*8);
                    else bytes.push_back(static_cast<std::uint8_t>(raw>>(i*8)));
                if(reading)
                {
                    if constexpr(std::is_same_v<T,double>) std::memcpy(&value,&raw,8);
                    else if constexpr(std::is_signed_v<T>) value=static_cast<T>(static_cast<std::int32_t>(raw));
                    else value=static_cast<T>(raw);
                }
            }
        }
        template<class T,std::size_t N> void IO(std::array<T,N>& values) { for(auto& value:values) IO(value); }
        template<class T> void IO(std::vector<T>& values)
        {
            unsigned count=static_cast<unsigned>(values.size());IO(count);
            Require(count<=65536);if(reading) { Require(count<=bytes.size()-offset);values.resize(count); }
            for(unsigned i=0;i<count;++i)
                if constexpr(std::is_same_v<T,bool>) { bool v=values[i];IO(v);values[i]=v; }
                else IO(values[i]);
        }
        void IO(std::u32string& value)
        {
            unsigned count=static_cast<unsigned>(value.size());IO(count);Require(count<=4096);
            if(reading) { Require(count<=(bytes.size()-offset)/4);value.resize(count); }
            for(auto& code:value) { IO(code);Require(code<=0x10ffff && !(code>=0xd800 && code<=0xdfff)); }
        }
        template<class T> void IO(std::optional<T>& value)
        { bool present=value.has_value();IO(present);if(reading) { if(present) value.emplace();else value.reset(); }if(present) IO(*value); }
        void IO(Tile&);
        void IO(City&);
        void IO(Region&);
        void IO(General&);
        void IO(Army&);
        void IO(TacticEvent&);
        void IO(ReturnEvent&);
        void IO(Bond&);
        void IO(Mission&);
        void IO(Assignment&);
        void IO(Training&);
        void IO(Chronicle&);
        void IO(Campaign&);
        void IO(ArmyContact&);
        void IO(BattleIntelligence&);
        void IO(TacticChain&);
        void IO(ChainOrder&);
        void IO(ChainUnitChange&);
        void IO(ChainEvent&);
        void IO(ChainResult&);
        void IO(JourneyBattleReview&);
        void IO(JourneyResult&);
        void IO(JourneyAssignment&);
        void IO(JourneyBattleStory&);
        void IO(StrategistJourney&);
        void IO(JourneyStandingOrder&);
        void IO(JourneyRealtime&);
        void IO(JourneyChainSignal&);
        void IO(JourneyBattleNotice&);
        void IO(JourneyBattleWatch&);
        void IO(JourneySaveData&);
    };
}
