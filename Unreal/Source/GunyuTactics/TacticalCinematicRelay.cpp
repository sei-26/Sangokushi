#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
using namespace gunyuui;
void ATacticalController::BuildTacticRelay(TSharedRef<SCanvas> Canvas)
{
    auto Read=[this] { return Field->ReadTacticCinema(); };
    auto Relay=SNew(STacticalChrome).Padding(FMargin(10,6)).Accent(Gold)
      .Visibility_Lambda([this,Read] { return bTacticCinema && Read().Relay>.001f ? EVisibility::HitTestInvisible:EVisibility::Collapsed; })
      [SNew(SHorizontalBox)
       + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([Read] { return Read().NextActor; },36,42)]
       + SHorizontalBox::Slot().FillWidth(1).Padding(12,0).VAlign(VAlign_Center)
         [SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[Label([] { return TEXT("連鎖  →  次の一手へ"); },12,Gold)]
          + SVerticalBox::Slot().AutoHeight().Padding(0,3)[Label([Read] { return Read().NextOfficer; },19)]]];
    CinematicRelay=Relay;Relay->SetRenderOpacity(0);
    Canvas->AddSlot().Position(FVector2D(860,168)).Size(FVector2D(368,56))[Relay];
}
