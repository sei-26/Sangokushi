#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyEpisode.hpp"
using namespace frontline;

namespace gunyugrand
{
    bool SmokeTest()
    {
        FGrandProgress A;A.bActive=A.bBattle=true;A.Mission=CoalitionMission;
        auto B=A;B.Rescued=6;B.bRetreat=true;
        bool Passed=Compare(A,A).Kind==EGrandMoment::None && Compare({},B).Kind==EGrandMoment::None && Compare(A,B).Kind==EGrandMoment::Relief && Compare(A,B).Reply.Contains(TEXT("橋"));
        B.bRetreat=false;B.Opened=B.Rallied=6;
        Passed=Passed && Compare(A,B).Kind==EGrandMoment::Rally && Compare(A,B).Title.Contains(TEXT("援軍"));
        B.Mission=YangrenMission;Passed=Passed && Compare(A,B).Kind==EGrandMoment::None;
        A.Mission=YangrenMission;B=A;B.Rallied=B.Opened=B.Counter=5;
        Passed=Passed && Compare(A,B).Kind==EGrandMoment::Counter && Compare(A,B).Speaker==LuBu;
        B.bBattle=false;Passed=Passed && Compare(A,B).Kind==EGrandMoment::None;
        B.bWon=true;Passed=Passed && Compare(A,B).Kind==EGrandMoment::Victory;
        UE_LOG(LogTemp,Display,TEXT("GUNYU_GRAND_MOMENT_READ: %s (actual changes, no repetition, deployment and mission guards, combined counterattack and terminal outcome)"),Passed ? TEXT("PASS"):TEXT("FAIL"));return Passed;
    }
}

bool ATacticalField::PrepareGrandMomentPreview(int Kind)
{
    if(Kind<0 || Kind>3) return false;
    const bool Yangren=Kind==1;
    if(!PrepareGrandBattlePreview(Yangren ? YangrenMission:CoalitionMission,0,Yangren ? 2:Kind==2 ? 1:2)) return false;
    State->bNoPlayback=false;bPauseOnBattleAlert=false;Issue(ETacticalCommand::AutoBattle);
    const auto Desired=Kind==0 ? EGrandMoment::Relief:Kind==1 ? EGrandMoment::Counter:Kind==2 ? EGrandMoment::Rally:EGrandMoment::Victory;
    bool Found=false;
    for(int Cycle=0;Cycle<40 && State->Journey.stage==JourneyStage::Battle;++Cycle)
    {
        for(float Delta:{12.1f,.1f})
        {
            AdvanceRealtime(Delta);
            if(IsPlaying() && ReadGrandMoment().Kind!=EGrandMoment::None) return false;
            AdvancePresentation(60);
            if(State->GrandMoment.Kind==Desired) { Found=true;break; }
        }
        if(Found) break;
    }
    if(IsAutoBattle()) StopAutoBattle();
    AdvancePresentation(.35f);
    const bool Valid=Found && ReadGrandMoment().Weight>.9f;
    if(!Valid) UE_LOG(LogTemp,Warning,TEXT("GUNYU_GRAND_MOMENT_DIAGNOSTIC: kind=%d found=%d read=%d weight=%.2f stage=%d turn=%d opened=%d rallied=%d rescued=%d counter=%d won=%d"),
        Kind,Found,int(ReadGrandMoment().Kind),ReadGrandMoment().Weight,int(State->Journey.stage),State->Journey.turn,
        State->Journey.battleReview.grand.openedOn,State->Journey.battleReview.grand.ralliedOn,State->Journey.battleReview.grand.coalition.rescuedOn,State->Journey.battleReview.grand.yangren.counterOn,State->Journey.won);
    return Valid;
}
bool ATacticalField::GrandMomentSmokeTest()
{
    if(!gunyugrand::SmokeTest()) return false;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_GRAND_MOMENT: FAIL %s"),Why);return V; };
    for(int Kind:{0,1,2,3})
    {
        if(!Check(PrepareGrandMomentPreview(Kind),TEXT("actual automatic actions reach the expected fully displayed milestone"))) return false;
        const auto R=ReadGrandMoment();std::vector<std::uint8_t> Before,After;
        if(!Check(ExportSession(Before),TEXT("the settled action remains saveable"))) return false;
        ReadGrandMoment();AdvancePresentation(.25f);ExportSession(After);
        if(!Check(Before==After && ReadGrandMoment().Kind==R.Kind,TEXT("reading and real-time display duration leave the saved game unchanged"))) return false;
        if(!Check(ImportSession(Before) && ReadGrandMoment().Kind==EGrandMoment::None,TEXT("a loaded milestone never replays its banner or sound"))) return false;
    }
    if(!Check(PrepareGrandMomentPreview(0),TEXT("the rescue moment can be reached again on a new actual deployment"))) return false;
    AdvancePresentation(6);
    if(!Check(ReadGrandMoment().Kind==EGrandMoment::None,TEXT("the moment expires without input or slowing the next command"))) return false;
    if(!Check(PrepareGrandMomentPreview(0),TEXT("a new real moment supports immediate manual dismissal"))) return false;
    std::vector<std::uint8_t> Before,After;ExportSession(Before);DismissGrandMoment();ExportSession(After);
    if(!Check(ReadGrandMoment().Kind==EGrandMoment::None && Before==After,TEXT("returning to command dismisses only the presentation"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_GRAND_MOMENT: PASS (actual rescue, rally, cavalry response and victory, no early reveal, immutable reads, save/load, restart and real-time expiration)"));return true;
}
