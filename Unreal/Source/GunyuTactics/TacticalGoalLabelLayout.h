#pragma once
#include "CoreMinimal.h"
namespace gunyugoallabel
{
    inline FVector2D Place(FVector2D Position,FVector2D Size,bool bResponse)
    {
        // Share the response panel's existing reserved rectangle with the goal and unit plates.
        if(bResponse && Position.X<732 && Position.X+Size.X>32 && Position.Y<260 && Position.Y+Size.Y>120)
            Position.Y=264;
        return Position;
    }
}
