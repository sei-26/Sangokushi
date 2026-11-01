#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SBox.h"
void ATacticalController::BuildEnsemblePortraits(TSharedRef<SCanvas> Canvas,bool Council)
{
    for(int I=0;I<3;++I)
    {
        auto Emphasis=[this,I,Council]
        {
            if(Council) return CouncilPresence[I];
            const float T=FMath::SmoothStep(0.f,1.f,FMath::Clamp(NovelLineTime/.28f,0.f,1.f));
            return FMath::Lerp(NovelPreviousSpeaker==I ? 1.f:0.f,CurrentEpisodeLine().Speaker==I ? 1.f:0.f,T);
        };
        auto Portrait=SNew(SBox).Visibility_Lambda([this,Council]
          { const bool Visible=Council ? CouncilSceneVisible():bEpisodeOpen && !bNovelPlan && IsEnsembleEpisode() && CurrentEpisodeLine().Speaker>=0;
            return Visible ? EVisibility::HitTestInvisible:EVisibility::Collapsed; })
          [SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
           [SNew(SImage).Image_Lambda([this,I] { return NovelPortraitBrush(I); })
            .ColorAndOpacity_Lambda([this,Emphasis,Council]
             { const float Light=.88f+.12f*Emphasis();const float Entrance=FMath::Clamp((Council ? CouncilStageTime:NovelStageTime)/.35f,0.f,1.f);
               return FLinearColor(Light,Light,Light,Entrance); })]];
        Canvas->AddSlot().Position_Lambda([this,I,Council,Emphasis]
          { const float Entrance=FMath::SmoothStep(0.f,1.f,FMath::Clamp((Council ? CouncilStageTime:NovelStageTime)/.35f,0.f,1.f));
            return FVector2D(-58+I*522-12*Emphasis(),26+32*(1-Entrance)-16*Emphasis()); })
          .Size_Lambda([Emphasis] { return FVector2D(610+24*Emphasis(),915+36*Emphasis()); })[Portrait];
    }
}
