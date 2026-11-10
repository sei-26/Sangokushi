#pragma once
#include "CoreMinimal.h"
#include "Layout/SlateRect.h"
namespace gunyumaplabel
{
    inline const FVector2D Hidden(-2000,-2000);
    inline FVector2D GoalSize(int Index,bool bBeacons)
    { return Index==2 ? FVector2D(174,34):bBeacons ? FVector2D(180,70):FVector2D(140,34); }
    inline bool Intersects(const FSlateRect& A,const FSlateRect& B)
    {
        return A.Left<B.Right && B.Left<A.Right && A.Top<B.Bottom && B.Top<A.Bottom;
    }
    inline FVector2D Reserve(FVector2D Base,FVector2D Size,FVector2D Min,FVector2D Max,TArray<FSlateRect>& Occupied)
    {
        if(Base==Hidden) return Hidden;
        const FVector2D Limit=Max-Size;
        TArray<double,TInlineAllocator<48>> Xs{FMath::Clamp(Base.X,Min.X,Limit.X),Min.X,Limit.X};
        TArray<double,TInlineAllocator<48>> Ys{FMath::Clamp(Base.Y,Min.Y,Limit.Y),Min.Y,Limit.Y};
        for(const auto& R:Occupied)
        {
            Xs.AddUnique(FMath::Clamp(double(R.Left)-Size.X-4,Min.X,Limit.X));
            Xs.AddUnique(FMath::Clamp(double(R.Right)+4,Min.X,Limit.X));
            Ys.AddUnique(FMath::Clamp(double(R.Top)-Size.Y-4,Min.Y,Limit.Y));
            Ys.AddUnique(FMath::Clamp(double(R.Bottom)+4,Min.Y,Limit.Y));
        }
        double Best=1e30;FVector2D Chosen=Hidden;
        for(double X:Xs) for(double Y:Ys)
        {
            const FVector2D P(X,Y);const double Cost=(P-Base).SizeSquared();
            if(Cost>=Best) continue;
            const FSlateRect Bounds(X-3,Y-3,X+Size.X+3,Y+Size.Y+3);
            if(Occupied.ContainsByPredicate([&](const FSlateRect& R) { return Intersects(Bounds,R); })) continue;
            Best=Cost;Chosen=P;
        }
        if(Chosen!=Hidden) Occupied.Add(FSlateRect(Chosen.X-3,Chosen.Y-3,Chosen.X+Size.X+3,Chosen.Y+Size.Y+3));
        return Chosen;
    }
}
