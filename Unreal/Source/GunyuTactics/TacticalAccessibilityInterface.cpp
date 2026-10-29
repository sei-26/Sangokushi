#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
using namespace gunyuui;
TSharedRef<SWidget> ATacticalController::BuildDifficultyChoice()
{
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,6,0)
          [Button([this] { return Field->Presentation().Journey.bStoryDifficulty ? TEXT("● 物語重視"):TEXT("○ 物語重視"); },
            [this] { Field->Issue(ETacticalCommand::DifficultyStory); },[this] { return Field->CanIssue(ETacticalCommand::DifficultyStory); })]
        + SHorizontalBox::Slot().AutoWidth()
          [Button([this] { return Field->Presentation().Journey.bStoryDifficulty ? TEXT("○ 標準"):TEXT("● 標準"); },
            [this] { Field->Issue(ETacticalCommand::DifficultyStandard); },[this] { return Field->CanIssue(ETacticalCommand::DifficultyStandard); })];
}
TSharedRef<SWidget> ATacticalController::BuildPlanningExample()
{
    auto Panel=SNew(SVerticalBox);
    Panel->AddSlot().AutoHeight()[Label([this] { return Field->Presentation().Journey.ExampleReason.IsEmpty() ? TEXT("作戦の見本 ／ 出陣前のたたき台"):TEXT("前戦を生かす提案 ／ 未採用"); },15,Gold)];
    Panel->AddSlot().AutoHeight().Padding(0,6)
      [SNew(SHorizontalBox).Visibility_Lambda([this] { return Field->Presentation().Journey.ExampleReason.IsEmpty() ? EVisibility::Collapsed:EVisibility::Visible; })
       + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([this] { return Field->Presentation().Journey.ExampleAdviser; },34,40)]
       + SHorizontalBox::Slot().FillWidth(1).Padding(8,0)[Label([this] { return Field->Presentation().Journey.ExampleReason; },14,Gold)]];
    Panel->AddSlot().AutoHeight().Padding(0,6)[Label([this] { return Field->Presentation().Journey.ExampleName; },18,Gold)];
    Panel->AddSlot().AutoHeight().Padding(0,4)[Label([this] { return Field->Presentation().Journey.ExamplePurpose; },15)];
    const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
    for(int I=0;I<3;++I)
        Panel->AddSlot().AutoHeight().Padding(0,4)[Label([this,I,Name=FString(Names[I])]
        {
            const auto& Orders=Field->Presentation().Journey.ExampleOrders;
            return Name+TEXT("：")+(Orders.IsValidIndex(I) ? Orders[I]:FString());
        },14,Gold)];
    Panel->AddSlot().AutoHeight().Padding(0,8,0,3)[Label([] { return TEXT("戦場で、自分が指揮すること"); },17,Gold)];
    Panel->AddSlot().AutoHeight()[Label([this] { return Field->Presentation().Journey.ExampleManual; },15)];
    Panel->AddSlot().AutoHeight().Padding(0,8)[Label([this] { return Field->Presentation().Journey.ExampleCaution; },13)];
    return Panel;
}
