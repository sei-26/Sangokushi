#pragma once
#include "Widgets/SLeafWidget.h"
#include "TacticalPreparationMap.h"

DECLARE_DELEGATE_OneParam(FOnPlanningTile,int32);
class SPlanningMap : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SPlanningMap) {}
        SLATE_ATTRIBUTE(const FPreparationMap*,Map)
        SLATE_ATTRIBUTE(int32,SelectedTile)
        SLATE_EVENT(FOnPlanningTile,OnTile)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args);
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(650,400); }
    virtual int32 OnPaint(const FPaintArgs&,const FGeometry&,const FSlateRect&,FSlateWindowElementList&,int32,const FWidgetStyle&,bool) const override;
    virtual FReply OnMouseButtonDown(const FGeometry&,const FPointerEvent&) override;
    static FVector2D TilePosition(int Tile,FVector2D Size);
    static int TileAt(FVector2D Point,FVector2D Size);
private:
    TAttribute<const FPreparationMap*> Map;
    TAttribute<int32> SelectedTile;
    FOnPlanningTile OnTile;
};
