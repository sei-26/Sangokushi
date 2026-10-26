#include "TacticalTrial.h"
#include "TacticalChrome.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"
namespace
{
    using FReadSegments=TFunction<TArray<TPair<FVector2D,FVector2D>>()>;
    class SUnitLabelLeaders : public SLeafWidget
    {
    public:
        SLATE_BEGIN_ARGS(SUnitLabelLeaders) {} SLATE_ARGUMENT(FReadSegments,Segments) SLATE_END_ARGS()
        void Construct(const FArguments& Args) { Segments=Args._Segments;SetCanTick(false); }
        FVector2D ComputeDesiredSize(float) const override { return gunyulayout::MapSize; }
        int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& E,int32 Layer,const FWidgetStyle&,bool) const override
        {
            for(const auto& Line:Segments())
            {
                TArray<FVector2f> Points{FVector2f(Line.Key-gunyulayout::MapOrigin),FVector2f(Line.Value-gunyulayout::MapOrigin)};
                FSlateDrawElement::MakeLines(E,Layer,G.ToPaintGeometry(),MoveTemp(Points),ESlateDrawEffect::None,FLinearColor(.8f,.73f,.48f,.75f),true,1.f);
            }
            return Layer;
        }
    private: TFunction<TArray<TPair<FVector2D,FVector2D>>()> Segments;
    };
}
TSharedRef<SWidget> ATacticalController::MapLabelLeaderOverlay()
{
    auto View=SNew(SUnitLabelLeaders).Visibility(EVisibility::HitTestInvisible).Clipping(EWidgetClipping::ClipToBounds)
        .Segments([this]
        {
            if(LabelLayoutFrame!=GFrameCounter) LayoutMapLabels();
            return bSaveMenuOpen || bBattleLogOpen || bMarchPlanOpen ? TArray<TPair<FVector2D,FVector2D>>{} : UnitLabelLeaders;
        });
    View->ForceVolatile(true);return View;
}
