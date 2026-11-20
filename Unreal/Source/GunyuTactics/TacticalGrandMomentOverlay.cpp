#include "TacticalTrial.h"
#include "TacticalChrome.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"
namespace
{
    struct FCommandWave { TArray<FVector2f> Points;FLinearColor Color; };
    class SGrandWaves : public SLeafWidget
    {
    public:
        SLATE_BEGIN_ARGS(SGrandWaves) {} SLATE_ARGUMENT(TFunction<TArray<FCommandWave>()>,Read) SLATE_END_ARGS()
        void Construct(const FArguments& A) { Read=A._Read;SetCanTick(false); }
        FVector2D ComputeDesiredSize(float) const override { return gunyulayout::MapSize; }
        int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& E,int32 L,const FWidgetStyle&,bool) const override
        {
            for(const auto& W:Read())
            {
                auto Glow=W.Color;Glow.A*=.18f;
                FSlateDrawElement::MakeLines(E,L,G.ToPaintGeometry(),W.Points,ESlateDrawEffect::None,Glow,true,11.f);
                FSlateDrawElement::MakeLines(E,L+1,G.ToPaintGeometry(),W.Points,ESlateDrawEffect::None,W.Color,true,2.3f);
            }
            return L+1;
        }
    private:TFunction<TArray<FCommandWave>()> Read;
    };
}
TSharedRef<SWidget> ATacticalController::GrandMomentOverlay()
{
    auto View=SNew(SGrandWaves).Visibility(EVisibility::HitTestInvisible).Clipping(EWidgetClipping::ClipToBounds).Read([this]
    {
        TArray<FCommandWave> Waves;const auto R=Field->ReadGrandMoment();const auto& Units=Field->VisualUnits();
        if(!bTacticCinema || R.Weight<=0 || Field->Presentation().Journey.bVisible || bSaveMenuOpen || bEpisodeOpen || bMarchPlanOpen || bBattleLogOpen ||
            !Units.IsValidIndex(R.FocusArmy) || Units[R.FocusArmy].Id<0 || Units[R.FocusArmy].Troops<=0) return Waves;
        for(int Ring=0;Ring<2;++Ring)
        {
            const float Phase=FMath::Frac(R.Age*.45f+Ring*.5f),Radius=120+Phase*390;
            FCommandWave W;W.Color=R.Kind==EGrandMoment::Counter ? FLinearColor(1,.28f,.12f):FLinearColor(1,.79f,.25f);
            W.Color.A=R.Weight*(1-Phase)*.8f;
            for(int I=0;I<=64;++I)
            {
                const float Angle=I*2*PI/64;FVector P=Units[R.FocusArmy].Position+FVector(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,0);
                P.Z=Field->SurfaceHeight(P)+18;
                const auto Screen=MapLabelPosition(P,FVector2D::ZeroVector);
                if(Screen.X<-1000) { W.Points.Reset();break; }
                W.Points.Add(FVector2f(Screen-gunyulayout::MapOrigin));
            }
            if(W.Points.Num()>1) Waves.Add(MoveTemp(W));
        }
        return Waves;
    });
    View->ForceVolatile(true);return View;
}
