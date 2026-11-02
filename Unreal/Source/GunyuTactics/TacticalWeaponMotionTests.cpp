#include "TacticalWeaponMotion.h"
#include "TacticalInfantry.h"
namespace gunyuweapon
{
    bool SmokeTest()
    {
        auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_WEAPON_MOTION: FAIL %s"),Why);return V; };
        TArray<FVisualUnit> Units;Units.SetNum(6);
        for(int I=0;I<4;++I) { Units[I].Id=I;Units[I].Troops=1400;Units[I].bEnemy=I==3; }
        Units[3].Position=FVector(200,0,0);
        FBattleBeat Beat;Beat.Kind=EBattleCue::Strike;Beat.Target=3;Beat.Before=Beat.After=Units;Beat.After[3].Troops-=300;
        auto Height=[](FVector P) { return float(P.X*.01+P.Y*.005); };
        for(int Id=0;Id<3;++Id)
        {
            Beat.Actor=Id;const auto E=gunyucompany::Equipment(Units[Id],3,40);
            auto At=[&](float Phase) { return Pose(gunyumotion::Read(Units[Id],Units,&Beat,Phase),E,3,Id,1.25f); };
            const auto Wind=At(.28f),Hit=At(.48f),Finish=At(.68f);
            const auto Axis=[](const gunyumotion::FPose& P) { return P.Spear.RotateVector(FVector::UpVector); };
            if(!Check(Axis(Hit).X>.9,TEXT("each weapon reaches the visible enemy at the published impact"))) return false;
            if(!Check(At(.35f).Foot[1].Z>Hit.Foot[1].Z+2,TEXT("forward foot lifts then lands with the actual blow"))) return false;
            if(Id==0 && !Check(Wind.Hand[1].Z>Hit.Hand[1].Z+10 && Axis(Finish).Y>.8,TEXT("sword cuts down across the body and follows through"))) return false;
            if(Id==1 && !Check(Axis(Wind).Y<-.8 && Axis(Finish).Y>.8,TEXT("glaive sweeps from one side of the formation to the other"))) return false;
            if(Id==2 && !Check(Hit.Hand[1].X-Wind.Hand[1].X>30 && Hit.Root.X>Wind.Root.X+10,TEXT("serpent spear drives forward with a planted stepping stance"))) return false;
            for(int N=0;N<=100;++N)
            {
                const float Phase=N/100.f;const auto P=At(Phase);
                if(N<100 && !Check(FVector::Dist(BladeTip(P,E),BladeTip(At(Phase+.001f),E))<5,TEXT("windup, impact and recovery remain continuous"))) return false;
                if(!Check(P.Spear.IsNormalized() && P.Torso.IsNormalized() && P.Foot[0].Z>=3 && P.Foot[1].Z>=3,TEXT("weapons and stepping feet stay valid"))) return false;
                for(float Stature:{.65f,1.f})
                {
                    gunyuinfantry::FBatches B;const FVector Base(123,200,12);const FQuat Heading=FRotator(0,47,0).Quaternion();
                    gunyuinfantry::Soldier(B,P,Base,Heading,E.VisualScale,Stature,E);
                    const auto& Blade=Id==0 ? B.Swords[0]:Id==1 ? B.Glaives[0]:B.Serpents[0];
                    if(Id==1 && N==48 && !Check(Blade.GetRotation().RotateVector(FVector::ForwardVector).Z>.98,TEXT("glaive keeps its cutting plane while standing or crouched"))) return false;
                    FVector Expected=BladeTip(P,E)+P.Root;Expected.Z*=Stature;Expected=Base+Heading.RotateVector(Expected*E.VisualScale);
                    if(!Check(Blade.TransformPosition(FVector(0,0,50)).Equals(Expected,.01),TEXT("sweep tip matches the rendered sword or polearm in both stances"))) return false;
                    if(Id==0)
                    {
                        FVector Grip=P.Hand[1]+P.Root;Grip.Z*=Stature;Grip=Base+Heading.RotateVector(Grip*E.VisualScale);
                        if(!Check(Blade.TransformPosition(FVector(0,0,-35)).Equals(Grip,.01),TEXT("swinging sword remains attached to the hand"))) return false;
                    }
                    else if(!Check(Blade.TransformPosition(FVector(0,0,-50)).Equals(B.Poles[0].TransformPosition(FVector(0,0,50)),.01),TEXT("sweeping blade stays attached to its shaft"))) return false;
                }
            }
            const auto Trails=Ribbons(Units,&Beat,.50f,1.25f,Height);
            if(!Check(Trails.Num()==8 && Ribbons(Units,&Beat,.35f,1.25f,Height).IsEmpty() && Ribbons(Units,&Beat,.8f,1.25f,Height).IsEmpty(),TEXT("bounded motion trail appears during the blow and clears in recovery"))) return false;
            for(const auto& Q:Trails) for(const auto& V:{Q.A,Q.B,Q.C,Q.D})
                if(!Check(!V.ContainsNaN() && FVector::Dist(V,Units[Id].Position)<350 && Q.Color.A>=0 && Q.Color.A<=1,TEXT("ribbon geometry stays finite, local and bounded"))) return false;
            const auto M=gunyumotion::Read(Units[Id],Units,&Beat,.50f);const auto P=At(.50f);
            const FQuat Heading=M.Facing.Rotation().Quaternion();
            FVector Base=Units[Id].Position+Heading.RotateVector(gunyumotion::FormationOffset(M,3,40));Base.Z=Height(Base);
            FVector Tip=BladeTip(P,E)+P.Root;Tip.Z*=M.Stature;Tip=Base+Heading.RotateVector(Tip*E.VisualScale);
            if(!Check(Trails.Last().C.Equals(Tip,.01),TEXT("current ribbon edge ends at the actual moving blade, not the victim center"))) return false;
        }
        Beat.Actor=1;
        for(auto Kind:{EBattleCue::Stop,EBattleCue::March,EBattleCue::Lure,EBattleCue::Guard,EBattleCue::Rally,EBattleCue::SerpentRoar})
        { Beat.Kind=Kind;if(!Check(Ribbons(Units,&Beat,.5f,1.25f,Height).IsEmpty(),TEXT("support, roar, movement and stopped chains invent no slash"))) return false; }
        Beat.Kind=EBattleCue::Strike;Units[3].Id=-1;
        if(!Check(Ribbons(Units,&Beat,.5f,1.25f,Height).IsEmpty(),TEXT("unknown victim cannot reveal a swing or trail"))) return false;
        Units[3]=Beat.Before[3];Units[3].Troops=0;Beat.After[3].Troops=0;
        if(!Check(!Ribbons(Units,&Beat,.5f,1.25f,Height).IsEmpty(),TEXT("a visible killing blow still follows through"))) return false;
        Units[1].Id=-1;
        if(!Check(Ribbons(Units,&Beat,.5f,1.25f,Height).IsEmpty(),TEXT("unknown actor cannot leave a trail"))) return false;
        Units[1]=Beat.Before[1];Beat.Actor=2;Beat.Kind=EBattleCue::SerpentRoar;
        const auto E=gunyucompany::Equipment(Units[2],3,40);
        const auto Idle=Pose(gunyumotion::Read(Units[2],Units,nullptr,0),E,3,2,0);
        const auto Roar=Pose(gunyumotion::Read(Units[2],Units,&Beat,.48f),E,3,2,0);
        if(!Check(Roar.Hand[1].Z>Idle.Hand[1].Z+15,TEXT("battle cry raises the weapon without inventing a thrust"))) return false;
        UE_LOG(LogTemp,Display,TEXT("GUNYU_WEAPON_MOTION: PASS (sword cut, glaive sweep, serpent drive, connected grips/sockets/tips, continuous recovery, real blade ribbons, support/hidden/stop guards)"));return true;
    }
}
