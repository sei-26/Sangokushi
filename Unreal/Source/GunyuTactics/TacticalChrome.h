#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

// Small shared paint surface: lacquer, brass rules and cut-corner ornaments.
// Code-drawn at the viewport's actual resolution; no stretched background image.
class STacticalChrome : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STacticalChrome) : _Padding(12),_Accent(.48f,.35f,.14f),_Opacity(.96f) {}
        SLATE_ARGUMENT(FMargin,Padding)
        SLATE_ARGUMENT(FLinearColor,Accent)
        SLATE_ARGUMENT(float,Opacity)
        SLATE_DEFAULT_SLOT(FArguments,Content)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args);
    int32 OnPaint(const FPaintArgs&,const FGeometry&,const FSlateRect&,FSlateWindowElementList&,int32,const FWidgetStyle&,bool) const override;
private:
    FLinearColor Accent;
    float Opacity=1;
};
namespace gunyulayout
{
    inline const FVector2D MapOrigin(16,174),MapSize(1240,538);
    inline const FVector2D CommandOrigin(1268,116),CommandSize(316,578);
}
