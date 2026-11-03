#pragma once
#include "TacticalBattleResponse.h"
namespace gunyumission
{
    // Actual command snapshots only. Terminal results belong to the aftermath.
    gunyuresponse::FRead Read(const FBattleBeat* Beat);
    gunyuresponse::FRead ReadCredit(const FBattleBeat* Beat);
    gunyuresponse::FRead ReadSupport(const FBattleBeat* Beat);
    bool SupportSmokeTest();
    bool CreditSmokeTest();
    bool SmokeTest();
}
