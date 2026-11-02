#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "TacticalBattleResponse.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
using namespace gunyuui;
void ATacticalController::BuildTacticRelay(TSharedRef<SCanvas> Canvas)
{
    auto Read=[this] { return gunyuresponse::Read(Field->CurrentBeat(),Field->VisualUnits(),Field->BeatProgress(),Field->ReadTacticCinema().NextActor); };
    auto Relay=SNew(STacticalChrome).Padding(FMargin(12,10)).Accent(Gold)
      .Visibility_Lambda([this,Read] { return bTacticCinema && Read().Weight>.001f ? EVisibility::HitTestInvisible:EVisibility::Collapsed; })
      [SNew(SVerticalBox)
       + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([Read] { return Read().Actor; },62,82)]
        + SHorizontalBox::Slot().FillWidth(1).Padding(12,0)
          [SNew(SVerticalBox)
           + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Speaker+TEXT(" / ")+Read().Caption; },13,Gold)]
           + SVerticalBox::Slot().AutoHeight().Padding(0,7)[Label([Read] { return Read().Line; },20)]]]
       + SVerticalBox::Slot().AutoHeight().Padding(74,5,0,0)[Label([Read] { return Read().Reply; },14,Gold)]];
    Relay->ForceVolatile(true);CinematicRelay=Relay;Relay->SetRenderOpacity(0);
    Canvas->AddSlot().Position(FVector2D(984,168)).Size(FVector2D(588,136))[Relay];
}
