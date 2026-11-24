#include "TacticalTrial.h"
#include "TacticalChrome.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"
namespace
{
    using FReadSegments=TFunction<TArray<gunyumaplabel::FLeader>()>;
    class SMapLabelLeaders : public SLeafWidget
    {
    public:
        SLATE_BEGIN_ARGS(SMapLabelLeaders) {} SLATE_ARGUMENT(FReadSegments,Segments) SLATE_END_ARGS()
        void Construct(const FArguments& Args) { Segments=Args._Segments;SetCanTick(false); }
        FVector2D ComputeDesiredSize(float) const override { return gunyulayout::LabelSize; }
        int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& E,int32 Layer,const FWidgetStyle&,bool) const override
        {
            for(const auto& Line:Segments())
            {
                const FVector2f From(Line.From-gunyulayout::LabelOrigin),To(Line.To-gunyulayout::LabelOrigin);
                const TArray<FVector2f> Points{From,To};
                FSlateDrawElement::MakeLines(E,Layer,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,FLinearColor(.005f,.012f,.015f,.85f),true,Line.Width+2.f);
                FSlateDrawElement::MakeLines(E,Layer+1,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,Line.Color,true,Line.Width);
                const TArray<FVector2f> Marker{From+FVector2f(0,-3),From+FVector2f(3,0),From+FVector2f(0,3),From+FVector2f(-3,0),From+FVector2f(0,-3)};
                FSlateDrawElement::MakeLines(E,Layer+2,G.ToPaintGeometry(),Marker,ESlateDrawEffect::None,Line.Color,true,1.2f);
            }
            return Layer+2;
        }
    private: FReadSegments Segments;
    };
}
TSharedRef<SWidget> ATacticalController::MapLabelLeaderOverlay()
{
    auto View=SNew(SMapLabelLeaders).Visibility(EVisibility::HitTestInvisible).Clipping(EWidgetClipping::ClipToBounds)
        .Segments([this]
        {
            if(LabelLayoutFrame!=GFrameCounter) LayoutMapLabels();
            return bSaveMenuOpen || bBattleLogOpen || bMarchPlanOpen ? TArray<gunyumaplabel::FLeader>{} : MapLabelLeaders;
        });
    View->ForceVolatile(true);return View;
}
