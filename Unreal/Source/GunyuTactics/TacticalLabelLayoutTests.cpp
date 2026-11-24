#include "TacticalTrial.h"
#include "TacticalChrome.h"
#include "TacticalTargetVisual.h"
#include "JourneySave.hpp"
#include <memory>

bool ATacticalController::LabelLayoutSmokeTest(bool bExerciseInput)
{
    auto Check=[](bool V,const TCHAR* Why){if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_LABEL_LAYOUT: FAIL %s"),Why);return V;};
    if(!Check(Field && Field->Presentation().bBattle && !Field->IsPlaying(),TEXT("a settled real battle supplies the rendered label test"))) return false;
    std::vector<std::uint8_t> Before,After;
    if(!Field->ExportSession(Before)) return false;
    LayoutMapLabels();const auto First=UnitLabelPositions;const auto Goals=GoalLabelPositions;
    LayoutMapLabels();
    if(!Check(First==UnitLabelPositions && Goals==GoalLabelPositions,TEXT("an unchanged rendered scene keeps every label in the same place"))) return false;
    const auto& Units=Field->VisualUnits();
    TArray<FSlateRect> Rects;int Shown=0,Short=0;double TotalLeaderDistance=0,Maximum=0;
    for(int Id=0;Id<Units.Num();++Id)
    {
        const auto& U=Units[Id];const auto P=LabelPosition(Id),S=UnitLabelSize(Id);
        if(U.Id<0 || U.Troops<=0)
        { if(!Check(P==gunyumaplabel::Hidden,TEXT("unknown and absent formations receive no label"))) return false;continue; }
        const auto Anchor=MapLabelPosition(U.Position+FVector(0,0,70),FVector2D::ZeroVector);
        const bool OnScreen=Anchor.X>=gunyulayout::MapOrigin.X && Anchor.X<gunyulayout::MapOrigin.X+gunyulayout::MapSize.X &&
            Anchor.Y>=gunyulayout::MapOrigin.Y && Anchor.Y<gunyulayout::MapOrigin.Y+gunyulayout::MapSize.Y;
        if(OnScreen && !Check(P!=gunyumaplabel::Hidden,TEXT("visible formations retain their own accessible nameplates"))) return false;
        if(P==gunyumaplabel::Hidden) continue;
        ++Shown;const FSlateRect R(P.X,P.Y,P.X+S.X,P.Y+S.Y);
        if(!Check(P.X>=16 && P.Y>=174 && R.Right<=1256.5 && R.Bottom<=712.5,TEXT("labels stay inside the playable view and clear of the command panel"))) return false;
        for(const auto& Other:Rects) if(!Check(!gunyumaplabel::Intersects(R,Other),TEXT("real nameplates do not overlap each other"))) return false;
        Rects.Add(R);
        for(const auto& V:Units) if(V.Id>=0 && V.Troops>0)
        {
            const auto Body=MapLabelPosition(V.Position+FVector(0,0,60),FVector2D::ZeroVector);
            if(!Check(!(Body.X>=R.Left && Body.X<=R.Right && Body.Y>=R.Top && Body.Y<=R.Bottom),TEXT("nameplates do not conceal displayed formation centers"))) return false;
        }
        if(!Check(UnitLabelContents.IsValidIndex(Id) && UnitLabelContents[Id].IsValid() && UnitLabelContents[Id]->GetDesiredSize().Y+2<=S.Y+.5,
            TEXT("the rendered name, status and actual loss fit inside the allocated plate"))) return false;
        if(Id<3 && U.Loss==0)
        { ++Short;if(!Check(S.Y<82,TEXT("a loss-free officer has no reserved empty casualty row"))) return false; }
        const auto Edge=gunyumaplabel::NearestEdge(Anchor,P,S);const double Length=FVector2D::Distance(Anchor,Edge);
        TotalLeaderDistance+=Length;Maximum=FMath::Max(Maximum,Length);
        if(Length>5)
        {
            const auto* Leader=MapLabelLeaders.FindByPredicate([Id](const gunyumaplabel::FLeader& L){return L.Army==Id;});
            if(!Check(Leader && Leader->From.Equals(Anchor,.1) && Leader->To.Equals(Edge,.1),TEXT("each connection starts at its displayed formation and ends on its own nearest edge"))) return false;
        }
    }
    for(int I=0;I<GoalLabelPositions.Num();++I)
        if(GoalLabelPositions[I]!=gunyumaplabel::Hidden)
        {
            const auto P=GoalLabelPositions[I],S=GoalLabelSize(I);const FSlateRect G(P.X,P.Y,P.X+S.X,P.Y+S.Y);
            for(const auto& R:Rects) if(!Check(!gunyumaplabel::Intersects(R,G),TEXT("unit names and mission plates remain separate"))) return false;
        }
    if(!Check(Shown>0 && Short>0 && Field->ExportSession(After) && Before==After,TEXT("measuring, arranging and reading labels cannot spend resources or edit the session"))) return false;
    if(bExerciseInput)
    {
        auto A=std::make_unique<frontline::JourneySaveData>(),B=std::make_unique<frontline::JourneySaveData>();
        if(!frontline::DecodeJourneySave(Before,*A)) return false;
        const auto OldFocus=Focus;const bool EnemyDetails=bEnemyDetailsOpen;
        SelectMapUnit(1);
        if(!Check(Field->Presentation().Selected==1,TEXT("the Guan Yu plate selects Guan Yu after compaction"))) return false;
        int Enemy=-1;for(int Id=3;Id<Units.Num();++Id) if(Units[Id].Id>=0 && Units[Id].bEnemy && Units[Id].Troops>0){Enemy=Id;break;}
        if(Enemy>=0)
        {
            SelectMapUnit(Enemy);
            if(!Check(bEnemyDetailsOpen && gunyutargetui::MapTargetSelected(Field->Presentation(),Field->VisualUnits()[Enemy]),TEXT("an enemy plate selects and highlights that observed enemy"))) return false;
        }
        if(!Field->ExportSession(After) || !frontline::DecodeJourneySave(After,*B)) return false;
        B->selected=A->selected;B->target=A->target;
        std::vector<std::uint8_t> Normalized;
        if(!Check(frontline::EncodeJourneySave(*B,Normalized) && Before==Normalized,TEXT("plate selection changes only the selected officer and observed target"))) return false;
        if(!Field->ImportSession(Before)) return false;
        Focus=OldFocus;bEnemyDetailsOpen=EnemyDetails;LabelLayoutFrame=MAX_uint64;
    }
    UE_LOG(LogTemp,Display,TEXT("GUNYU_LABEL_LAYOUT: PASS (rendered=%d compact=%d mean-leader=%.1f max-leader=%.1f; measured content, stable layout, nonoverlap, public anchors, immutable view and correct selection)"),Shown,Short,Shown ? TotalLeaderDistance/Shown:0,Maximum);
    return true;
}
