#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
using namespace gunyuui;

void ATacticalController::BuildSaveInterface(TSharedRef<SCanvas> Canvas)
{
    auto Open=Button([] { return TEXT("記録 F6"); },[this] { ToggleSaveMenu(); },[this] { return Field && !Field->IsPlaying(); },true);
    InputPanels.Add(Open);Canvas->AddSlot().Position(FVector2D(1160,36)).Size(FVector2D(120,42))[Open];
    auto Content=SNew(SVerticalBox);
    Content->AddSlot().AutoHeight()[SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1)[Label([] { return TEXT("軍師の記録 / 保存・再開"); },25,Gold)]
        + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("閉じる / F6・Esc"); },[this] { ToggleSaveMenu(); },[] { return true; },true)]];
    Content->AddSlot().AutoHeight().Padding(0,12)[Label([this] { return SaveStatus; },16,Gold)];
    for(int Slot=0;Slot<4;++Slot)
        Content->AddSlot().AutoHeight().Padding(0,5)
        [SNew(SBorder).Padding(12).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor_Lambda([this,Slot] { return PendingSaveSlot==Slot ? FLinearColor(.13f,.095f,.035f) : FLinearColor(.018f,.029f,.035f); })
         [SNew(SHorizontalBox)
          + SHorizontalBox::Slot().FillWidth(1)[SNew(SVerticalBox)
             + SVerticalBox::Slot().AutoHeight()[Label([Slot] { return Slot==3 ? FString(TEXT("自動記録 / 軍議・戦後")) : FString::Printf(TEXT("手動記録 %d"),Slot+1); },18,Gold)]
             + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Label([this,Slot] { return SaveSlotLabels.IsValidIndex(Slot) ? SaveSlotLabels[Slot] : FString(); },14)]]
          + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8,0)
            [SNew(SBox).Visibility(Slot==3 ? EVisibility::Collapsed : EVisibility::Visible)
             [Button([] { return TEXT("保存"); },[this,Slot] { RequestSaveAction(Slot,false); },[this] { return !Field->IsPlaying() && PendingSaveSlot<0; })]]
          + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [Button([] { return TEXT("再開"); },[this,Slot] { RequestSaveAction(Slot,true); },[this,Slot] { return PendingSaveSlot<0 && SaveSlotAvailable.IsValidIndex(Slot) && SaveSlotAvailable[Slot]; })] ]];
    Content->AddSlot().AutoHeight().Padding(0,12)
        [SNew(SHorizontalBox).Visibility_Lambda([this] { return PendingSaveSlot>=0 ? EVisibility::Visible : EVisibility::Collapsed; })
         + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,12,0)[Button([this] { return bPendingLoad ? TEXT("選んだ記録から再開する") : TEXT("選んだ記録に上書き保存する"); },[this] { ConfirmSaveAction(); },[] { return true; })]
         + SHorizontalBox::Slot().FillWidth(1)[Button([] { return TEXT("戻る"); },[this] { PendingSaveSlot=-1;SaveStatus=TEXT("記録を選んでください。"); },[] { return true; })]];
    Content->AddSlot().AutoHeight().Padding(0,8)[Label([] { return TEXT("閲覧中と再開直後は一時停止。演出中は記録できません。旧作品のセーブとは別に保存します。"); },13)];
    auto Shade=SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0,0,0,.72f))
        .Visibility_Lambda([this] { return bSaveMenuOpen ? EVisibility::Visible : EVisibility::Collapsed; });
    auto Panel=SNew(STacticalChrome).Padding(24).Accent(FLinearColor(.56f,.42f,.18f))
        .Visibility_Lambda([this] { return bSaveMenuOpen ? EVisibility::Visible : EVisibility::Collapsed; })[Content];
    InputPanels.Add(Shade);InputPanels.Add(Panel);
    Canvas->AddSlot().Position(FVector2D::ZeroVector).Size(FVector2D(1600,900))[Shade];
    Canvas->AddSlot().Position(FVector2D(300,140)).Size(FVector2D(1000,640))[Panel];
}
