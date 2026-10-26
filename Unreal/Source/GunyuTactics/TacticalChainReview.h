#pragma once
#include "TacticalPresentation.h"
namespace frontline { struct ChainResult; }
struct FChainReviewRow
{
    int Actor=-1,Damage=0;
    bool bSuccess=false;
    FString Officer,Tactic,Role,Subject,Outcome,Link,Reason;
};
struct FChainReview
{
    TArray<FChainReviewRow> Rows;
    FString Summary,Lesson;
    int Completed=0,Damage=0;
    bool bAvailable=false,bStopped=false;
};
namespace gunyureview
{
    FChainReview Build(const frontline::ChainResult&,const TArray<FString>& KnownNames);
}
