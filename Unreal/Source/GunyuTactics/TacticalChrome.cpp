#include "TacticalChrome.h"
#include "TacticalStyle.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void STacticalChrome::Construct(const FArguments& Args)
{
    Accent=Args._Accent;Opacity=Args._Opacity;SetCanTick(false);
    ChildSlot.Padding(Args._Padding)[Args._Content.Widget];
}
int32 STacticalChrome::OnPaint(const FPaintArgs& Args,const FGeometry& G,const FSlateRect& Cull,
    FSlateWindowElementList& E,int32 Layer,const FWidgetStyle& Style,bool Enabled) const
{
    const FVector2f S=G.GetLocalSize();
    TArray<FSlateGradientStop> Stops;
    Stops.Add(FSlateGradientStop(FVector2f(0,0),FLinearColor(.024f,.028f,.028f,Opacity)));
    Stops.Add(FSlateGradientStop(FVector2f(0,S.Y),FLinearColor(.008f,.012f,.015f,Opacity)));
    FSlateDrawElement::MakeGradient(E,Layer,G.ToPaintGeometry(),MoveTemp(Stops),Orient_Vertical);
    auto Line=[&](TArray<FVector2f> Points,FLinearColor Color,float Width=1.f)
    { FSlateDrawElement::MakeLines(E,Layer+1,G.ToPaintGeometry(),MoveTemp(Points),ESlateDrawEffect::None,Color,true,Width); };
    Line({{1,1},{S.X-1,1},{S.X-1,S.Y-1},{1,S.Y-1},{1,1}},Accent*.28f);
    Line({{16,1},{FMath::Min(64.f,S.X-16),1}},Accent*.9f,1.5f);
    Line({{1,16},{1,FMath::Min(42.f,S.Y-16)}},gunyustyle::Vermilion,2.f);
    return SCompoundWidget::OnPaint(Args,G,Cull,E,Layer+2,Style,Enabled);
}
