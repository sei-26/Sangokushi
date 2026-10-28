#include "TacticalPlanningMap.h"
#include "CampaignTypes.hpp"
#include "HexGrid.hpp"
#include "TacticalStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

namespace
{
    double ScaleFor(FVector2D Size) { return FMath::Max(1.,FMath::Min((Size.X-36)/14.5,(Size.Y-34)/10.7)); }
    FVector2D OriginFor(FVector2D Size) { const double S=ScaleFor(Size);return (Size-FVector2D(14.5,10.7)*S)*.5; }
    FLinearColor TerrainColor(uint8 CellKind)
    {
        using frontline::Terrain;
        if(CellKind==uint8(Terrain::Forest)) return FLinearColor(.07f,.18f,.115f);
        if(CellKind==uint8(Terrain::Mountain)) return FLinearColor(.21f,.23f,.215f);
        if(CellKind==uint8(Terrain::River)) return FLinearColor(.035f,.17f,.23f);
        if(CellKind==uint8(Terrain::Bridge)) return FLinearColor(.38f,.27f,.13f);
        return FLinearColor(.15f,.20f,.12f);
    }
}
void SPlanningMap::Construct(const FArguments& Args)
{ Map=Args._Map;SelectedTile=Args._SelectedTile;SelectedOfficer=Args._SelectedOfficer;PreviewRoute=Args._PreviewRoute;OnTile=Args._OnTile;SetCanTick(false); }
FVector2D SPlanningMap::TilePosition(int Tile,FVector2D Size)
{
    const auto P=hexgrid::Center(Tile%frontline::Width,Tile/frontline::Width);
    return OriginFor(Size)+(FVector2D(P.first,P.second)-FVector2D(1,hexgrid::Row))*ScaleFor(Size);
}
int SPlanningMap::TileAt(FVector2D Point,FVector2D Size)
{
    const auto P=(Point-OriginFor(Size))/ScaleFor(Size)+FVector2D(1,hexgrid::Row);
    const auto Cell=hexgrid::Pick(P.X,P.Y);
    return Cell.first>=1 && Cell.first<=14 && Cell.second>=1 && Cell.second<=12 ? Cell.first+Cell.second*frontline::Width:-1;
}
FReply SPlanningMap::OnMouseButtonDown(const FGeometry& G,const FPointerEvent& Event)
{
    if(Event.GetEffectingButton()==EKeys::LeftMouseButton)
        OnTile.ExecuteIfBound(TileAt(G.AbsoluteToLocal(Event.GetScreenSpacePosition()),G.GetLocalSize()));
    return FReply::Handled();
}
int32 SPlanningMap::OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& E,int32 Layer,const FWidgetStyle&,bool) const
{
    const auto* Data=Map.Get(nullptr);if(!Data) return Layer;
    const FVector2D Size=G.GetLocalSize();const float Scale=ScaleFor(Size);
    const auto* White=FCoreStyle::Get().GetBrush("WhiteBrush");
    FSlateDrawElement::MakeBox(E,Layer,G.ToPaintGeometry(),White,ESlateDrawEffect::None,FLinearColor(.018f,.031f,.029f));
    auto Point=[&](int Tile) { return FVector2f(TilePosition(Tile,Size)); };
    auto Line=[&](TArray<FVector2f> P,FLinearColor Color,float Thickness=1.f,int Offset=2)
    { FSlateDrawElement::MakeLines(E,Layer+Offset,G.ToPaintGeometry(),MoveTemp(P),ESlateDrawEffect::None,Color,true,Thickness); };
    auto Box=[&](FVector2f P,FVector2f Extent,FLinearColor Color,int Offset)
    { FSlateDrawElement::MakeBox(E,Layer+Offset,G.ToPaintGeometry(Extent,FSlateLayoutTransform(P-Extent*.5f)),White,ESlateDrawEffect::None,Color); };
    auto Text=[&](FString Value,FVector2f P,FLinearColor Color,int Offset=5)
    { FSlateDrawElement::MakeText(E,Layer+Offset,G.ToPaintGeometry(FVector2f(80,24),FSlateLayoutTransform(P)),Value,gunyustyle::Font(14,true),ESlateDrawEffect::None,Color); };
    TArray<FSlateVertex> Vertices;TArray<SlateIndex> Indices;
    for(const auto& Cell:Data->Cells)
    {
        const FVector2f C=Point(Cell.Tile);const SlateIndex First=Vertices.Num();
        const auto Color=TerrainColor(Cell.Terrain).ToFColor(true);
        Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),C,FVector2f(.5f),Color));
        TArray<FVector2f> Outline;
        for(int Side=0;Side<6;++Side)
        {
            const float A=FMath::DegreesToRadians(30.f+Side*60.f);
            const FVector2f P=C+FVector2f(FMath::Cos(A),FMath::Sin(A))*Scale*hexgrid::Radius;
            Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),P,FVector2f(.5f),Color));Outline.Add(P);
        }
        for(int Side=0;Side<6;++Side) { Indices.Add(First);Indices.Add(First+1+Side);Indices.Add(First+1+(Side+1)%6); }
        const FVector2f FirstCorner=Outline[0];Outline.Add(FirstCorner);Line(MoveTemp(Outline),FLinearColor(.34f,.39f,.31f,.55f),.7f);
        using frontline::Terrain;
        if(Cell.Terrain==uint8(Terrain::Mountain))
            Line({C+FVector2f(-.30f,.15f)*Scale,C+FVector2f(-.07f,-.24f)*Scale,C+FVector2f(.19f,.15f)*Scale,C+FVector2f(.33f,-.06f)*Scale},FLinearColor(.51f,.54f,.48f),1.f);
        if(Cell.Terrain==uint8(Terrain::Forest))
            for(float X:{-.16f,.16f}) Line({C+FVector2f(X-.13f,.12f)*Scale,C+FVector2f(X,-.20f)*Scale,C+FVector2f(X+.13f,.12f)*Scale},FLinearColor(.25f,.43f,.25f),1.f);
        if(Cell.Terrain==uint8(Terrain::Bridge))
        {
            bool Vertical=false;
            for(const auto& Other:Data->Cells)
                if(Other.Terrain==uint8(Terrain::River) && Other.Tile%frontline::Width==Cell.Tile%frontline::Width && FMath::Abs(Other.Tile/frontline::Width-Cell.Tile/frontline::Width)==1) Vertical=true;
            for(float Step:{-.18f,0.f,.18f})
                Line(Vertical ? TArray<FVector2f>{C+FVector2f(Step,-.31f)*Scale,C+FVector2f(Step,.31f)*Scale}:
                    TArray<FVector2f>{C+FVector2f(-.31f,Step)*Scale,C+FVector2f(.31f,Step)*Scale},FLinearColor(.78f,.60f,.34f),2.f);
        }
    }
    FSlateDrawElement::MakeCustomVerts(E,Layer+1,FSlateApplication::Get().GetRenderer()->GetResourceHandle(*White),Vertices,Indices,nullptr,0,0);
    const FLinearColor Gold(.96f,.74f,.32f),Teal(.25f,.83f,.79f);
    for(int I=1;I<Data->ConvoyRoute.Num();++I)
    {
        const FVector2f A=Point(Data->ConvoyRoute[I-1]),B=Point(Data->ConvoyRoute[I]),D=(B-A).GetSafeNormal();
        Line({A,B},Gold,2.5f,3);const FVector2f P=FMath::Lerp(A,B,.65f),N(-D.Y,D.X);
        Line({P-D*6+N*3,P,P-D*6-N*3},Gold,1.5f,3);
    }
    const auto Route=PreviewRoute.Get(TArray<int>());
    for(int I=1;I<Route.Num();++I)
    {
        const FVector2f A=Point(Route[I-1]),B=Point(Route[I]),D=(B-A).GetSafeNormal(),N(-D.Y,D.X);
        Line({A,B},Teal,3.f,3);
        const FVector2f Mid=FMath::Lerp(A,B,.55f);
        Line({Mid-D*5+N*3,Mid,Mid-D*5-N*3},Teal,1.5f,3);
    }
    const int Officer=SelectedOfficer.Get(-1);
    if(const auto* Start=Data->Starts.FindByPredicate([Officer](const FPlanningStart& S) { return S.Officer==Officer && Officer>=0; }))
    {
        const FVector2f C=Point(Start->Tile);TArray<FVector2f> Ring;
        for(int Side=0;Side<=6;++Side) { const float Angle=FMath::DegreesToRadians(30.f+Side*60.f);Ring.Add(C+FVector2f(FMath::Cos(Angle),FMath::Sin(Angle))*Scale*hexgrid::Radius); }
        Line(MoveTemp(Ring),Teal,3.f,4);
    }
    const int Selected=SelectedTile.Get(-1);
    if(Selected>=0)
    {
        TArray<FVector2f> Outline;const FVector2f C=Point(Selected);
        for(int Side=0;Side<=6;++Side) { const float A=FMath::DegreesToRadians(30.f+Side*60.f);Outline.Add(C+FVector2f(FMath::Cos(A),FMath::Sin(A))*Scale*hexgrid::Radius); }
        Line(MoveTemp(Outline),Gold,2.5f,4);
    }
    const TCHAR* Names[]={TEXT("劉"),TEXT("関"),TEXT("張")};
    for(const auto& Start:Data->Starts)
    {
        if(Data->Duties.ContainsByPredicate([&Start](const FPlanningDuty& D) { return D.Kind>0 && D.Tile==Start.Tile && D.Officer==Start.Officer; })) continue;
        const FVector2f P=Point(Start.Tile)+FVector2f(0,.22f)*Scale;
        Box(P,FVector2f(18,18),FLinearColor(.025f,.21f,.22f),4);
        Text(Start.Officer<0 ? TEXT("車"):Names[Start.Officer],P-FVector2f(7,9),Teal);
    }
    for(const auto& Duty:Data->Duties)
    {
        if(Duty.Kind==0 || Duty.Officer<0 || Duty.Officer>=3) continue;
        const FVector2f P=Point(Duty.Tile);
        const int Site=Data->Sites.IndexOfByPredicate([&Duty](const FPlanningSite& S) { return S.Tile==Duty.Tile; });
        const FString Caption=(Site>=0 ? FString::FromInt(Site+1):FString())+Names[Duty.Officer]+(Duty.Kind==1 ? TEXT("守"):TEXT("伏"));
        const FVector2f Badge(Site>=0 ? 38.f:30.f,19.f);
        const bool StartsHere=Data->Starts.ContainsByPredicate([&Duty](const FPlanningStart& S) { return S.Officer==Duty.Officer && S.Tile==Duty.Tile; });
        if(StartsHere) Box(P,Badge+FVector2f(3,3),Teal,4);
        Box(P,Badge,FLinearColor(.32f,.065f,.035f),4);
        Text(Caption,P-FVector2f(Site>=0 ? 18.f:13.f,9),FLinearColor(1.f,.76f,.55f));
    }
    for(int I=0;I<Data->Sites.Num();++I)
    {
        const int Tile=Data->Sites[I].Tile;
        if(Data->Duties.ContainsByPredicate([Tile](const FPlanningDuty& D) { return D.Kind>0 && D.Tile==Tile; })) continue;
        const FVector2f P=Point(Tile)+FVector2f(.25f,-.28f)*Scale;
        Box(P,FVector2f(16,16),FLinearColor(.24f,.17f,.06f),4);
        Text(FString::FromInt(I+1),P-FVector2f(4,9),Gold);
    }
    Text(TEXT("北 ↑"),FVector2f(12,8),Gold);
    return Layer+6;
}
