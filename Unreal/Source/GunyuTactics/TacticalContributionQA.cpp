#include "TacticalContributionQA.h"
#include "TacticalTrial.h"
#include "../../../tests/JourneyContributionPlayer.hpp"
bool PrepareContributionPreview(ATacticalField* Field,int Variant)
{
    if(!Field || Variant<0 || Variant>3) return false;
    Field->Issue(ETacticalCommand::Skip);
    auto Session=contributionqa::Play(Variant);if(!Session) return false;
    std::vector<std::uint8_t> Bytes;if(!frontline::EncodeJourneySave(*Session,Bytes)) return false;
    Field->bLegacyJourney=false;return Field->ImportSession(Bytes);
}
bool PrepareCooperationCouncilPreview(ATacticalField* Field,int Variant)
{
    if(!Field || Variant<0 || Variant>3) return false;
    Field->Issue(ETacticalCommand::Skip);auto Session=contributionqa::Play(Variant);
    if(!Session || !Session->journey.Continue()) return false;
    Session->beforeBattle=Session->journey;Session->realtimeMode=true;
    std::vector<std::uint8_t> Bytes;if(!frontline::EncodeJourneySave(*Session,Bytes)) return false;
    Field->bLegacyJourney=false;return Field->ImportSession(Bytes);
}

bool PreparePortHandoffPreview(ATacticalField* Field,int Stage)
{
    using namespace frontline;
    using namespace contributionqa;
    if(!Field || Stage<0 || Stage>3) return false;
    Field->Issue(ETacticalCommand::Skip);
    const auto Session=PortApproach();
    std::vector<std::uint8_t> Bytes;
    if(!Session || !EncodeJourneySave(*Session,Bytes) || !Field->ImportSession(Bytes)) return false;
    auto Move=[Field](int Officer,int Tile)
    {
        Field->SelectOfficer(Officer);
        Field->Issue(ETacticalCommand::Move);
        Field->ClickTile(Tile);
    };
    Move(1,Campaign::At(12,8));
    Field->AdvancePresentation(10);
    Move(2,PortBridgeExit);
    if(!Field->IsPlaying() || Field->Presentation().PortCrossings[1].Tile>=0) return false;
    Field->AdvancePresentation(10);
    if(!Field->Presentation().PortCrossings[0].Caption.Contains(TEXT("関羽")) ||
        !Field->Presentation().PortCrossings[1].Caption.Contains(TEXT("張飛")) || Field->Presentation().bPortOpening) return false;
    Field->SelectAt(PortBlocker);
    Field->SelectTactic(int(ChainTactic::Breakthrough));
    Field->Issue(ETacticalCommand::QueueTactic);
    Field->Issue(ETacticalCommand::Execute);
    if(!Field->IsPlaying() || Field->Presentation().bPortOpening) return false;
    Field->AdvancePresentation(10);
    if(!Field->Presentation().bPortOpening || !Field->Presentation().MissionStep.Contains(TEXT("張飛隊が道を開いた"))) return false;
    if(Stage==0) return true;
    Move(0,Session->journey.goal);
    const auto* Beat=Field->CurrentBeat();
    if(!Beat || Beat->PortCaptor!=0 || Field->CueReason().Contains(TEXT("旗を立てた"))) return false;
    Field->AdvancePresentation(Beat->Duration*.9f);
    if(!Field->IsPlaying() || !Field->CueReason().Contains(TEXT("劉備隊が指揮所に旗を立てた"))) return false;
    if(Stage==1) return true;
    Field->AdvancePresentation(10);
    if(!Field->Presentation().Journey.CampaignChange.Contains(TEXT("道を開いた：張飛")) ||
        !Field->Presentation().Journey.CampaignChange.Contains(TEXT("河港に旗を立てた：劉備"))) return false;
    if(Stage==2) return true;
    Field->Issue(ETacticalCommand::Continue);
    return Field->Presentation().Mission==2 && Field->Presentation().Journey.PlanObjective.Contains(TEXT("２指揮"));
}
