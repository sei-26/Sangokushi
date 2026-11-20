#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
using namespace gunyuui;

void ATacticalController::BuildGrandMomentInterface(TSharedRef<SCanvas> Canvas)
{
    auto Read=[this] { return Field->ReadGrandMoment(); };
    auto Panel=SNew(STacticalChrome).Padding(12).Accent(FLinearColor(.94f,.65f,.19f))
      .Visibility_Lambda([this,Read]
      {
          const auto R=Read();return bTacticCinema && R.Weight>.001f && !bTacticPickerOpen && !bEpisodeOpen && !bSaveMenuOpen && !bBattleLogOpen && !bMarchPlanOpen &&
              (!Field->Presentation().Journey.bVisible || R.bVictory) ? EVisibility::SelfHitTestInvisible:EVisibility::Collapsed;
      })
      [SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()[NovelPortraitWidget([Read] { return Read().Speaker; },92,120)]
        + SHorizontalBox::Slot().FillWidth(1).Padding(14,0)[SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1)[Label([Read] { return Read().Title; },26,Gold)]
            + SHorizontalBox::Slot().AutoWidth().Padding(8,0)[Button([] { return TEXT("采配へ"); },[this] { Field->DismissGrandMoment(); })]]
          + SVerticalBox::Slot().AutoHeight().Padding(0,5)[Label([Read] { return Read().Line; },19)]
          + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Label([Read] { return Read().Reply; },14,Gold)]]];
    GrandMomentPanel=Panel;Panel->SetRenderOpacity(0);InputPanels.Add(Panel);
    Canvas->AddSlot().Position(FVector2D(30,116)).Size(FVector2D(990,154))[Panel];
}

void ATacticalController::ApplyGrandMomentCamera(FVector& Center,float& Radius,float& Tilt) const
{
    if(!bTacticCinema || Field->IsPlaying() || !Field->Presentation().bBattle || bSaveMenuOpen || bEpisodeOpen || bBattleLogOpen || bMarchPlanOpen || bTacticPickerOpen) return;
    const auto R=Field->ReadGrandMoment();const auto& Units=Field->VisualUnits();
    if(R.Weight<=0 || !Units.IsValidIndex(R.FocusArmy) || Units[R.FocusArmy].Id<0 || Units[R.FocusArmy].Troops<=0) return;
    FVector Target=Units[R.FocusArmy].Position;
    if(R.Tile>=0) Target=FMath::Lerp(Target,Field->SurfaceLocation(R.Tile),.35f);
    const float W=R.Weight*.65f;
    Center=FMath::Lerp(Center,Target,W);
    Radius=FMath::Lerp(Radius,FMath::Max(Radius,3900.f),W);
    Tilt=FMath::Lerp(Tilt,43.f,W);
}
