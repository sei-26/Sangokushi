#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalSaveStorage.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void ATacticalField::CheckpointJourney(bool bForce)
{
    if(!GetWorld() || !GetWorld()->IsGameWorld() || IsPlaying()) return;
    const int Stage=static_cast<int>(State->Journey.stage),Mission=State->Journey.mission;
    if(!bForce && State->CheckpointStage==Stage && State->CheckpointMission==Mission) return;
    const bool bTransition=bForce || State->CheckpointStage>=0;
    State->CheckpointStage=Stage;State->CheckpointMission=Mission;
    if(!bTransition) return;
    const bool QA=FParse::Param(FCommandLine::Get(),TEXT("GunyuSmoke")) || FParse::Param(FCommandLine::Get(),TEXT("GunyuCapture"));
    if(QA && !FParse::Param(FCommandLine::Get(),TEXT("GunyuCheckpointQA"))) return;
    if(State->Journey.stage!=frontline::JourneyStage::Preparation && State->Journey.stage!=frontline::JourneyStage::Debrief && State->Journey.stage!=frontline::JourneyStage::Complete) return;
    std::vector<std::uint8_t> Bytes;
    const bool Saved=ExportSession(Bytes) && gunyusave::Write(3,Bytes);
    if(!Saved) { State->Report+=TEXT("\n自動保存できませんでした。記録画面で手動保存してください。");State->HUD.Report=State->Report; }
    UE_LOG(LogTemp,Display,TEXT("GUNYU_CHECKPOINT: %s mission=%d stage=%d"),Saved ? TEXT("saved") : TEXT("failed"),Mission,Stage);
}
