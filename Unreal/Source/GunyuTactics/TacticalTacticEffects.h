#pragma once
#include "TacticalPresentation.h"
namespace gunyufx
{
    using FQuadEmitter=TFunctionRef<void(FVector,FVector,FVector,FVector,FLinearColor)>;
    void Signature(const FBattleBeat&,const TArray<FVisualUnit>&,float Progress,FQuadEmitter);
    void Tactic(const FBattleBeat&,const TArray<FVisualUnit>&,float Progress,FQuadEmitter);
}
