#pragma once
#include "CoreMinimal.h"
#include <vector>
#include <cstdint>
namespace gunyusave
{
    FString SlotPath(int Slot);
    bool Read(int Slot,std::vector<std::uint8_t>& Bytes,bool& bBackup);
    bool Write(int Slot,const std::vector<std::uint8_t>& Bytes);
    FString Describe(int Slot,bool& bAvailable,bool& bExists);
}
