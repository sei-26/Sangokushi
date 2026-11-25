#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "TacticalMissionTarget.h"
#include "JourneySave.hpp"
#include <memory>
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
using namespace gunyuui;
void ATacticalController::BuildTacticTargetInterface(TSharedRef<SCanvas> Canvas)
{
    auto Panel=SNew(STacticalChrome).Padding(FMargin(8,4)).Accent(FLinearColor(.21f,.61f,.55f))
        .Visibility_Lambda([this] { return bDetailedBattle && Field->Presentation().bBattle && !Field->Presentation().TacticTargetSummary.IsEmpty() && !Field->IsPlaying() && !bTacticPickerOpen ? EVisibility::Visible:EVisibility::Collapsed; })
        [SNew(SHorizontalBox)
         + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
           [SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[Label([this] { const auto& H=Field->Presentation();return H.SelectedTacticName+TEXT(" / ")+H.TacticTargetSummary; },12,Gold)]
            + SVerticalBox::Slot().AutoHeight().Padding(0,2,0,0)[Label([this] { return Field->Presentation().bTacticTargetOverlay ? TEXT("水色＝単独可　金＝連鎖後可 ／ 敵札に理由"):TEXT("自隊と周囲2マスを支援 ／ 敵の指定は不要"); },11)]]
         + SHorizontalBox::Slot().AutoWidth().Padding(6,0,0,0).VAlign(VAlign_Center)
           [SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
              [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(FMargin(5,0))
               .ToolTipText(FText::FromString(TEXT("成立見込みの敵を順に選び、指揮を停止します。予約と発動は自分で行います。")))
               .IsEnabled_Lambda([this] { const auto& H=Field->Presentation();return H.bTacticTargetOverlay && H.ReadyTacticTargets>0 && !Field->IsPlaying(); })
               .OnClicked_Lambda([this] { FocusNextTacticTarget();return FReply::Handled(); })[Label([] { return TEXT("届く敵へ"); },11)]]
            + SVerticalBox::Slot().AutoHeight().Padding(0,3,0,0)
              [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(FMargin(5,0))
               .ToolTipText(FText::FromString(TEXT("選択隊の隣接1歩を仮配置し、戦法の成立と潜伏・守備の代償を比較します。番号を選ぶだけでは移動しません。予約済みの隊は予約欄の×で外すと比較できます。")))
               .IsEnabled_Lambda([this] { return Field->CanCompareTacticPositions(); })
               .OnClicked_Lambda([this] { OpenTacticPositionComparison();return FReply::Handled(); })[Label([] { return TEXT("配置を比較"); },11)]]]];
    InputPanels.Add(Panel);Canvas->AddSlot().Position(FVector2D(728,116)).Size(FVector2D(528,50))[Panel];
}
bool ATacticalController::FocusNextTacticTarget()
{
    if(!Field->SelectNextTacticTarget()) return false;
    for(const auto& Read:Field->Presentation().TacticTargets) if(Read.bSelected)
    { Focus=Field->SurfaceLocation(Read.Tile);LabelLayoutFrame=MAX_uint64;break; }
    IgnoreMapInputThroughFrame=Frames+1;return true;
}
bool ATacticalController::TacticTargetInterfaceSmokeTest()
{
    const auto SavedFocus=Focus;
    Field->PrepareTacticTargetPreview(2);
    bool Passed=UnitMapStatus(3).Contains(TEXT("単独可")) && UnitMapStatus(4).Contains(TEXT("単独可"));
    Passed=Passed && FocusNextTacticTarget();
    if(Passed)
    {
        const auto& H=Field->Presentation();const auto* Selected=H.TacticTargets.FindByPredicate([](const auto& Read) { return Read.bSelected; });
        Passed=Selected && Selected->Army==4 && Focus.Equals(Field->SurfaceLocation(Selected->Tile)) && H.Orders.IsEmpty();
    }
    Field->PrepareTacticTargetPreview(1);Passed=Passed && UnitMapStatus(3).Contains(TEXT("2手目可"));
    Field->Issue(ETacticalCommand::Move);
    Passed=Passed && !UnitMapStatus(3).Contains(TEXT("手目可")) && Field->Presentation().TacticTargetSummary.IsEmpty() && !FocusNextTacticTarget();
    Field->Issue(ETacticalCommand::Cancel);
    std::vector<std::uint8_t> Original,Before,After,Normalised;
    const bool Council=Field->bStartWithCouncil,Legacy=Field->bLegacyJourney,Grand=Field->bGrandHistoricalBattles;
    const int Mission=Field->FirstMission;
    Passed=Passed && Field->ExportSession(Original) && Field->PrepareGrandBattlePreview(frontline::YangrenMission,3,2);
    if(Passed)
    {
        bool Reserved=false;
        for(int Actor=0;Actor<3 && !Reserved;++Actor)
        {
            Field->SelectOfficer(Actor);Field->SelectTactic(int(frontline::ChainTactic::Feint));
            const auto* Available=Field->Presentation().TacticTargets.FindByPredicate([](const FTacticTargetPresentation& T) { return T.bReady; });
            if(Available)
            { Field->SelectAt(Available->Tile);Field->Issue(ETacticalCommand::QueueTactic);Reserved=!Field->Presentation().Orders.IsEmpty(); }
        }
        Passed=Reserved;
        const auto& H=Field->Presentation();
        const auto* Other=H.TacticTargets.FindByPredicate([&](const FTacticTargetPresentation& T) { return T.Tile!=H.MissionFocusTile; });
        Passed=Passed && Other!=nullptr;
        if(Passed) Field->SelectAt(Other->Tile);
        const auto* MissionRead=gunyucommand::MissionEnemy(Field->Presentation());
        Passed=Passed && MissionRead;
        if(Passed)
        {
            const int Army=MissionRead->Army,Tile=MissionRead->Tile;
            if(Field->RealtimePaused()) Field->Issue(ETacticalCommand::RealtimePause);
            Passed=Field->ExportSession(Before) && gunyucommand::SelectMissionEnemy(*Field,Army,Tile) && Field->ExportSession(After);
            auto A=std::make_unique<frontline::JourneySaveData>(),B=std::make_unique<frontline::JourneySaveData>();
            Passed=Passed && frontline::DecodeJourneySave(Before,*A) && frontline::DecodeJourneySave(After,*B);
            if(Passed)
            {
                A->target=B->target;
                A->realtime.paused=B->realtime.paused;
                Passed=B->target==Army && B->realtime.paused && frontline::EncodeJourneySave(*A,Normalised) && Normalised==After;
            }
            FocusMissionObjective();Passed=Passed && Focus.Equals(Field->SurfaceLocation(Tile));
            auto Hidden=Field->Presentation();
            for(auto& T:Hidden.TacticTargets) if(T.Army==Army) T.bSelected=false;
            Passed=Passed && gunyucommand::MissionEnemy(Hidden);
            Hidden.TacticTargets.RemoveAll([Army](const FTacticTargetPresentation& T) { return T.Army==Army; });
            Passed=Passed && !gunyucommand::MissionEnemy(Hidden);
            const auto& Now=Field->Presentation();
            const auto* Alternative=Now.TacticTargets.FindByPredicate([Army](const FTacticTargetPresentation& T) { return T.Army!=Army; });
            if(Alternative) Field->SelectAt(Alternative->Tile);else Passed=false;
            Passed=Passed && Field->ExportSession(Before) && !gunyucommand::SelectMissionEnemy(*Field,Army,Tile+1) && Field->ExportSession(After) && Before==After;
            Field->Issue(ETacticalCommand::Move);
            Passed=Passed && !Field->Presentation().bTacticTargetOverlay && Field->ExportSession(Before) &&
                !gunyucommand::SelectMissionEnemy(*Field,Army,Tile) && Field->ExportSession(After) && Before==After;
        }
    }
    Field->bStartWithCouncil=Council;Field->bLegacyJourney=Legacy;Field->bGrandHistoricalBattles=Grand;Field->FirstMission=Mission;
    if(!Original.empty()) Passed=Field->ImportSession(Original) && Passed;
    Focus=SavedFocus;
    if(Passed) { UE_LOG(LogTemp,Display,TEXT("GUNYU_TACTIC_TARGET_UI: PASS (public badges, actual cavalry focus, explicit target selection, unchanged reservations and budgets, stale release and destination-input isolation)")); }
    else { UE_LOG(LogTemp,Error,TEXT("GUNYU_TACTIC_TARGET_UI: FAIL (badges, focus or input isolation)")); }
    return Passed;
}
