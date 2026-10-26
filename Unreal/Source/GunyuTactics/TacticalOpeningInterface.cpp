#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
using namespace gunyuui;

TSharedRef<SWidget> ATacticalController::OpeningRouteChoices()
{
    auto Choice=[this](ETacticalCommand Command,TFunction<FString()> Name)
    { return Button(Name,[this,Command] { Field->Issue(Command); },[this,Command] { return Field->CanIssue(Command); }); };
    return SNew(SVerticalBox).Visibility_Lambda([this] { return Field->Presentation().Journey.bChooseRoute ? EVisibility::Visible : EVisibility::Collapsed; })
        + SVerticalBox::Slot().AutoHeight().Padding(0,3)[Label([] { return TEXT("季安の車をどこへ通すか"); },17,Gold)]
        + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,8,0)[Choice(ETacticalCommand::RouteRoad,[this] { return Field->Presentation().Journey.bSouthernRoute ? TEXT("○ 街道 / ６区間") : TEXT("● 街道 / ６区間"); })]
            + SHorizontalBox::Slot().FillWidth(1)[Choice(ETacticalCommand::RouteSouthernWood,[this] { return Field->Presentation().Journey.bSouthernRoute ? TEXT("● 南の林道 / ７区間・案内料120") : TEXT("○ 南の林道 / ７区間・案内料120"); })]]
        + SVerticalBox::Slot().AutoHeight().Padding(0,6,0,12)[Label([this] { return Field->Presentation().Journey.ConvoyRouteInfo; },15)];
}
