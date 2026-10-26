#include "TacticalSaveStorage.h"
#include "JourneySave.hpp"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TacticalState.h"
namespace gunyusave
{
    FString SlotPath(int Slot)
    {
        const bool QA=FParse::Param(FCommandLine::Get(),TEXT("GunyuSmoke")) || FParse::Param(FCommandLine::Get(),TEXT("GunyuCapture"));
        return FPaths::ProjectSavedDir()/(QA ? TEXT("Automation/JourneySaves") : TEXT("SaveGames/JourneyV1"))/FString::Printf(TEXT("slot%d.gny"),Slot);
    }
    static bool ReadFile(const FString& Path,std::vector<std::uint8_t>& Bytes)
    {
        const auto Size=IFileManager::Get().FileSize(*Path);
        if(Size<12 || Size>static_cast<int64>(frontline::JourneySaveLimit)) return false;
        TArray<uint8> Data;if(!FFileHelper::LoadFileToArray(Data,*Path)) return false;
        auto Candidate=std::make_unique<frontline::JourneySaveData>();
        std::vector<std::uint8_t> Copy(Data.GetData(),Data.GetData()+Data.Num());
        if(!frontline::DecodeJourneySave(Copy,*Candidate)) return false;
        Bytes=std::move(Copy);return true;
    }
    bool Read(int Slot,std::vector<std::uint8_t>& Bytes,bool& bBackup)
    {
        bBackup=false;if(Slot<0 || Slot>3) return false;
        if(ReadFile(SlotPath(Slot),Bytes)) return true;
        bBackup=ReadFile(SlotPath(Slot)+TEXT(".bak"),Bytes);return bBackup;
    }
    bool Write(int Slot,const std::vector<std::uint8_t>& Bytes)
    {
        if(Slot<0 || Slot>3) return false;
        auto Check=std::make_unique<frontline::JourneySaveData>();if(!frontline::DecodeJourneySave(Bytes,*Check)) return false;
        const FString Path=SlotPath(Slot),Temp=Path+TEXT(".tmp");
        if(!IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path),true)) return false;
        if(!FFileHelper::SaveArrayToFile(TArrayView<const uint8>(Bytes.data(),static_cast<int32>(Bytes.size())),*Temp)) return false;
        std::vector<std::uint8_t> Verified;if(!ReadFile(Temp,Verified) || Verified!=Bytes) return false;
        std::vector<std::uint8_t> Previous;
        // Keep the last valid primary; a broken primary never destroys a valid backup.
        if(ReadFile(Path,Previous) && IFileManager::Get().Copy(*(Path+TEXT(".bak")),*Path,true,true)!=COPY_OK) return false;
        return IFileManager::Get().Move(*Path,*Temp,true,true,false,true);
    }
    FString Describe(int Slot,bool& bAvailable,bool& bExists)
    {
        bExists=IFileManager::Get().FileExists(*SlotPath(Slot)) || IFileManager::Get().FileExists(*(SlotPath(Slot)+TEXT(".bak")));
        std::vector<std::uint8_t> Bytes;bool bBackup=false;bAvailable=Read(Slot,Bytes,bBackup);
        if(!bAvailable) return bExists ? TEXT("読み込めない記録です。現在の進行は変更しません。") : TEXT("記録なし");
        auto Save=std::make_unique<frontline::JourneySaveData>();frontline::DecodeJourneySave(Bytes,*Save);
        const TCHAR* Stages[]={TEXT("出会い"),TEXT("軍議"),TEXT("戦闘"),TEXT("戦後"),TEXT("序章完了")};
        const auto& J=Save->journey;
        return FString::Printf(TEXT("第%d戦・%s / 経過%d周期 / 信頼%d%s\n%s"),J.mission+1,Stages[static_cast<int>(J.stage)],FMath::Max(0,J.turn-1),J.trust,
            bBackup ? TEXT(" / 前回の記録から復元可") : TEXT(""),*JourneyText(J.MissionName()));
    }
}
