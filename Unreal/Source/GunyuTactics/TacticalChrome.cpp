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
    Stops.Add(FSlateGradientStop(FVector2f(0,0),FLinearColor(.105f,.037f,.055f,Opacity)));
    Stops.Add(FSlateGradientStop(FVector2f(0,S.Y),FLinearColor(.018f,.028f,.047f,Opacity)));
    FSlateDrawElement::MakeGradient(E,Layer,G.ToPaintGeometry(),MoveTemp(Stops),Orient_Vertical);
    auto Line=[&](TArray<FVector2f> Points,FLinearColor Color,float Width=1.f)
    { FSlateDrawElement::MakeLines(E,Layer+1,G.ToPaintGeometry(),MoveTemp(Points),ESlateDrawEffect::None,Color,true,Width); };
    const FLinearColor Gold=gunyustyle::Brass;
    Line({{1,1},{S.X-1,1},{S.X-1,S.Y-1},{1,S.Y-1},{1,1}},Gold*.55f);
    if(S.X>100 && S.Y>60)
    {
        Line({{5,18},{5,5},{26,5}},Gold*.9f,1.4f);
        Line({{S.X-26,5},{S.X-5,5},{S.X-5,18}},Gold*.9f,1.4f);
        Line({{5,S.Y-18},{5,S.Y-5},{26,S.Y-5}},Gold*.65f,1.4f);
        Line({{S.X-26,S.Y-5},{S.X-5,S.Y-5},{S.X-5,S.Y-18}},Gold*.65f,1.4f);
        const float C=S.X*.5f;
        Line({{C-36,1},{C-8,1},{C,6},{C+8,1},{C+36,1}},Gold,1.6f);
        Line({{1,24},{1,FMath::Min(66.f,S.Y-20)}},Accent,2.f);
    }
    return SCompoundWidget::OnPaint(Args,G,Cull,E,Layer+2,Style,Enabled);
}
