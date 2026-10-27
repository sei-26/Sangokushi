#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SUniformGridPanel.h"

using namespace gunyuui;

TSharedRef<SWidget> ATacticalController::TacticPickerButton()
{
    return Button([this]
    {
        FString Name=TEXT("戦法：")+Field->Presentation().SelectedTacticName;
        for(const auto& T:Field->Presentation().Tactics) if(T.bSelected && T.bUsed) return T.Name+TEXT("［済］ ▼");
        return Name+TEXT(" ▼");
    }, [this]
    {
        bTacticPickerOpen = !bTacticPickerOpen;
        IgnoreMapInputThroughFrame = Frames + 1;
    }, [this] { return !Field->IsPlaying() && !Field->Presentation().Tactics.IsEmpty(); });
}

void ATacticalController::BuildTacticInterface(TSharedRef<SCanvas> Canvas)
{
    auto Content = SNew(SVerticalBox);
    Content->AddSlot().AutoHeight()[Label([] { return TEXT("連鎖に使う戦法を選ぶ"); }, 19, Gold)];
    Content->AddSlot().AutoHeight().Padding(0, 5, 0, 8)
        [Label([] { return TEXT("1指揮1手 / 選択・予約では消費しない"); }, 12)];
    auto Options = SNew(SUniformGridPanel).SlotPadding(FMargin(3));
    for (int Index = 0; Index < 4; ++Index)
        Options->AddSlot(Index % 2, Index / 2)
        [SNew(SButton).ButtonStyle(&CommandStyle()).IsFocusable(false).ContentPadding(9)
         .Visibility_Lambda([this, Index] { return Field->Presentation().Tactics.IsValidIndex(Index) ? EVisibility::Visible : EVisibility::Collapsed; })
         .ButtonColorAndOpacity_Lambda([this, Index]
         {
             const auto& T = Field->Presentation().Tactics;
             return FSlateColor(T.IsValidIndex(Index) && T[Index].bSelected ? FLinearColor(2.7f,2.25f,1.3f) : FLinearColor::White);
         })
         .OnClicked_Lambda([this, Index]
         {
             const auto& T = Field->Presentation().Tactics;
             if (T.IsValidIndex(Index)) Field->SelectTactic(T[Index].Id);
             bTacticPickerOpen = false; IgnoreMapInputThroughFrame = Frames + 1;
             return FReply::Handled();
         })
         [SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()[Label([this, Index]
            {
                const auto& T = Field->Presentation().Tactics;
                return T.IsValidIndex(Index) ? (T[Index].bUnique ? TEXT("固有  ") : TEXT("通常  ")) + T[Index].Name : FString();
            }, 17, Gold)]
          + SVerticalBox::Slot().AutoHeight().Padding(0, 4)[Label([this, Index] { const auto& T = Field->Presentation().Tactics; return T.IsValidIndex(Index) ? T[Index].Description : FString(); }, 12)]
          + SVerticalBox::Slot().AutoHeight()[Label([this, Index] { const auto& T = Field->Presentation().Tactics; return T.IsValidIndex(Index) ? T[Index].Cost : FString(); }, 11, FLinearColor(.55f,.83f,.78f))] ]];
    Content->AddSlot().FillHeight(1)[Options];
    auto Panel = SNew(STacticalChrome).Padding(16).Accent(FLinearColor(.56f,.42f,.18f))
        .Visibility_Lambda([this] { return bTacticPickerOpen && Field->Presentation().bBattle && !Field->IsPlaying() ? EVisibility::Visible : EVisibility::Collapsed; })[Content];
    InputPanels.Add(Panel);
    Canvas->AddSlot().Position(FVector2D(480, 126)).Size(FVector2D(700, 392))[Panel];
}
