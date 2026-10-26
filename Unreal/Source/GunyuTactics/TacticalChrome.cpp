#include "TacticalChrome.h"
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
    Stops.Add(FSlateGradientStop(FVector2f(0,0),FLinearColor(.028f,.040f,.046f,Opacity)));
    Stops.Add(FSlateGradientStop(FVector2f(0,S.Y),FLinearColor(.007f,.013f,.020f,Opacity)));
    FSlateDrawElement::MakeGradient(E,Layer,G.ToPaintGeometry(),MoveTemp(Stops),Orient_Vertical);
    auto Line=[&](TArray<FVector2f> Points,FLinearColor Color,float Width=1.f)
    { FSlateDrawElement::MakeLines(E,Layer+1,G.ToPaintGeometry(),MoveTemp(Points),ESlateDrawEffect::None,Color,true,Width); };
    Line({{1,12},{1,S.Y-12},{12,S.Y-1},{S.X-12,S.Y-1},{S.X-1,S.Y-12},{S.X-1,12},{S.X-12,1},{12,1},{1,12}},Accent*.55f);
    Line({{12,1},{FMath::Min(90.f,S.X-12),1}},Accent,2.f);
    Line({{S.X-34,S.Y-3},{S.X-12,S.Y-3},{S.X-3,S.Y-12}},Accent*.8f);
    Line({{12,5},{S.X-12,5}},FLinearColor(.27f,.28f,.25f,.14f));
    return SCompoundWidget::OnPaint(Args,G,Cull,E,Layer+2,Style,Enabled);
}
