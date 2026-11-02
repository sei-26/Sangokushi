#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include "TacticalWidgets.h"
#include <memory>
bool ATacticalController::CouncilSceneSmokeTest()
{
    using namespace frontline;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_COUNCIL_SCENE: FAIL %s"),Why);return V; };
    const bool PreviousLegacy=Field->bLegacyJourney;
    CloseEpisode();Field->bLegacyJourney=false;
    auto Save=std::make_unique<JourneySaveData>();Save->journey.ResetCampaign();Save->journey.Meet();
    const auto InitialCamp=Save->journey.world.cities[0];
    std::vector<std::uint8_t> Bytes,Before,After;
    if(!Check(EncodeJourneySave(*Save,Bytes) && Field->ImportSession(Bytes),TEXT("real unpaid campaign council"))) return false;
    Field->ExportSession(Before);OpenJourneyEntry();
    if(!Check(ReactionTextures.Num()==3 && ReactionTextures[0] && ReactionTextures[1] && ReactionTextures[2],TEXT("three transparent identity-preserving reaction portraits are loaded"))) return false;
    if(!Check(CouncilSceneVisible() && CouncilTopic==-1 && IsEnsembleEpisode(),TEXT("ordinary council opens with three officers instead of the ledger"))) return false;
    for(int I=0;I<3;++I)
    {
        if(!Check(ReactionBrushes[I]->ImageSize==FVector2D(1024,1536),TEXT("expression swaps preserve the verified source aspect even before texture streaming finishes"))) return false;
        OpenCouncilVoice(I);AdvanceCouncilScene(.1f);
        if(!Check(CouncilTopic==I && CurrentCouncilLine().Speaker==I && CouncilPresence[I]>0 && !VisibleCouncilText().IsEmpty(),TEXT("each officer owns a distinct voiced proposal with active presence"))) return false;
        if(I==0 && !Check(NovelPortraitBrush(0)==ReactionBrushes[0].Get(),TEXT("authored warm expression replaces the neutral portrait without changing game data"))) return false;
        CouncilElapsed=1000;NextCouncilLine();HandleEpisodeKey(EKeys::Left);
        if(!Check(CouncilPage==0 && VisibleCouncilText()==CurrentCouncilLine().Text,TEXT("previous line returns with its full text and no adopted choice"))) return false;
        for(int L=0;L<12;++L) { CouncilElapsed=1000;NextCouncilLine(); }
        if(!Check(CouncilSceneVisible() && Field->ExportSession(After) && Before==After,TEXT("listening/animation/repeating does not adopt, spend, deploy or award trust"))) return false;
    }
    OpenCouncilVoice(2);CouncilChoice(0);
    if(!Check(Field->Presentation().Journey.Plans[1].bSelected && CurrentCouncilLine().Speaker==3 && !CouncilReaction(2).IsEmpty(),TEXT("player's answer selects the actual reserve preparation and prompts group reactions"))) return false;
    Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    const auto Camp=Save->journey.world.cities[0];
    if(!Check(Camp.food==InitialCamp.food && Camp.gold==InitialCamp.gold && Save->journey.reserveFood==0 && Save->journey.trust==0,TEXT("adoption does not pay early or farm trust"))) return false;
    bCouncilLedger=true;if(!Check(!CouncilSceneVisible(),TEXT("the detailed ledger owns input"))) return false;
    HandleEpisodeKey(EKeys::Escape);if(!Check(CouncilSceneVisible(),TEXT("escape returns from details to the same council"))) return false;
    OpenCouncilVoice(0);CouncilChoice(1);
    if(!Check(Field->Presentation().Journey.bRescue && CurrentCouncilLine().Speaker==3,TEXT("the spoken rescue choice changes the real objective"))) return false;
    const int Food=Camp.food,Gold=Camp.gold;
    Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);const auto Cost=Save->journey.PreparationCosts(1);
    HandleEpisodeKey(EKeys::Enter);HandleEpisodeKey(EKeys::Enter);
    if(!Check(Field->Presentation().Journey.bPreparation,TEXT("advancing the exchange never deploys"))) return false;
    DeployNovelPlan();Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(!bEpisodeOpen && Save->journey.HasConvoy() && Save->journey.preparation==1 && Save->journey.reserveFood==1800 && Save->journey.world.cities[0].food==Food-Cost.TotalFood() && Save->journey.world.cities[0].gold==Gold-Cost.gold,TEXT("explicit deployment pays the selected reserve/escort exactly once"))) return false;
    const auto Paid=Bytes;ChooseCouncilDecision(3);DeployNovelPlan();Field->ExportSession(After);
    if(!Check(Paid==After,TEXT("stale council actions cannot change the deployed battle"))) return false;
    Field->bLegacyJourney=PreviousLegacy;CloseEpisode();UE_LOG(LogTemp,Display,TEXT("GUNYU_COUNCIL_SCENE: PASS (three-person stage, distinct proposals, immutable listening, player answer and reactions, real preparation/objective, details ownership and one-time paid deployment)"));return true;
}
