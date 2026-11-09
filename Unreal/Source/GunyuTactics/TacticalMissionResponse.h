#pragma once
#include "TacticalBattleResponse.h"
namespace gunyumission
{
    // Actual command snapshots only. Terminal results belong to the aftermath.
    gunyuresponse::FRead Read(const FBattleBeat* Beat,gunyuresponse::FRead RetryDefense={});
    gunyuresponse::FRead ReadCredit(const FBattleBeat* Beat);
    gunyuresponse::FRead ReadSupport(const FBattleBeat* Beat);
    bool SupportSmokeTest();
    gunyuresponse::FRead ReadSupportCombo(const TArray<FBattleBeat>& Beats);
    bool SupportComboSmokeTest();
    gunyuresponse::FRead ReadDefense(const FBattleBeat* Beat);
    gunyuresponse::FRead RetryDefenseVoices(const FBattleBeat& Beat,int Guard,int Supporter,bool bNorth);
    bool DefenseSmokeTest();
    bool CreditSmokeTest();
    bool SmokeTest();
}
