#include "TacticalTrial.h"
#include "TacticalTacticEffects.h"
#include "TacticalState.h"
#include "TacticalTargetVisual.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "ProfilingDebugging/CsvProfiler.h"

namespace
{
    void Commit(UInstancedStaticMeshComponent* Mesh, const TArray<FTransform>& Transforms)
    {
        if (Mesh->GetInstanceCount() == Transforms.Num())
            Mesh->BatchUpdateInstancesTransforms(0, Transforms, false, true, true);
        else
        {
            Mesh->ClearInstances();
            Mesh->AddInstances(Transforms, false, false);
        }
    }
}
void ATacticalField::RenderEffects()
{
    CSV_SCOPED_TIMING_STAT_GLOBAL(GunyuEffects);
    TArray<FVector> Vertices, Normals;
    TArray<int32> Triangles;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> Colors;
    TArray<FTransform> DustParts;
    auto Quad = [&](FVector A, FVector B, FVector C, FVector D, FLinearColor Color)
    {
        const int Base = Vertices.Num();
        Vertices.Append({A, B, C, D});
        Triangles.Append({Base, Base + 1, Base + 2, Base, Base + 2, Base + 3});
        for (int N = 0; N < 4; ++N) { Normals.Add(FVector::UpVector); UVs.Add(FVector2D::ZeroVector); Colors.Add(Color); }
    };
    auto Ring = [&](FVector Center, float Radius, float Width, FLinearColor Color, bool bGround = false)
    {
        for (int I = 0; I < 48; ++I)
        {
            const float A = I * 2 * PI / 48, B = (I + 1) * 2 * PI / 48;
            const FVector VA(FMath::Cos(A), FMath::Sin(A), 0), VB(FMath::Cos(B), FMath::Sin(B), 0);
            FVector P=Center+VA*Radius,Q=Center+VB*Radius,R=Center+VB*(Radius+Width),S=Center+VA*(Radius+Width);
            if (bGround)
            {
                const float Lift=Center.Z-SurfaceHeight(Center);
                for (FVector* Point : {&P,&Q,&R,&S}) Point->Z=SurfaceHeight(*Point)+Lift;
            }
            Quad(P,Q,R,S,Color);
        }
    };
    const auto* Beat = CurrentBeat();
    const float Progress = BeatProgress();
    const auto& H = State->HUD;
    if (!Beat && H.bBattle && H.bThreatOverlay)
    {
        // Estimate only: dashed hexes and one-cell route. Do not draw final
        // simulation positions during playback or any unobserved enemy data.
        const FLinearColor Threat(.95f,.23f,.09f,.66f);
        for (int Tile : H.ThreatCells)
        {
            const FVector Center = SurfaceLocation(Tile) + FVector(0,0,12);
            for (int Edge = 0; Edge < 6; ++Edge)
            {
                const float A = (Edge * 60.f + 30.f) * PI / 180.f;
                const float B = ((Edge + 1) * 60.f + 30.f) * PI / 180.f;
                FVector P = Center + FVector(FMath::Cos(A),FMath::Sin(A),0) * 105;
                FVector Q = Center + FVector(FMath::Cos(B),FMath::Sin(B),0) * 105;
                P.Z=SurfaceHeight(P)+12; Q.Z=SurfaceHeight(Q)+12;
                const FVector Side = FVector::CrossProduct((Q-P).GetSafeNormal(), FVector::UpVector) * 2;
                Quad(P-Side, P+Side, Q+Side, Q-Side, Threat);
            }
        }
        if (H.ThreatFrom >= 0 && H.ThreatNext >= 0 && H.ThreatFrom != H.ThreatNext)
        {
            const FVector A = SurfaceLocation(H.ThreatFrom) + FVector(0,0,16);
            const FVector B = SurfaceLocation(H.ThreatNext) + FVector(0,0,16);
            const FVector Side = FVector::CrossProduct((B-A).GetSafeNormal(), FVector::UpVector) * 4;
            for (int I = 0; I < 5; ++I)
            {
                const FVector P = FMath::Lerp(A,B,I/5.f), Q = FMath::Lerp(A,B,(I+.55f)/5.f);
                Quad(P-Side, P+Side, Q+Side, Q-Side, Threat);
            }
            Ring(B, 23, 4, Threat,true);
        }
    }
    if(!Beat && H.bBattle && H.bTacticTargetOverlay)
        for(const auto& Read:H.TacticTargets)
        {
            if(!Read.bReady) continue;
            const auto Center=SurfaceLocation(Read.Tile);auto Color=gunyutargetui::Color(Read);
            Color.A=Read.bSelected ? .95f:.65f;
            // Outside the team ring: dashed corners mark a prediction, not a hit.
            for(int Edge=0;Edge<6;++Edge) for(int Segment=0;Segment<2;++Segment)
            {
                const float A=(Edge*60.f+30.f)*PI/180.f,B=((Edge+1)*60.f+30.f)*PI/180.f;
                const auto P=Center+FVector(FMath::Cos(A),FMath::Sin(A),0)*117;
                const auto Q=Center+FVector(FMath::Cos(B),FMath::Sin(B),0)*117;
                auto V=FMath::Lerp(P,Q,Segment==0 ? 0.f:.74f),W=FMath::Lerp(P,Q,Segment==0 ? .26f:1.f);
                V.Z=SurfaceHeight(V)+18;W.Z=SurfaceHeight(W)+18;
                const auto Side=FVector::CrossProduct((W-V).GetSafeNormal(),FVector::UpVector)*(Read.bSelected ? 3.5f:2.f);
                Quad(V-Side,V+Side,W+Side,W-Side,Color);
            }
        }
    // Soft formation footprint and a segmented facing-independent selection halo.
    for (const auto& Unit : State->Units)
    {
        if (Unit.Id < 0 || Unit.Troops <= 0) continue;
        const FLinearColor Team = Unit.bEnemy ? FLinearColor(1.f, .24f, .08f, .8f) : FLinearColor(.18f, .72f, .88f, .75f);
        Ring(Unit.Position + FVector(0, 0, 7), 79, Unit.Id == State->Selected ? 6 : 2, Unit.Id == State->Selected ? FLinearColor(1.f, .75f, .22f, 1) : Team,true);
        if (Unit.Id == State->Selected)
            Ring(Unit.Position + FVector(0, 0, 5), 91, 1.5f, FLinearColor(1.f, .8f, .4f, .55f),true);
        if (Unit.bDisordered || Unit.bPressured || Unit.bInspired)
            Ring(Unit.Position + FVector(0, 0, 9), 85, 2.5f, Unit.bInspired ? FLinearColor(1,.82f,.25f,.8f) : Unit.bDisordered ? FLinearColor(.8f,.35f,1,.8f) : FLinearColor(1,.5f,.15f,.8f),true);
        if (Unit.Motion > 0)
            for (int I = 0; I < 7; ++I)
            {
                const float Age = FMath::Frac(Progress * 3 + I / 7.f);
                FVector Direction(1, 0, 0);
                if (Beat && Beat->Before.IsValidIndex(Unit.Id) && Beat->After.IsValidIndex(Unit.Id))
                    Direction = (Beat->After[Unit.Id].Position - Beat->Before[Unit.Id].Position).GetSafeNormal();
                const FVector Across(-Direction.Y, Direction.X, 0);
                const FVector Offset = -Direction * (40 + Age * 60) + Across * ((I - 3) * 19) + FVector(0, 0, 10 + Age * 20);
                DustParts.Add(FTransform(FRotator::ZeroRotator, Unit.Position + Offset, FVector(.16 + Age * .28, .16 + Age * .28, .08 + Age * .15)));
            }
    }
    if (Beat)
    {
        const int FocusId = Beat->Target >= 0 ? Beat->Target : Beat->Actor;
        if (State->Units.IsValidIndex(FocusId) && State->Units[FocusId].Id >= 0)
        {
            const FVector Center = State->Units[FocusId].Position + FVector(0, 0, 12);
            const float Pulse = FMath::Clamp((Progress - .40f) / .5f, 0.f, 1.f);
            const bool bHit = Beat->Kind == EBattleCue::Strike || Beat->Kind == EBattleCue::Disrupt;
            FLinearColor Color = Beat->Kind == EBattleCue::Ambush ? FLinearColor(.20f, 1.f, .5f, 1) : Beat->Kind == EBattleCue::Lure ? FLinearColor(.15f, .75f, 1.f, 1) : FLinearColor(1.f, .52f, .12f, 1);
            if (Beat->Kind == EBattleCue::AzureDragon) Color = FLinearColor(.12f,1.f,.65f,1);
            if (Beat->Kind == EBattleCue::Rally) Color = FLinearColor(1.f,.85f,.25f,1);
            if (Beat->Kind == EBattleCue::SerpentRoar) Color = FLinearColor(1.f,.23f,.10f,1);
            Color.A = 1 - Pulse;
            if (Progress > .40f && Progress < .90f && Beat->Kind != EBattleCue::Stop)
            {
                Ring(Center, 35 + Pulse * 150, 8 * (1 - Pulse) + 1, Color);
                if (Beat->Kind == EBattleCue::SerpentRoar)
                    for (int I = 1; I < 4; ++I) Ring(Center, (25 + Pulse * 140) * I * .65f, 5, Color);
                if (Beat->Kind == EBattleCue::Rally)
                    for (const auto& Ally : State->Units)
                        if (Ally.Id >= 0 && Ally.bInspired && !Ally.bEnemy) Ring(Ally.Position + FVector(0,0,10), 55 + Pulse * 65, 5, Color);
                if (bHit)
                    for (int I = 0; I < 12; ++I)
                    {
                        const float Angle = I * 2 * PI / 12;
                        const FVector Ray(FMath::Cos(Angle), FMath::Sin(Angle), .6);
                        const FVector Side(-Ray.Y * 3, Ray.X * 3, 0);
                        const FVector A = Center + Ray * (25 + Pulse * 70), B = Center + Ray * (80 + Pulse * 100);
                        Quad(A - Side, A + Side, B + Side, B - Side, Color);
                    }
                if (bHit)
                {
                    // Curved weapon trails rise out of the battle, grounded at the real victim.
                    for (int Arc = 0; Arc < 3; ++Arc) for (int Segment = 0; Segment < 18; ++Segment)
                    {
                        const float A = (Segment / 18.f) * PI * .92f;
                        const float B = ((Segment + 1) / 18.f) * PI * .92f;
                        const float R = (65 + Arc * 23 + Pulse * 85) * (Beat->Kind == EBattleCue::AzureDragon ? 1.4f : 1.f);
                        const FVector Axis = FVector(FMath::Cos(Arc * 1.9f), FMath::Sin(Arc * 1.9f), 0);
                        const FVector P = Center + Axis * (FMath::Cos(A) * R) + FVector(0, 0, FMath::Sin(A) * R * .85f);
                        const FVector Q = Center + Axis * (FMath::Cos(B) * R) + FVector(0, 0, FMath::Sin(B) * R * .85f);
                        const FVector Side = FVector::CrossProduct((Q - P).GetSafeNormal(), FVector(0, -1, .5f)).GetSafeNormal() * (4 + 8 * (1 - Pulse));
                        Quad(P - Side, P + Side, Q + Side, Q - Side, Color);
                    }
                    for (int I = 0; I < 14; ++I)
                    {
                        const float Angle = I * 2.4f;
                        const FVector P = Center + FVector(FMath::Cos(Angle) * Pulse * 130, FMath::Sin(Angle) * Pulse * 130, FMath::Sin(Pulse * PI) * (18 + I * 3));
                        DustParts.Add(FTransform(FRotator::ZeroRotator, P, FVector(.20 + Pulse * .45, .20 + Pulse * .45, .1 + Pulse * .25)));
                    }
                }
            }
            if (Beat->Before.IsValidIndex(Beat->Actor) && Beat->Before.IsValidIndex(Beat->Target) && Beat->Kind != EBattleCue::Stop)
            {
                const FVector A = State->Units[Beat->Actor].Position + FVector(0, 0, 20), B = Center;
                const FVector Side = FVector::CrossProduct((B - A).GetSafeNormal(), FVector::UpVector) * 4;
                Color.A = .65f;
                Quad(A - Side, A + Side, B + Side, B - Side, Color);
            }
        }
    }
    if(Beat) gunyufx::Tactic(*Beat,State->Units,Progress,Quad);
    const auto* Section=Effects->GetProcMeshSection(0);
    // All effects use independent quads, so equal counts mean equal topology.
    if (Section && Section->ProcVertexBuffer.Num()==Vertices.Num() && Section->ProcIndexBuffer.Num()==Triangles.Num())
        Effects->UpdateMeshSection_LinearColor(0,Vertices,Normals,UVs,Colors,{},false);
    else Effects->CreateMeshSection_LinearColor(0,Vertices,Triangles,Normals,UVs,Colors,{},false);
    Commit(Dust, DustParts);
}

void ATacticalField::PlayCueSound(bool bImpact)
{
    const auto* Beat = CurrentBeat();
    if (!Beat || EffectsVolume <= 0 || State->bNoPlayback || Beat->Kind==EBattleCue::Stop) return;
    if (bImpact && (Beat->Kind == EBattleCue::March || Beat->Kind == EBattleCue::Guard || Beat->Kind == EBattleCue::Conceal || Beat->Kind == EBattleCue::Stop || Beat->Kind == EBattleCue::Withdraw || Beat->Kind == EBattleCue::Rally)) return;
    const TCHAR* Path = bImpact ? TEXT("/Game/Audio/S_Impact.S_Impact") : Beat->Kind == EBattleCue::March || Beat->Kind == EBattleCue::Lure || Beat->Kind == EBattleCue::Enemy ? TEXT("/Game/Audio/S_March.S_March") : TEXT("/Game/Audio/S_Command.S_Command");
    if (auto* Sound = LoadObject<USoundBase>(nullptr, Path))
    {
        const float Pitch=Beat->Kind==EBattleCue::SerpentRoar ? .76f : Beat->Kind==EBattleCue::AzureDragon ? 1.13f : Beat->Kind==EBattleCue::Rally ? .9f : 1.f;
        CueAudio = UGameplayStatics::SpawnSound2D(this, Sound, FMath::Clamp(EffectsVolume, 0.f, 1.f),Pitch);
        UE_LOG(LogTemp, Display, TEXT("GUNYU_CUE_AUDIO: %s playing=%d"), *Sound->GetName(), CueAudio && CueAudio->IsPlaying());
    }
}
