#include "TacticalTrial.h"
#include "TacticalSaveStorage.h"

void ATacticalController::RefreshSaveSlots()
{
    SaveSlotLabels.Reset();SaveSlotAvailable.Reset();SaveSlotExists.Reset();
    for(int Slot=0;Slot<4;++Slot)
    {
        bool Available=false,Exists=false;SaveSlotLabels.Add(gunyusave::Describe(Slot,Available,Exists));
        SaveSlotAvailable.Add(Available);SaveSlotExists.Add(Exists);
    }
}
void ATacticalController::ToggleSaveMenu()
{
    if(!Field || (!bSaveMenuOpen && Field->IsPlaying())) return;
    bSaveMenuOpen=!bSaveMenuOpen;PendingSaveSlot=-1;
    if(bSaveMenuOpen)
    {
        bInterfaceVisible=true;bBattleLogOpen=bMarchPlanOpen=bTacticPickerOpen=false;
        if(Field->bRealtimeCommand && !Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
        SaveStatus=TEXT("軍議・戦後に自動保存。戦闘中の命令も手動で記録できます。読み込み後は一時停止から再開します。");
        RefreshSaveSlots();
    }
    IgnoreMapInputThroughFrame=Frames+1;
}
void ATacticalController::RequestSaveAction(int Slot,bool bLoad)
{
    if(!bSaveMenuOpen || Slot<0 || Slot>3 || (!bLoad && Slot==3) || Field->IsPlaying()) return;
    if(bLoad && !SaveSlotAvailable[Slot]) return;
    PendingSaveSlot=Slot;bPendingLoad=bLoad;
    if(!bLoad && !SaveSlotExists[Slot]) { ConfirmSaveAction();return; }
    const FString Name=Slot==3 ? TEXT("自動記録") : FString::Printf(TEXT("手動記録 %d"),Slot+1);
    SaveStatus=bLoad ? Name+TEXT(" を読み込みますか？ 保存していない現在の進行は失われます。") : Name+TEXT(" を現在の進行で上書きしますか？");
}
void ATacticalController::ConfirmSaveAction()
{
    const int Slot=PendingSaveSlot;PendingSaveSlot=-1;
    if(Slot<0 || Slot>3 || Field->IsPlaying()) return;
    bool Success=false,bBackup=false;std::vector<std::uint8_t> Bytes;
    if(bPendingLoad)
    {
        Success=gunyusave::Read(Slot,Bytes,bBackup) && Field->ImportSession(Bytes);
        SaveStatus=Success ? (bBackup ? TEXT("前回の有効な記録から復元しました。閉じて作戦を確認してください。") : TEXT("記録から再開しました。閉じて作戦を確認してください。")) : TEXT("読み込めませんでした。現在の進行は変更していません。");
    }
    else
    {
        Success=Field->ExportSession(Bytes) && gunyusave::Write(Slot,Bytes);
        SaveStatus=Success ? TEXT("現在の進行と作戦を記録しました。") : TEXT("保存できませんでした。以前の記録を確認し、もう一度お試しください。");
    }
    RefreshSaveSlots();
}
