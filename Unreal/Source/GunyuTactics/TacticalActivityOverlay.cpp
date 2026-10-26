#include "TacticalTrial.h"
#include "TacticalActivity.h"
#include "TacticalChrome.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"
namespace
{
    struct FScreenMark { TArray<FVector2f> Points;FLinearColor Color; };
    using FReadMarks=TFunction<TArray<FScreenMark>()>;
    class SBattleActivity : public SLeafWidget
    {
    public:
        SLATE_BEGIN_ARGS(SBattleActivity) {} SLATE_ARGUMENT(FReadMarks,Marks) SLATE_END_ARGS()
        void Construct(const FArguments& A) { Marks=A._Marks;SetCanTick(false); }
        FVector2D ComputeDesiredSize(float) const override { return gunyulayout::MapSize; }
        int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& E,int32 L,const FWidgetStyle&,bool) const override
        {
            for(const auto& Mark:Marks())
            {
                FSlateDrawElement::MakeLines(E,L,G.ToPaintGeometry(),Mark.Points,ESlateDrawEffect::None,FLinearColor(.015f,.02f,.015f,.9f),true,4.f);
                FSlateDrawElement::MakeLines(E,L+1,G.ToPaintGeometry(),Mark.Points,ESlateDrawEffect::None,Mark.Color,true,2.f);
            }
            return L+1;
        }
    private:FReadMarks Marks;
    };
}
TSharedRef<SWidget> ATacticalController::BattleActivityOverlay()
{
    auto View=SNew(SBattleActivity).Visibility(EVisibility::HitTestInvisible).Clipping(EWidgetClipping::ClipToBounds)
      .Marks([this]
      {
          TArray<FScreenMark> Result;
          if(!Field->IsPlaying() || bSaveMenuOpen || bMarchPlanOpen || bBattleLogOpen || Field->Presentation().Journey.bVisible) return Result;
          for(const auto& Mark:gunyuactivity::Markers(Field->VisualUnits(),Field->CurrentBeat(),Field->BeatProgress()))
          {
              FScreenMark S;S.Color=Mark.Color;bool Valid=true;
              for(auto P:Mark.Points)
              {
                  P.Z=Field->SurfaceHeight(P)+14;
                  const auto Q=MapLabelPosition(P,FVector2D::ZeroVector);
                  if(Q.X<-1000) { Valid=false;break; }
                  S.Points.Add(FVector2f(Q-gunyulayout::MapOrigin));
              }
              if(Valid) Result.Add(MoveTemp(S));
          }
          return Result;
      });
    View->ForceVolatile(true);return View;
}
