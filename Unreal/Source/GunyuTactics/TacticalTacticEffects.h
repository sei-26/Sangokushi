#pragma once
#include "TacticalPresentation.h"
namespace gunyufx
{
    using FQuadEmitter=TFunctionRef<void(FVector,FVector,FVector,FVector,FLinearColor)>;
    void Tactic(const FBattleBeat&,const TArray<FVisualUnit>&,float Progress,FQuadEmitter);
}
