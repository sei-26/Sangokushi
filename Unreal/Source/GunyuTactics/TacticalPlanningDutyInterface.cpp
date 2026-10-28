#include "TacticalTrial.h"
#include "TacticalPlanningGuide.h"
#include "TacticalWidgets.h"
#include "Widgets/SBoxPanel.h"
using namespace gunyuui;
TSharedRef<SWidget> ATacticalController::BuildPlanningDuties()
{
    auto Panel=SNew(SVerticalBox);
    Panel->AddSlot().AutoHeight()[Label([] { return TEXT("① 誰に任せる？"); },21,Gold)];
    const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
    auto People=SNew(SHorizontalBox);
    for(int I=0;I<3;++I)
        People->AddSlot().FillWidth(1).Padding(0,0,6,0)
          [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(7)
           .OnClicked_Lambda([this,I] { SelectPlanningOfficer(I);return FReply::Handled(); })
           [SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([I] { return I; },28,34)]
            + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center).Padding(6,0)
              [Label([this,I,Name=FString(Names[I])] { return (PlanningOfficer==I ? TEXT("● "):TEXT("○ "))+Name; },16,Gold)]]];
    Panel->AddSlot().AutoHeight().Padding(0,4,0,8)[People];
    Panel->AddSlot().AutoHeight()[Label([] { return TEXT("② どこへ向かう？"); },21,Gold)];
    Panel->AddSlot().AutoHeight().Padding(0,6)[Label([this]
    { return bPlanningLocationChosen ? planningguide::Place(Field->Presentation().Journey.Map,PlanningTile):TEXT("左の地図で、目的地のマスをクリック。"); },16)];
    Panel->AddSlot().AutoHeight().Padding(0,2,0,5)[Label([this]
    { return bPlanningLocationChosen ? TEXT("水色は仮経路。敵・味方との接触で変わります。"):TEXT("青い出陣位置から、行き先を考えよう。"); },13,Gold)];
    Panel->AddSlot().AutoHeight().Padding(0,0,0,5)[Label([] { return TEXT("③ 到着したら、何をする？"); },21,Gold)];
    for(int Kind:{1,2})
    {
        Panel->AddSlot().AutoHeight().Padding(0,3)
          [Button([Kind] { return Kind==1 ? TEXT("この場所を守らせる"):TEXT("ここで伏兵を準備させる"); },[this,Kind] { SetPlanningDuty(Kind); },
            [this,Kind] { return PlanningDutyReason(Kind).IsEmpty(); })];
        Panel->AddSlot().AutoHeight().Padding(0,2,0,5)[Label([this,Kind]
        {
            const auto Problem=PlanningDutyReason(Kind);
            if(!Problem.IsEmpty()) return Problem;
            if(Kind==1 && Field->Presentation().Journey.Map.ConvoyRoute.Contains(PlanningTile)) return FString(TEXT("護送路上です。固守すると護送車を止めます。"));
            return FString(Kind==1 ? TEXT("移動後に固守。攻撃の行動権を使います。"):TEXT("移動 → 伏兵準備。敵の接近や斥候で失敗することもある。"));
        },13)];
    }
    Panel->AddSlot().AutoHeight().Padding(0,3)
        [Button([] { return TEXT("選択中の武将の担当を解除"); },[this] { SetPlanningDuty(0); },[this]
        { const auto& D=Field->Presentation().Journey.Map.Duties;return D.IsValidIndex(PlanningOfficer) && D[PlanningOfficer].Kind!=0; },true)];
    Panel->AddSlot().AutoHeight().Padding(0,5)[Label([this]
    {
        return FString(Field->bRealtimeCommand ? TEXT("Dで出陣 → Spaceで開始。途中の指示はXで解除できます。"):
            TEXT("手動指揮：Dで出陣後、自分で移動と準備を実行。"))+
            TEXT("\n未指定の武将も、戦場で指揮できます。");
    },13,Gold)];
    return Panel;
}
