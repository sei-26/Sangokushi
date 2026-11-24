#include "TacticalPortReadingQA.h"
#include "TacticalTrial.h"
#include "TacticalPortLandscape.h"
#include "TacticalContributionQA.h"
#include "JourneySave.hpp"
#include <memory>
using namespace frontline;
bool PreparePortReadingPreview(ATacticalField* F,int Phase,bool North)
{
    if(!F || Phase<0 || Phase>4) return false;
    if(Phase>=3) return PreparePortHandoffPreview(F,Phase==3 ? 0:2);
    if(!F->PrepareEpicPreludePreview(1)) return false;
    F->Issue(North ? ETacticalCommand::StoryReplyPreserve:ETacticalCommand::StoryReplyCoordinate);
    F->Issue(ETacticalCommand::ContinuePrelude);
    if(F->Presentation().Mission!=1 || !F->Presentation().Journey.bPreparation) return false;
    if(Phase==0) return true;
    F->Issue(ETacticalCommand::Deploy);
    if(!F->Presentation().bBattle) return false;
    if(Phase==1) return true;
    F->Issue(ETacticalCommand::AutoBattle);
    for(int I=0;I<28 && F->Presentation().bBattle;++I)
    { F->AdvanceRealtime(12.1f);F->AdvancePresentation(60);F->AdvanceRealtime(.1f);F->AdvancePresentation(60); }
    if(F->IsAutoBattle()) F->StopAutoBattle();
    auto Save=std::make_unique<JourneySaveData>();std::vector<std::uint8_t> Bytes;
    return F->ExportSession(Bytes) && DecodeJourneySave(Bytes,*Save) && Save->journey.won && Save->journey.IsPortAssault();
}
bool VerifyPortReading(ATacticalField* F)
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_PORT_READING: FAIL %s"),Why);return V; };
    std::vector<std::uint8_t> Before,After;
    for(bool North:{false,true})
    {
        if(!Check(PreparePortReadingPreview(F,0,North),TEXT("the real first battle leads to either port approach"))) return false;
        auto Save=std::make_unique<JourneySaveData>();
        if(!F->ExportSession(Before) || !DecodeJourneySave(Before,*Save)) return false;
        const auto Bridges=gunyuportlandscape::Bridges(Save->journey);
        if(!Check(Bridges.Num()==2 && Save->journey.Battlefield()==(North ? JourneyMap::PortNorth:JourneyMap::Port),TEXT("public preparation terrain contains the two actual bridges"))) return false;
        const auto& Map=F->Presentation().Journey.Map;
        for(int Tile:Bridges)
        {
            const auto* Site=Map.Sites.FindByPredicate([Tile](const FPlanningSite& S) { return S.Tile==Tile; });
            if(!Check(Site && Site->Hints.Num()==3 && !Site->Hints[0].Contains(TEXT("車が渡る")) && Site->Hints[0].Contains(TEXT("旗を立てる")),
                TEXT("capture council describes flag occupation, not a nonexistent convoy"))) return false;
        }
        F->ReadCouncilExchange(0);F->ReadCouncilExchange(1);F->ExportSession(After);
        if(!Check(Before==After,TEXT("bridge advice and public terrain reading spend and adopt nothing"))) return false;
        const int Food=Save->journey.world.cities[0].food,Cost=Save->journey.PreparationCosts(Save->preparation).TotalFood();
        F->Issue(ETacticalCommand::Deploy);F->ExportSession(After);if(!DecodeJourneySave(After,*Save)) return false;
        const auto& H=F->Presentation();
        if(!Check(Save->journey.world.cities[0].food==Food-Cost && H.PortCrossings[0].Tile==Bridges[0] && H.PortCrossings[1].Tile==Bridges[1] &&
            !H.PortCrossings[0].Caption.Contains(TEXT("突破")) && !H.PortCrossings[1].Caption.Contains(TEXT("突破")),TEXT("both bridges are named before crossing without claiming an achievement"))) return false;
        if(!Check(H.MissionDetail.Contains(TEXT("同じ隊でも勝利")),TEXT("ordinary occupation and the optional team bonus are distinguished"))) return false;
        if(!Check(PreparePortReadingPreview(F,2,North) && F->CanIssue(ETacticalCommand::Continue),TEXT("both approaches finish a real paid battle"))) return false;
        if(!F->ExportSession(Before) || !F->ImportSession(Before)) return false;
        F->Issue(ETacticalCommand::Continue);
        if(!Check(F->Presentation().Mission==2,TEXT("saved optional port victories still open the following defense mission"))) return false;
    }
    if(!Check(PreparePortReadingPreview(F,3,false) && F->Presentation().bPortOpening &&
        F->Presentation().MissionDetail.Contains(TEXT("同じ隊でも")),TEXT("actual clearance explains the ordinary win and bonus window"))) return false;
    if(!Check(PreparePortHandoffPreview(F,3),TEXT("actual crossed officers, timely clearance and another captor still earn the next-battle benefit"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_PORT_READING: PASS (real prelude, both port approaches, free council reading, paid sortie, initial bridge names, actual victories, save/load and timed cooperation)"));return true;
}
