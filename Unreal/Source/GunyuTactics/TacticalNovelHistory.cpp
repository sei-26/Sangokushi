#include "TacticalTrial.h"
#include "TacticalWidgets.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
using namespace gunyuui;
void ATacticalController::BuildNovelHistory(TSharedRef<SCanvas> Canvas)
{
    auto Rows=SNew(SVerticalBox);
    for(int Index=0;Index<32;++Index)
    {
        auto Read=[this,Index] { return EpisodeSnapshot.Lines.IsValidIndex(Index) ? EpisodeSnapshot.Lines[Index]:FDialoguePresentation{}; };
        Rows->AddSlot().AutoHeight().Padding(0,8)
            [SNew(SBox).Visibility_Lambda([this,Index] { return Index<=EpisodePage && EpisodeSnapshot.Lines.IsValidIndex(Index) ? EVisibility::Visible:EVisibility::Collapsed; })
             [SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Name; },19,Gold)]
              + SVerticalBox::Slot().AutoHeight().Padding(0,6)[Label([this,Index,Read] { return NovelHistoryText(Index); },22)]
              + SVerticalBox::Slot().AutoHeight()[Label([Read] { return Read().Direction; },14,FLinearColor(.55f,.57f,.54f))]]];
    }
    auto Panel=SNew(STacticalChrome).Padding(30).Opacity(1.f).Visibility_Lambda([this] { return bEpisodeOpen && bNovelHistory ? EVisibility::Visible:EVisibility::Collapsed; })
        [SNew(SVerticalBox)
         + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1)[Label([] { return TEXT("言葉の記録"); },30,Gold)]
            + SHorizontalBox::Slot().AutoWidth()[Button([] { return TEXT("戻る / L・Esc"); },[this] { bNovelHistory=false; })]]
         + SVerticalBox::Slot().FillHeight(1).Padding(0,18)[SNew(SScrollBox)+SScrollBox::Slot()[Rows]]];
    InputPanels.Add(Panel);Canvas->AddSlot().Position(FVector2D(110,112)).Size(FVector2D(1380,710))[Panel];
}
