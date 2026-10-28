#include "TacticalTrial.h"
#include "CampaignTypes.hpp"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
using namespace gunyuui;
TSharedRef<SWidget> ATacticalController::BuildPlanningDuties()
{
    auto Panel=SNew(SVerticalBox);
    Panel->AddSlot().AutoHeight()[Label([] { return TEXT("誰に、どこを任せるか"); },21,Gold)];
    const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
    auto People=SNew(SHorizontalBox);
    for(int I=0;I<3;++I)
        People->AddSlot().FillWidth(1).Padding(0,0,4,0)[Button([this,I,Name=FString(Names[I])] { return (PlanningOfficer==I ? TEXT("● "):TEXT("○ "))+Name; },[this,I] { PlanningOfficer=I; },[] { return true; },true)];
    Panel->AddSlot().AutoHeight().Padding(0,8)[People];
    Panel->AddSlot().AutoHeight()[Label([this] { return TEXT("指定先：")+PlanningTitle(); },17,Gold)];
    Panel->AddSlot().AutoHeight().Padding(0,4)[Label([this] { return FString::Printf(TEXT("地図を選ぶ → 役目を確定　(%d,%d)"),PlanningTile%frontline::Width,PlanningTile/frontline::Width); },13)];
    const TCHAR* Actions[]={TEXT("担当を解除"),TEXT("ここへ進み、固守"),TEXT("ここへ進み、伏兵準備")};
    for(int Kind:{1,2,0})
    {
        Panel->AddSlot().AutoHeight().Padding(0,4)[Button([Name=FString(Actions[Kind])] { return Name; },[this,Kind] { SetPlanningDuty(Kind); },
            [this,Kind] { return Field->PlanningDutyProblem(PlanningOfficer,PlanningTile,Kind).IsEmpty(); },true)];
    }
    Panel->AddSlot().AutoHeight().Padding(0,6)[Label([this]
    {
        const auto Reason=Field->PlanningDutyProblem(PlanningOfficer,PlanningTile,1);
        return Reason.IsEmpty() ? TEXT("伏兵：関羽＋森林で指定可能。\n固守は攻撃の行動権を使う。護送路を塞ぐ配置には注意。"):Reason;
    },14)];
    Panel->AddSlot().AutoHeight().Padding(0,10,0,4)[Label([] { return TEXT("確定した担当"); },17,Gold)];
    for(int I=0;I<3;++I)
        Panel->AddSlot().AutoHeight().Padding(0,4)[Label([this,I,Name=FString(Names[I])]
        {
            const auto& Duties=Field->Presentation().Journey.Map.Duties;
            return Name+TEXT("：")+(Duties.IsValidIndex(I) ? Duties[I].Status:FString());
        },14)];
    return Panel;
}
