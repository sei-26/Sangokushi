#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
using namespace frontline;

bool ATacticalField::ExportSession(std::vector<std::uint8_t>& Bytes) const
{
    if(IsPlaying()) return false;
    auto Save=std::make_unique<JourneySaveData>();
    Save->journey=State->Journey;
    if(State->BeforeBattle) Save->beforeBattle=*State->BeforeBattle;
    Save->realtime=State->Realtime;Save->signal=State->ChainSignal;Save->watch=State->BattleWatch;
    Save->orders=State->Orders;Save->tactics=State->SelectedTactics;
    Save->selected=State->Selected;Save->target=State->Target;
    Save->dialogue=State->DialogueIndex;Save->preparation=State->PreparationChoice;Save->interlocutor=State->Interlocutor;
    Save->realtimeMode=bRealtimeCommand;Save->pauseOnAlert=bPauseOnBattleAlert;Save->threatOverlay=State->bThreatOverlay;
    return EncodeJourneySave(*Save,Bytes);
}
bool ATacticalField::ImportSession(const std::vector<std::uint8_t>& Bytes)
{
    if(IsPlaying()) return false;
    auto Save=std::make_unique<JourneySaveData>();
    if(!DecodeJourneySave(Bytes,*Save)) return false;
    auto Restored=TUniquePtr<FJourneyState,FJourneyDeleter>(new FJourneyState);
    Restored->Journey=std::move(Save->journey);Restored->Realtime=std::move(Save->realtime);
    Restored->ChainSignal=std::move(Save->signal);Restored->BattleWatch=std::move(Save->watch);
    Restored->Orders=std::move(Save->orders);Restored->SelectedTactics=Save->tactics;
    Restored->Selected=Save->selected;Restored->Target=Save->target;
    Restored->DialogueIndex=Save->dialogue;Restored->PreparationChoice=Save->preparation;Restored->Interlocutor=Save->interlocutor;
    if(Save->beforeBattle) Restored->BeforeBattle=std::make_unique<StrategistJourney>(std::move(*Save->beforeBattle));
    bRealtimeCommand=Save->realtimeMode;bPauseOnBattleAlert=Save->pauseOnAlert;
    Restored->bThreatOverlay=Save->threatOverlay;
    Restored->bRealtimeInBattle=bRealtimeCommand && Restored->Journey.stage==JourneyStage::Battle;
    Restored->CheckpointStage=static_cast<int>(Restored->Journey.stage);Restored->CheckpointMission=Restored->Journey.mission;
    Restored->bNoPlayback=State->bNoPlayback;
    Restored->Report=TEXT("記録から再開しました。戦況と命令を確認し、Spaceで作戦を再開できます。");
    State=MoveTemp(Restored);Refresh();return true;
}
