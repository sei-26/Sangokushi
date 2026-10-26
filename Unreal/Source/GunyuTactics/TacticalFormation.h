#pragma once
#include "TacticalPresentation.h"
namespace gunyuformation
{
    enum class EShape { Line,Column,ShieldWall,Hidden,Disordered,Wedge };
    EShape Shape(const FVisualUnit&,bool bAttacking);
    FVector Offset(EShape,int Soldier,int Count);
}
