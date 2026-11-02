#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "TacticalMissionResponse.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
using namespace gunyuui;
void ATacticalController::BuildMissionMomentInterface(TSharedRef<SCanvas> Canvas)
{
    auto Read=[this] { return Field->ReadMissionResponse(); };
    auto Panel=SNew(STacticalChrome).Padding(FMargin(12,10)).Opacity(1.f)
      .Visibility_Lambda([this,Read] { return bTacticCinema && !bEpisodeOpen && !bSaveMenuOpen && !bBattleLogOpen && !bMarchPlanOpen && Read().Weight>.001f ? EVisibility::Visible:EVisibility::Collapsed; })
      [SNew(SVerticalBox)
       + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([Read] { return Read().Actor; },68,88)]
        + SHorizontalBox::Slot().FillWidth(1).Padding(12,0)[SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Speaker+TEXT(" / ")+Read().Caption; },14,Gold)]
          + SVerticalBox::Slot().AutoHeight().Padding(0,7)[Label([Read] { return Read().Line; },20)]]]
       + SVerticalBox::Slot().AutoHeight().Padding(80,5,0,0)[Label([Read] { return Read().Reply; },14,Gold)]];
    MissionMomentPanel=Panel;Panel->SetRenderOpacity(0);
    InputPanels.Add(Panel);
    Canvas->AddSlot().Position(FVector2D(32,120)).Size(FVector2D(700,140))[Panel];
}
