#include "TacticalChrome.h"
#include "TacticalTrial.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElementTypes.h"

namespace
{
    class SMarchRouteOverlay : public SLeafWidget
    {
    public:
        SLATE_BEGIN_ARGS(SMarchRouteOverlay) {} SLATE_ARGUMENT(TFunction<TArray<FVector2f>()>, Points) SLATE_ARGUMENT(FLinearColor, Color) SLATE_END_ARGS()
        void Construct(const FArguments& Args) { Points=Args._Points;Color=Args._Color;SetCanTick(false); }
        FVector2D ComputeDesiredSize(float) const override { return gunyulayout::MapSize; }
        int32 OnPaint(const FPaintArgs&,const FGeometry& Geometry,const FSlateRect&,FSlateWindowElementList& Elements,int32 Layer,const FWidgetStyle&,bool) const override
        {
            const auto Path=Points();if (Path.Num()<2) return Layer;
            const auto Paint=Geometry.ToPaintGeometry();
            FSlateDrawElement::MakeLines(Elements,Layer,Paint,Path,ESlateDrawEffect::None,FLinearColor(.08f,.045f,.01f,.9f),true,4.f);
            FSlateDrawElement::MakeLines(Elements,Layer+1,Paint,Path,ESlateDrawEffect::None,Color,true,1.8f);
            for (int I=1;I<Path.Num();++I)
            {
                const FVector2f Delta=Path[I]-Path[I-1];if (Delta.SizeSquared()<36) continue;
                const auto Direction=Delta.GetSafeNormal(),Side=FVector2f(-Direction.Y,Direction.X);
                const auto Tip=(Path[I]+Path[I-1])*.5f;
                TArray<FVector2f> Arrow{Tip-Direction*7.f+Side*4.f,Tip,Tip-Direction*7.f-Side*4.f};
                FSlateDrawElement::MakeLines(Elements,Layer+2,Paint,MoveTemp(Arrow),ESlateDrawEffect::None,Color,true,1.6f);
            }
            return Layer+2;
        }
    private:
        TFunction<TArray<FVector2f>()> Points;
        FLinearColor Color;
    };
}
TSharedRef<SWidget> ATacticalController::MarchRouteOverlay(bool bConvoy)
{
    auto View=SNew(SMarchRouteOverlay).Visibility(EVisibility::HitTestInvisible).Clipping(EWidgetClipping::ClipToBounds)
        .Color(bConvoy ? FLinearColor(.2f,.95f,.84f,.9f) : FLinearColor(1.f,.81f,.28f,.95f))
        .Points([this,bConvoy]
        {
            TArray<FVector2f> Points;const auto& H=Field->Presentation();
            if (!Field->bRealtimeCommand || !H.bBattle || bMarchPlanOpen || bBattleLogOpen) return Points;
            for (int Tile : bConvoy ? H.ConvoyRoute : H.MarchRoute)
            {
                const FVector2D Point=MapLabelPosition(Field->SurfaceLocation(Tile)+FVector(0,0,15),FVector2D::ZeroVector);
                if (Point.X<=-1000) return TArray<FVector2f>{};
                Points.Add(FVector2f(Point-gunyulayout::MapOrigin));
            }
            return Points;
        });
    View->ForceVolatile(true);return View;
}
