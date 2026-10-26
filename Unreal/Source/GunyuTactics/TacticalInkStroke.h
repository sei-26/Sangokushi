#pragma once
#include "Widgets/SLeafWidget.h"
#include "TacticalPresentation.h"
class STacticalInkStroke : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STacticalInkStroke) {} SLATE_ARGUMENT(TFunction<FTacticCinematicRead()>,Read) SLATE_END_ARGS()
    void Construct(const FArguments& Args) { Read=Args._Read;SetCanTick(false); }
    FVector2D ComputeDesiredSize(float) const override { return FVector2D(560,220); }
    int32 OnPaint(const FPaintArgs&,const FGeometry&,const FSlateRect&,FSlateWindowElementList&,int32,const FWidgetStyle&,bool) const override;
private:TFunction<FTacticCinematicRead()> Read;
};
