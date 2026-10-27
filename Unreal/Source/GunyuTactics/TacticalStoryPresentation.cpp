#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;
void ATacticalField::BuildStoryPresentation()
{
    const auto& J=State->Journey;auto& P=State->HUD.Journey;
    if(!P.bPreparation && !P.bDebrief && !P.bComplete) return;
    P.PersonalTrust=FString::Printf(TEXT("主人公への信頼  劉備 %d / 関羽 %d / 張飛 %d"),J.officerTrust[0],J.officerTrust[1],J.officerTrust[2]);
    if(P.bPreparation)
    {
        const int Base=State->PreparationChoice==1 ? 100:90;
        const int L=FMath::Clamp(Base+J.StoryMoraleBonus(0),0,100)-Base;
        const int G=FMath::Clamp(Base+J.StoryMoraleBonus(1),0,100)-Base;
        const int Z=FMath::Clamp(Base+J.StoryMoraleBonus(2),0,100)-Base;
        P.PersonalTrust+=FString::Printf(TEXT("  / 出陣士気 %+d・%+d・%+d（上限100）"),L,G,Z);
        if(J.CurrentPledge()!=JourneyPledge::None) P.PledgeText=TEXT("主人公が交わした約束：")+JourneyText(PledgeName(J.CurrentPledge()));
    }
    const auto& Story=J.history[J.mission].story;
    if(P.bDebrief || P.bComplete)
    {
        P.StoryTitle=JourneyText(BattleStoryName(Story,J.won,J.withdrawn));
        if(Story.recorded)
        {
            P.StoryFacts=FString::Printf(TEXT("この戦いでの主人公への信頼：劉備 %+d / 関羽 %+d / 張飛 %+d"),Story.trustDelta[0],Story.trustDelta[1],Story.trustDelta[2]);
            if(Story.pledge!=JourneyPledge::None) P.StoryFacts+=TEXT("\n出陣前の約束：")+JourneyText(PledgeName(Story.pledge))+(Story.pledgeKept ? TEXT(" → 果たした"):TEXT(" → 果たせなかった"));
            if(Story.reply!=JourneyPledge::None) P.PledgeText=TEXT("次の戦いで交わした約束：")+JourneyText(PledgeName(Story.reply));
        }
        P.bNeedsReply=J.NeedsStoryReply();
        if(P.bNeedsReply)
        {
            P.ReplyPrompt=TEXT("主人公として、次の戦いをどう約束する？ 返答だけで信頼は増えず、次戦の行動で評価される。");
            P.PrimaryLabel=TEXT("主人公の返答を選んで次へ");
        }
    }
    const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛"),TEXT("主人公")};
    const auto Lines=JourneyConversation(J,State->Interlocutor);P.Dialogue.Reset();
    for(const auto& Line:Lines)
    {
        FDialoguePresentation D;D.Speaker=Line.speaker;D.Name=Names[FMath::Clamp(Line.speaker,0,3)];D.Text=JourneyText(Line.text);D.bProtagonist=Line.speaker==3;
        P.Dialogue+=D.Name+TEXT("「")+D.Text+TEXT("」\n");P.Conversation.Add(MoveTemp(D));
    }
}
