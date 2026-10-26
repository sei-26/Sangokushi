#include "TacticalInkStroke.h"
#include "Rendering/DrawElements.h"
int32 STacticalInkStroke::OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& E,int32 Layer,const FWidgetStyle& Style,bool) const
{
    const auto C=Read();const FVector2f S=G.GetLocalSize();const float Opacity=Style.GetColorAndOpacityTint().A;
    for(int I=0;I<22;++I)
    {
        const float Y=10+(S.Y-20)*I/21.f;
        const float Rough=FMath::Sin(I*2.73f)*13;
        TArray<FVector2f> P{{8+FMath::Abs(Rough),Y+14},{S.X*.25f,Y+FMath::Sin(I*4.f)*3},{S.X*.55f,Y-3},{S.X*.80f,Y-12},{S.X-16-FMath::Abs(Rough)*3,Y-24}};
        FSlateDrawElement::MakeLines(E,Layer,G.ToPaintGeometry(),MoveTemp(P),ESlateDrawEffect::None,FLinearColor(.008f,.014f,.018f,.91f*Opacity),true,10.f+FMath::Sin(I*1.9f)*2.5f);
    }
    auto Accent=C.Accent;Accent.A=.8f*Opacity;
    for(int I=0;I<3;++I)
    {
        TArray<FVector2f> P{{S.X*.31f, S.Y-12.f+I*2},{S.X*.72f,S.Y-29.f+I*2},{S.X-16-I*9,S.Y-46.f+I*2}};
        FSlateDrawElement::MakeLines(E,Layer+1,G.ToPaintGeometry(),MoveTemp(P),ESlateDrawEffect::None,Accent,true,1.8f);
    }
    return Layer+1;
}
