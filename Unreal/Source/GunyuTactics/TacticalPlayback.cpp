#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyEpisode.hpp"
#include "JourneyCouncilStage.hpp"
#include "Components/AudioComponent.h"

using namespace frontline;

bool ATacticalField::IsPlaying() const { return State->Beats.IsValidIndex(State->BeatIndex); }
const TArray<FVisualUnit>& ATacticalField::VisualUnits() const { return State->Units; }
const FBattleBeat* ATacticalField::CurrentBeat() const { return IsPlaying() ? &State->Beats[State->BeatIndex] : nullptr; }
float ATacticalField::BeatProgress() const { return IsPlaying() ? FMath::Clamp(State->BeatTime / CurrentBeat()->Duration, 0.f, 1.f) : 0; }
FString ATacticalField::CueTitle() const
{
    const auto* Beat = CurrentBeat();
    return Beat ? FString::Printf(TEXT("%d / %d  %s"), State->BeatIndex + 1, State->Beats.Num(), *Beat->Title) : FString();
}
FString ATacticalField::CueReason() const { return CurrentBeat() ? State->HUD.Report : FString(); }

TArray<FVisualUnit> ATacticalField::ObserveUnits() const
{
    TArray<FVisualUnit> Result;
    const auto& J = State->Journey;
    Result.SetNum(FMath::Max(6,int(J.world.armies.size())));
    for (int Id = 0; Id < static_cast<int>(J.world.armies.size()); ++Id)
    {
        const auto& Actual = J.world.armies[Id];
        const Army* Seen = &Actual;
        if (Actual.faction != 0)
        {
            const auto* Contact = J.intelligence[0].Find(Id);
            if (!Contact || !Contact->current) continue;
            Seen = &Contact->observed;
        }
        auto& Unit = Result[Id];
        Unit.Id = Id; Unit.Troops = Seen->troops;
        Unit.Name = JourneyText(J.world.generals[Seen->general].name);
        if(Unit.Name==JourneyText(EpisodeSpeakerName(ZhangLiang))) Unit.HistoricalSpeaker=ZhangLiang;
        if(J.HasSunJian() && Id==3) Unit.HistoricalSpeaker=LuBu;
        if(J.HasYangrenStrategy() && Id==8) Unit.HistoricalSpeaker=LuBu;
        if(J.HasSunJian() && Id==StrategistJourney::AlliedArmy) Unit.HistoricalSpeaker=SunJian;
        if(J.HasGrandBattle() && !J.HasSunJian() && Id==StrategistJourney::AlliedArmy) Unit.HistoricalSpeaker=J.HasCoalitionBattle() ? CaoCao:HuangfuSong;
        Unit.bTransport=Seen->arm==Arm::Transport;
        Unit.bCavalry=Seen->arm==Arm::Cavalry;
        Unit.Position = SurfaceLocation(Seen->tile);
        Unit.bEnemy = Seen->faction != 0;
        Unit.bConcealed = Id < 3 && J.chain.concealed[Id];
        Unit.Morale = Seen->morale;Unit.bGuarded=Seen->stance==battle::Stance::Guard;
        Unit.bDisordered = J.chain.disordered[Id]; Unit.bPressured = J.chain.pressured[Id]; Unit.bInspired = J.chain.inspired[Id];
    }
    return Result;
}

void ATacticalField::QueueAction(EBattleCue Kind, int Actor, int Target, const TArray<FVisualUnit>& Before,FMissionProgress MissionBefore)
{
    if(Kind==EBattleCue::Strike || Kind==EBattleCue::Withdraw) State->RetryDefenseAid={};
    FBattleBeat Beat;
    const auto GrandBefore=MissionBefore.Grand;
    Beat.Before = Before; Beat.After = ObserveUnits();
    if(Kind==EBattleCue::Enemy || Kind==EBattleCue::Guard || Kind==EBattleCue::Conceal)
    { Beat.MissionBefore=MoveTemp(MissionBefore);Beat.MissionAfter=CaptureMissionProgress(); }
    Beat.Actor = Actor; Beat.Target = Target; Beat.Kind = Kind;
    if(State->Journey.IsPortAssault() && State->Journey.stage==JourneyStage::Debrief)
        Beat.PortCaptor=State->Journey.battleReview.port.capturedBy;
    if(Kind==EBattleCue::March) Beat.MissionBefore=Beat.MissionAfter=CaptureMissionProgress();
    Beat.MissionBefore.Grand=GrandBefore;Beat.MissionAfter.Grand=CaptureMissionProgress().Grand;
    if(Kind==EBattleCue::March || Kind==EBattleCue::Enemy) AttachMissionCredit(Beat);
    if(State->bNoPlayback) return;
    const TCHAR* Names[] = {TEXT("進軍"), TEXT("通常攻撃"), TEXT("誘導"), TEXT("伏兵"), TEXT("突破"), TEXT("固守"), TEXT("伏兵準備"), TEXT("敵の指揮 / 交戦と補給"), TEXT("連鎖停止"), TEXT("全軍撤収 / 生存兵と残りの糧を回収"), TEXT("牽制"), TEXT("攪乱"), TEXT("全軍へ補給"), TEXT("青龍"), TEXT("咆哮")};
    Beat.Title = (Before.IsValidIndex(Actor) ? Before[Actor].Name + TEXT("  ―  ") : FString()) + Names[static_cast<int>(Kind)];
    if(Kind==EBattleCue::Rally) Beat.Title=TEXT("主人公の采配  ―  全軍へ補給 / 士気＋20");
    Beat.Reason = State->Report;
    Beat.Duration = Kind == EBattleCue::Enemy ? 3.4f : Kind == EBattleCue::March ? 1.8f : 2.4f;
    const auto& A=Beat.MissionBefore;
    const auto& Z=Beat.MissionAfter;
    if(Kind==EBattleCue::Enemy && A.bBattle && Z.bBattle &&
        A.Convoy==Z.Convoy && A.Held==Z.Held && A.CompletedDuties==Z.CompletedDuties &&
        A.ConfirmedReliefs==Z.ConfirmedReliefs && State->Journey.world.tacticEvents.empty() && gunyugrand::Compare(A.Grand,Z.Grand).Kind==EGrandMoment::None)
    {
        bool hasImpact=false;
        for(int Id=0;Id<Beat.Before.Num();++Id)
        {
            const auto& BeforeUnit=Beat.Before[Id];
            const auto& AfterUnit=Beat.After[Id];
            if(BeforeUnit.Id!=AfterUnit.Id || BeforeUnit.Troops!=AfterUnit.Troops ||
                (BeforeUnit.Troops>0 && (BeforeUnit.bConcealed!=AfterUnit.bConcealed || BeforeUnit.bDisordered!=AfterUnit.bDisordered)))
            { hasImpact=true;break; }
        }
        if(!hasImpact) Beat.Duration=1.f;
    }
    State->Beats = {MoveTemp(Beat)};
    BeginPlayback();
}

void ATacticalField::QueueChain(const TArray<FVisualUnit>& Before,FMissionProgress MissionBefore)
{
    State->RetryDefenseAid={};
    if (State->bNoPlayback) return;
    State->Beats.Reset();
    auto Display = Before;

    const auto& Events = State->Journey.lastChain.events;
    for (int Index = 0; Index < static_cast<int>(Events.size()); ++Index)
    {
        const auto& Event = Events[Index];
        FBattleBeat Beat;
        Beat.Before = Display;Beat.ChainStep=Index+1;
        Beat.Combo=Event.combo;Beat.ComboPercent=Event.comboPercent;
        Beat.Actor = Event.army; Beat.Target = Event.enemy;
        switch (Event.tactic)
        {
        case ChainTactic::Lure: Beat.Kind = EBattleCue::Lure; break;
        case ChainTactic::Ambush: Beat.Kind = EBattleCue::Ambush; break;
        case ChainTactic::Breakthrough: Beat.Kind = EBattleCue::Breakthrough; break;
        case ChainTactic::Feint: Beat.Kind = EBattleCue::Feint; break;
        case ChainTactic::Disrupt: Beat.Kind = EBattleCue::Disrupt; break;
        case ChainTactic::Oath: Beat.Kind = EBattleCue::Rally; break;
        case ChainTactic::AzureDragon: Beat.Kind = EBattleCue::AzureDragon; break;
        case ChainTactic::SerpentRoar: Beat.Kind = EBattleCue::SerpentRoar; break;
        }
        Beat.bSignature = TacticChain::Definition(Event.tactic).unique && Event.success;
        if (!Event.success) Beat.Kind = EBattleCue::Stop;
        Beat.Title = Display.IsValidIndex(Event.army) ? Display[Event.army].Name + TEXT("  ―  ") : FString();
        if (Beat.bSignature) Beat.Title += TEXT("固有戦法 / ");
        Beat.Title += JourneyText(TacticChain::Name(Event.tactic)) + (Event.success ? TEXT(" 発動") : TEXT(" 停止"));
        Beat.Reason = JourneyText(Event.reason);
        Beat.CouncilVoice=JourneyText(CouncilBattleVoice(State->Journey,Event,Index));
        Beat.Duration = Event.success ? (Beat.bSignature ? 3.8f : 3.0f) : 2.5f;
        // Visual snapshots come from resolved events, never from preview and never
        // by executing commands again. The authoritative battle is resolved once.
        if (Event.success) for (const auto& Change : Event.changes)
        {
            if (!Display.IsValidIndex(Change.army) || Display[Change.army].Id < 0) continue;
            auto& Unit = Display[Change.army];
            Unit.Position = SurfaceLocation(Change.tile); Unit.Troops = Change.troops; Unit.Morale = Change.morale;
            if(Change.army==Event.enemy && (Event.tactic==ChainTactic::Disrupt || Event.tactic==ChainTactic::AzureDragon || Event.tactic==ChainTactic::SerpentRoar)) Unit.bGuarded=false;
            Unit.bConcealed = Change.concealed; Unit.bDisordered = Change.disordered;
            Unit.bPressured = Change.pressured; Unit.bInspired = Change.inspired;
        }
        Beat.After = Display;
        Beat.MissionBefore=Beat.MissionAfter=CaptureMissionProgress();AttachMissionCredit(Beat);
        State->Beats.Add(MoveTemp(Beat));
    }
    if(!State->Beats.IsEmpty() && State->Journey.IsPortAssault() && State->Journey.stage==JourneyStage::Debrief)
        State->Beats.Last().PortCaptor=State->Journey.battleReview.port.capturedBy;
    if(!State->Beats.IsEmpty()) State->Beats.Last().MissionBefore.Grand=MissionBefore.Grand;
    BeginPlayback();
}

void ATacticalField::BeginPlayback()
{
    State->MissionResponse={};State->MissionResponseRemaining=0;
    State->GrandMoment={};State->GrandMomentRemaining=0;
    State->bSupplyPlayback=false;
    State->BeatIndex = 0; State->BeatTime = 0; State->bImpactPlayed = false;
    State->Reachable.Reset(); State->Input = ETacticalInput::Select;
    if (!IsPlaying()) return;
    State->Units = CurrentBeat()->Before;
    State->HUD.bBusy = true;
    State->HUD.NextLabel = TEXT("演出を完了  /  Space");
    State->HUD.Report=ReadPlaybackReport();
    PlayCueSound(false);
}
void ATacticalField::FinishPlayback(bool bShowMissionResponse)
{
    if (CueAudio) CueAudio->Stop();
    if(bShowMissionResponse)
    {
        PublishMissionResponse();
        PublishMissionSignals();
        PublishGrandMoment();
    }
    else
    {
        State->MissionResponse={};State->MissionResponseRemaining=0;
        State->GrandMoment={};State->GrandMomentRemaining=0;
        State->SettledMissionSignals={};State->MissionSignalRemaining={};
    }
    if(!State->Beats.IsEmpty() && State->Beats.Last().Kind==EBattleCue::Enemy) State->RetryDefenseAid={};
    if(State->bSupplyPlayback && bRealtimeCommand) State->Realtime.paused=true;
    State->bSupplyPlayback=false;
    State->Beats.Reset(); State->BeatIndex = 0; State->BeatTime = 0;
    if (bRealtimeCommand && (State->bRealtimePauseAfterPlayback || (State->bWatchPauseAfterPlayback && bPauseOnBattleAlert)))
    { State->Realtime.paused=true;State->bRealtimePauseAfterPlayback=false; }
    State->bWatchPauseAfterPlayback=false;
    PublishBattleNotices();
    State->bRealtimeStepping=false;
    PublishDecisionMoment();
    Refresh();
}
void ATacticalField::AdvancePresentation(float Delta)
{
    State->MissionResponseRemaining=FMath::Max(0.f,State->MissionResponseRemaining-FMath::Max(0.f,Delta));
    State->GrandMomentRemaining=FMath::Max(0.f,State->GrandMomentRemaining-FMath::Max(0.f,Delta));
    for(float& Remaining:State->MissionSignalRemaining)
        Remaining=FMath::Max(0.f,Remaining-FMath::Max(0.f,Delta));
    float Elapsed=FMath::Max(0.f,Delta)*FMath::Clamp(static_cast<float>(State->Realtime.speed),.5f,5.f);
    if (bRealtimeCommand && !State->bSupplyPlayback)
        Elapsed=State->Realtime.paused && !State->bRealtimeStepping ? 0.f : Elapsed;
    State->VisualTime += Elapsed;
    if (IsPlaying())
    {
        State->BeatTime += Elapsed * FMath::Clamp(PlaybackSpeed, .5f, 2.f);
        // Retain excess time, so frame rate never changes the end state or pace.
        while (IsPlaying() && State->BeatTime >= CurrentBeat()->Duration)
        {
            State->BeatTime -= CurrentBeat()->Duration;
            ++State->BeatIndex; State->bImpactPlayed = false;
            if (IsPlaying()) PlayCueSound(false);
        }
        if (!IsPlaying()) { FinishPlayback(); return; }
        const auto& Beat = *CurrentBeat();
        const float Progress = BeatProgress();
        if(Beat.PortCaptor>=0 && Progress>=.84f && State->HUD.bPortOpening)
        {
            State->HUD.bPortOpening=false;
            State->HUD.MissionStep=TEXT("河港占領 ｜ 指揮所に旗が立った");
            State->HUD.MissionAdvice=TEXT("指揮所を確保した");
            State->HUD.BattleChoiceTarget.Reset();
            State->HUD.BattleChoiceReason=TEXT("旗が立った。演出の後、戦果を確認しよう。");
            State->HUD.BattleChoices.Reset();
        }
        if (!State->bImpactPlayed && Progress >= .48f)
        { PlayCueSound(true); State->bImpactPlayed = true; }

        State->Units = Beat.Before;
        for (int Id = 0; Id < State->Units.Num(); ++Id)
        {
            auto& Unit = State->Units[Id];
            if (Unit.Id < 0 || !Beat.After.IsValidIndex(Id) || Beat.After[Id].Id < 0) continue;
            const auto& End = Beat.After[Id];
            float Start = .18f, Finish = .78f;
            if (Beat.Kind == EBattleCue::Lure) { Start = Id == Beat.Actor ? .12f : .38f; Finish = Id == Beat.Actor ? .57f : .86f; }
            if (Beat.Kind == EBattleCue::Breakthrough) { Start = Id == Beat.Actor ? .26f : .46f; Finish = .82f; }
            const float Travel = FMath::SmoothStep(0.f, 1.f, FMath::Clamp((Progress - Start) / (Finish - Start), 0.f, 1.f));
            Unit.Position = FMath::Lerp(Unit.Position, End.Position, Travel);
            Unit.Motion = FVector::DistSquared(Beat.Before[Id].Position, End.Position) > 1 && Travel > 0 && Travel < 1 ? 1.f : 0.f;
            if ((Beat.Kind == EBattleCue::Strike || Beat.Kind == EBattleCue::Ambush || Beat.Kind == EBattleCue::Feint || Beat.Kind == EBattleCue::Disrupt || Beat.Kind == EBattleCue::AzureDragon) && Id == Beat.Actor && Beat.Before.IsValidIndex(Beat.Target))
                Unit.Position += (Beat.Before[Beat.Target].Position - Unit.Position).GetSafeNormal() * FMath::Sin(FMath::Clamp((Progress - .25f) / .4f, 0.f, 1.f) * PI) * 65;
            if (Progress >= .48f)
            {
                Unit.Loss = FMath::Max(0, Unit.Troops - End.Troops); Unit.Troops = End.Troops; Unit.bConcealed = End.bConcealed;
                Unit.Morale = End.Morale;Unit.bGuarded=End.bGuarded; Unit.bDisordered = End.bDisordered; Unit.bPressured = End.bPressured; Unit.bInspired = End.bInspired;
            }
            Unit.Position.Z=SurfaceHeight(Unit.Position);
        }
        for (int Id = 0; Id < State->HUD.Officers.Num(); ++Id)
            State->HUD.Officers[Id].Status = FString::Printf(TEXT("兵 %d 士気 %d\n%s"), State->Units[Id].Troops, State->Units[Id].Morale,*ReadUnitActivity(Id).Caption);
        if (State->Units.IsValidIndex(Beat.Target) && State->Units[Beat.Target].Id >= 0)
            State->HUD.Target = FString::Printf(TEXT("対象: %s\n兵 %d / %s"), *State->Units[Beat.Target].Name, State->Units[Beat.Target].Troops,*ReadUnitActivity(Beat.Target).Caption);
        State->HUD.Report=ReadPlaybackReport();
    }
    RenderUnits();
    RenderEffects();
}
