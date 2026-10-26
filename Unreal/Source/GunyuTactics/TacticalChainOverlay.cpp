#include "TacticalTrial.h"
#include "TacticalChainVisual.h"
#include "TacticalChrome.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"
namespace
{
    struct FPlanLink { FVector2f From,To;FLinearColor Color;bool bSelected=false; };
    using FReadPlanLinks=TFunction<TArray<FPlanLink>()>;
    class SChainPlanOverlay : public SLeafWidget
    {
    public:
        SLATE_BEGIN_ARGS(SChainPlanOverlay) {} SLATE_ARGUMENT(FReadPlanLinks,Links) SLATE_END_ARGS()
        void Construct(const FArguments& Args) { Links=Args._Links;SetCanTick(false); }
        FVector2D ComputeDesiredSize(float) const override { return gunyulayout::MapSize; }
        int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& E,int32 Layer,const FWidgetStyle&,bool) const override
        {
            for(const auto& L:Links())
            {
                const auto Delta=L.To-L.From;const float Length=Delta.Size();if(Length<40) continue;
                const auto D=Delta/Length,Side=FVector2f(-D.Y,D.X);
                // Lift the relationship curve off the ground route, with a
                // static dash rhythm: this is a reservation, not an execution.
                auto At=[&](float T) { return FMath::Lerp(L.From+D*24,L.To-D*20,T)+Side*FMath::Sin(T*PI)*24; };
                for(int I=0;I<16;++I)
                {
                    TArray<FVector2f> Points{At(I/16.f),At((I+.55f)/16.f)};
                    FSlateDrawElement::MakeLines(E,Layer,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,FLinearColor(.002f,.009f,.011f,.8f),true,L.bSelected ? 4.f:3.f);
                    FSlateDrawElement::MakeLines(E,Layer+1,G.ToPaintGeometry(),MoveTemp(Points),ESlateDrawEffect::None,L.Color,true,L.bSelected ? 2.f:1.f);
                }
                const auto Tip=At(1),Direction=(Tip-At(.96f)).GetSafeNormal(),Across=FVector2f(-Direction.Y,Direction.X);
                TArray<FVector2f> Arrow{Tip-Direction*9+Across*5,Tip,Tip-Direction*9-Across*5};
                FSlateDrawElement::MakeLines(E,Layer+2,G.ToPaintGeometry(),MoveTemp(Arrow),ESlateDrawEffect::None,L.Color,true,2.f);
            }
            return Layer+2;
        }
    private:FReadPlanLinks Links;
    };
}
TSharedRef<SWidget> ATacticalController::ChainPlanOverlay()
{
    auto View=SNew(SChainPlanOverlay).Visibility(EVisibility::HitTestInvisible).Clipping(EWidgetClipping::ClipToBounds)
        .Links([this]
        {
            TArray<FPlanLink> Result;const auto& H=Field->Presentation();const auto& U=Field->VisualUnits();
            if(!H.bBattle || H.Journey.bVisible || Field->IsPlaying() || bSaveMenuOpen || bBattleLogOpen || bMarchPlanOpen || bTacticPickerOpen) return Result;
            for(const auto& O:H.Orders)
            {
                if(!U.IsValidIndex(O.Actor) || !U.IsValidIndex(O.Target)) continue;
                const auto& A=U[O.Actor];const auto& T=U[O.Target];
                if(A.Id<0 || T.Id<0 || A.Troops<=0 || T.Troops<=0) continue;
                const auto From=MapLabelPosition(A.Position+FVector(0,0,55),FVector2D::ZeroVector);
                const auto To=MapLabelPosition(T.Position+FVector(0,0,55),FVector2D::ZeroVector);
                if(From.X<-1000 || To.X<-1000) continue;
                const bool Selected=O.Actor==H.Selected;auto Color=gunyuchainui::Color(O);Color.A=Selected ? .95f:.4f;
                Result.Add({FVector2f(From-gunyulayout::MapOrigin),FVector2f(To-gunyulayout::MapOrigin),Color,Selected});
            }
            return Result;
        });
    View->ForceVolatile(true);return View;
}
