#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Notifications/SProgressBar.h"
using namespace gunyuui;
void ATacticalController::BuildRosterInterface(TSharedRef<SCanvas> Canvas)
{
    auto Roster=SNew(SHorizontalBox);
    for(int Index=0;Index<3;++Index)
    {
        auto Card=SNew(SBorder).Padding(1).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor_Lambda([this,Index] { return Field->Presentation().Selected==Index ? Gold : FLinearColor(.18f,.23f,.24f); })
            [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(8)
             .ToolTipText_Lambda([this,Index] { const auto& O=Field->Presentation().Officers;return FText::FromString(O.IsValidIndex(Index) ? O[Index].Duty:FString()); })
             .IsEnabled_Lambda([this,Index] { return (!Field->IsPlaying() || Field->bRealtimeCommand) && Field->Presentation().Officers.IsValidIndex(Index) && Field->Presentation().Officers[Index].bLiving; })
             .OnClicked_Lambda([this,Index] { Field->SelectOfficer(Index);return FReply::Handled(); })
             [SNew(SHorizontalBox)
              + SHorizontalBox::Slot().AutoWidth()[PortraitWidget([Index] { return Index; },66,84)]
              + SHorizontalBox::Slot().FillWidth(1).Padding(12,0)
                [SNew(SVerticalBox)
                 + SVerticalBox::Slot().AutoHeight()[Label([this,Index] { const auto& O=Field->Presentation().Officers;return O.IsValidIndex(Index) ? O[Index].Name : FString(); },20,Gold)]
                 + SVerticalBox::Slot().AutoHeight().Padding(0,2)[Label([this,Index]
                    { const auto& U=Field->VisualUnits();return U.IsValidIndex(Index) ? FString::Printf(TEXT("兵 %d    士気 %d"),U[Index].Troops,U[Index].Morale) : FString(); },13)]
                 + SVerticalBox::Slot().AutoHeight()[Label([this,Index]
                    { const auto& O=Field->Presentation().Officers;return O.IsValidIndex(Index) ? O[Index].Tactic+TEXT(" / ")+O[Index].Status.Mid(O[Index].Status.Find(TEXT("\n"))+1).Replace(TEXT("\n"),TEXT("・")) : FString(); },12,FLinearColor(.48f,.82f,.78f))]
                 + SVerticalBox::Slot().AutoHeight().Padding(0,6)[SNew(SBox).HeightOverride(3)
                    [SNew(SProgressBar).Style(&ProgressStyle()).Percent_Lambda([this,Index] { const auto& U=Field->VisualUnits();return U.IsValidIndex(Index) ? FMath::Clamp(U[Index].Morale/100.f,0.f,1.f):0.f; })
                     .FillColorAndOpacity(FLinearColor(.23f,.62f,.57f))]]]]];
        Roster->AddSlot().FillWidth(1).Padding(0,0,Index<2 ? 10:0,0)[Card];
    }
    AddBattlePanel(Canvas,FVector2D(16,776),FVector2D(1240,108),Roster,4);
}
