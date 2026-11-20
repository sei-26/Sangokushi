#pragma once
#include "CoreMinimal.h"
struct FGrandProgress
{
    bool bActive=false,bBattle=false,bWon=false,bRetreat=false;
    int Mission=-1,Goal=-1,Opened=0,Rallied=0,Relieved=0,Rescued=0,Returned=0,Counter=0,Repelled=0;
};
enum class EGrandMoment : uint8 { None, Breach, Rally, Relief, Return, Counter, Repelled, Victory };
struct FGrandMoment
{
    EGrandMoment Kind=EGrandMoment::None;
    FString Title,Line,Reply;
    int Speaker=-1,FocusArmy=5,Tile=-1,Mission=-1;
    float Weight=0,Age=0;
    bool bVictory=false;
};
namespace gunyugrand
{
    FGrandMoment Compare(const FGrandProgress& Before,const FGrandProgress& After);
    bool SmokeTest();
}
