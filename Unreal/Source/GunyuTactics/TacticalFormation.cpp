#include "TacticalFormation.h"
namespace gunyuformation
{
    EShape Shape(const FVisualUnit& Unit,bool bAttacking)
    {
        if(Unit.bDisordered) return EShape::Disordered;
        if(Unit.Motion>.01f) return EShape::Column;
        if(Unit.bGuarded) return EShape::ShieldWall;
        if(Unit.bConcealed) return EShape::Hidden;
        if(bAttacking || Unit.Id==2) return EShape::Wedge;
        return EShape::Line;
    }
    FVector Offset(EShape Shape,int Soldier,int Count)
    {
        if(Shape==EShape::Wedge)
        {
            int Row=0,First=0;
            while(First+Row*2+1<=Soldier) { First+=Row*2+1;++Row; }
            return FVector((2.8f-Row)*21,(Soldier-First-Row)*14,0);
        }
        const int Files=Shape==EShape::Column ? 4:8,Rows=FMath::DivideAndRoundUp(Count,Files);
        const float Rank=Soldier/Files-(Rows-1)*.5f,File=Soldier%Files-(Files-1)*.5f;
        if(Shape==EShape::Column) return FVector(-Rank*14,File*21,0);
        if(Shape==EShape::ShieldWall) return FVector(-Rank*15,File*24,0);
        const FVector Line(-Rank*23,File*21,0);
        if(Shape==EShape::Hidden) return FVector(-Rank*20+FMath::Sin(Soldier*2.f)*5,File*24+FMath::Cos(Soldier*3.f)*5,0);
        if(Shape==EShape::Disordered) return Line*.8f+FVector(FMath::Sin(Soldier*2.7f)*12,FMath::Cos(Soldier*1.9f)*12,0);
        return Line;
    }
}
