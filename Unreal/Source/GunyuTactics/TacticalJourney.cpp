#include "TacticalTrial.h"
#include "TacticalState.h"

using namespace frontline;

void ATacticalField::IssueJourney(ETacticalCommand Command)
{
    auto& J = State->Journey;
    switch (Command)
    {
    case ETacticalCommand::Dialogue:
        if (++State->DialogueIndex >= 3) J.Meet();
        break;
    case ETacticalCommand::PlanAmbush: State->PreparationChoice = 0; break;
    case ETacticalCommand::PlanSupplies: State->PreparationChoice = 1; break;
    case ETacticalCommand::PromiseRoad: J.SelectPromise(JourneyPromise::OpenRoad); break;
    case ETacticalCommand::PromiseRescue: J.SelectPromise(JourneyPromise::EscortRefugees); break;
    case ETacticalCommand::RouteRoad:J.SelectConvoyRoute(JourneyConvoyRoute::Road);break;
    case ETacticalCommand::RouteSouthernWood:J.SelectConvoyRoute(JourneyConvoyRoute::SouthernWood);break;
    case ETacticalCommand::TalkLiu: State->Interlocutor = 0; break;
    case ETacticalCommand::TalkGuan: State->Interlocutor = 1; break;
    case ETacticalCommand::TalkZhang: State->Interlocutor = 2; break;
    case ETacticalCommand::StoryReplyPreserve: J.ChooseStoryReply(JourneyPledge::Preserve);break;
    case ETacticalCommand::StoryReplyCoordinate: J.ChooseStoryReply(JourneyPledge::Coordinate);break;
    case ETacticalCommand::Reinforce: J.Reinforce(); break;
    case ETacticalCommand::Deploy:
        CheckpointJourney(true);
        State->BeforeBattle = std::make_unique<StrategistJourney>(J);
        J.Prepare(State->PreparationChoice, J.enemyPolicy);
        State->Orders.clear(); State->Selected = 0; State->Target = 3;
        State->Input = ETacticalInput::Select; State->bConfirmEndTurn = false;
        break;
    case ETacticalCommand::Retry:
        J = *State->BeforeBattle;
        State->Orders.clear(); State->Selected = 0; State->Target = 3;
        State->Input = ETacticalInput::Select; State->bConfirmEndTurn = false;
        State->Report = TEXT("戦い直前の軍議へ戻りました。前回の出陣費用と損耗を戻し、準備方針を見直せます。");
        Refresh(); return;
    default: return;
    }
    State->Report = JourneyText(J.message);
    Refresh();
    if(Command==ETacticalCommand::StoryReplyPreserve || Command==ETacticalCommand::StoryReplyCoordinate) CheckpointJourney(true);
}

void ATacticalField::BuildJourneyPresentation()
{
    const auto& J = State->Journey;
    auto& P = State->HUD.Journey;
    P = {};
    P.bVisible = J.stage != JourneyStage::Battle;
    if (!P.bVisible) return;
    P.bPreparation = J.stage == JourneyStage::Preparation;
    P.bDebrief = J.stage == JourneyStage::Debrief;
    P.bComplete = J.stage == JourneyStage::Complete;
    P.Context = TEXT("184年・黄巾の乱を背景とする序章。局地戦・会話・主人公の関与は創作です。");
    const auto& Camp = J.world.cities[0];
    P.Resources = FString::Printf(TEXT("野営地  兵 %d    兵糧 %d    軍資金 %d    信頼 %d"), Camp.troops, Camp.food, Camp.gold, J.trust);
    P.Relationships = FString::Printf(TEXT("共同任務の絆  劉備―関羽 %d / 劉備―張飛 %d / 関羽―張飛 %d"), J.world.Affinity(0, 1), J.world.Affinity(0, 2), J.world.Affinity(1, 2));
    if (J.stage == JourneyStage::Encounter)
    {
        const TCHAR* Names[] = {TEXT("劉備"), TEXT("関羽"), TEXT("張飛")};
        const TCHAR* Lines[] = {
            TEXT("劉備「泥にはまった車を、君が出したのかい。」\n主人公「押しただけだ。兵を動かせるわけじゃない。」\n劉備「でも、誰かの帰る道を作った。今度は私たちの力も使ってくれ。」"),
            TEXT("関羽「この兵を、駒と思うな。」\n主人公「分かってる。……いや、まだ顔も知らない。」\n関羽「では知れ。私も、その一人だ。どこへ行けと言う？」"),
            TEXT("張飛「軍師ってのは、俺より先に敵を見つけるんだろ？」\n主人公「お前が飛び出す前に、帰り道も見つける。」\n張飛「ははっ！ なら頼むぜ。戻れって声は、でかく出せよ！」")};
        const int Index = FMath::Clamp(State->DialogueIndex, 0, 2);
        P.Heading = TEXT("序章  ―  名もなき軍師"); P.Speaker = Names[Index]; P.Dialogue = Lines[Index]; P.Portrait = Index;
        P.PrimaryLabel = Index < 2 ? TEXT("話を聞く  /  Enter") : TEXT("軍議へ進む  /  Enter");
        P.Outcome = TEXT("あなたは道と荷を見てきた若者。まず任された一つの仕事で、英傑たちの信頼を得る。軍議で、どこまで守ると約束するかを選ぼう。");
    }
    else if (P.bPreparation)
    {
        P.Heading = TEXT("軍議  ―  出陣の段取り");
        P.Speaker = State->PreparationChoice == 0 ? TEXT("関羽") : TEXT("劉備");
        P.Portrait = State->PreparationChoice == 0 ? 1 : 0;
        P.Dialogue = State->PreparationChoice == 0
            ? TEXT("伏兵の準備は私が担おう。敵を森林の隣へ引き込めれば、混乱から突破へつなげられる。ただし敵の接近や斥候には注意が要る。")
            : TEXT("長く戦うなら兵糧を厚くしよう。士気も高まるが、野営地の蓄えは減る。伏兵を使うなら、戦場で関羽に準備を命じてほしい。");
        P.PrimaryLabel = TEXT("この段取りで出陣  /  Enter");
        P.Outcome = FString::Printf(TEXT("期限 %d指揮 / %s"), J.TurnBudget(), *JourneyText(J.Definition().briefing));
        P.bChoosePromise = J.mission == 0; P.bRescue = J.IsRescue();
        P.PromiseText = J.IsRescue() ? TEXT("季安の避難車を東の出口まで護送。護衛600と兵糧900を追加で確保する。先鋒を崩しても護送は終わらない。")
            : TEXT("先鋒を崩すか出口を確保し、退路を開く。避難車を最後まで護送する約束はしない。追加の護衛と兵糧は使わない。");
        if (J.IsRescue()) P.Outcome = TEXT("期限10指揮 / 季安の避難車を６区間先の東の出口へ。敵将を倒すより、車の進路と護衛の配置が目的になる。");
        if (J.mission > 0)
            P.Outcome = FString::Printf(TEXT("前戦の帰還兵 %d・損失 %dを引き継いでいます。\n%s"), J.survivors, J.casualties, *P.Outcome);
        for (int Choice = 0; Choice < 2; ++Choice)
        {
            const auto Cost = J.PreparationCosts(Choice);
            const int Deployed = FMath::Max(0, FMath::Min(4200, Camp.troops - 1000 - Cost.escort));
            FPreparationPresentation Plan;
            Plan.Title = Choice == 0 ? TEXT("伏兵の段取り") : TEXT("兵糧を厚くする");
            Plan.Description = Choice == 0 ? TEXT("士気90。森林で関羽の伏兵準備を試みる。敵の配置によっては露見する。") : TEXT("士気100。携行兵糧を増やす。伏兵は戦場で別途準備する。");
            Plan.Cost = FString::Printf(TEXT("出陣 %d%s\n兵糧 %d / 資金 %d"), Deployed, Cost.escort ? TEXT(" ＋護衛600") : TEXT(""), Cost.TotalFood(), Cost.gold);
            Plan.Remaining = FString::Printf(TEXT("残兵 %d / 糧 %d\n残資金 %d"), Camp.troops - Deployed - Cost.escort, Camp.food - Cost.TotalFood(), Camp.gold - Cost.gold);
            if (Cost.cargo) Plan.Description += TEXT(" 糧車の積荷2,000と護衛用の糧900を含む。");
            Plan.bSelected = Choice == State->PreparationChoice; Plan.bAffordable = J.CanPrepare(Choice);
            if (!Plan.bAffordable) Plan.Remaining = TEXT("資源不足。必要な兵・兵糧・軍資金を確認してください。");
            P.Plans.Add(MoveTemp(Plan));
        }
    }
    else if (P.bDebrief)
    {
        P.Heading = J.won ? TEXT("戦後  ―  采配の帰結") : J.withdrawn ? TEXT("撤収  ―  残した兵と、残った約束") : TEXT("敗北  ―  次に変える一手");
        P.Speaker = TEXT("劉備"); P.Portrait = 0;
        P.PrimaryLabel = J.won ? TEXT("戦果を引き継いで次へ  /  Enter") : TEXT("軍議に戻って再挑戦  /  Enter");
        if (!J.won) P.Dialogue = TEXT("ここで責め合っても、退路は開かれない。敵を追うべき時と、目的を守るべき時を見直そう。次は、何を変えて臨む？");
        else if (J.casualties * 4 > J.deployed + J.convoyEscort)
            P.Dialogue = TEXT("道はつながった。だが、戻らなかった者も多い。この勝利の代償を、次の出陣に生かしてほしい。残った兵と糧を頼みに、もう一度策を練ろう。");
        else if (J.bestChain >= 3) P.Dialogue = TEXT("君の一策で、三人の働きが一つにつながった。民だけでなく、兵も多く帰ってきた。その采配を、次の任務でも頼りにしている。");
        else P.Dialogue = TEXT("大切なのは、敵を倒し尽くすことではなく、守るべきものを守ることだ。君が開いた道を、次の任務へつなごう。");
        P.Outcome = FString::Printf(TEXT("%s\n帰還兵 %d / 損失 %d / 最大連鎖 %d / 信頼 %d"), *JourneyText(J.message), J.survivors, J.casualties, J.bestChain, J.trust);
        if (J.Definition().kind == JourneyObjective::Escort) P.Outcome += FString::Printf(TEXT("\n持ち帰った積荷 %d"), J.recoveredCargo);
        P.Outcome += J.won ? TEXT("\n生存兵と残りの兵糧は野営地へ精算済み。共に生還した武将の絆が深まる。") : TEXT("\n再挑戦では今回の出陣直前の状態へ戻り、準備を選び直せます。");
    }
    else if (P.bComplete)
    {
        P.Heading = TEXT("序章  ―  三つの策を越えて"); P.Speaker = TEXT("劉備"); P.Portrait = 0;
        P.Dialogue = TEXT("退路を開き、糧を届け、烽火を守り抜いた。君はもう、我らとともに戦った軍師だ。乱世の道は続く。これまでの采配を、忘れずにいてほしい。");
        P.Outcome = FString::Printf(TEXT("序章の３任務を達成。信頼 %d\n仕官先の選択と、この先の歴史戦役は今後の実装です。"), J.trust);
        P.PrimaryLabel = TEXT("序章を最初から");
    }
    for (int Index = 0; Index < 3; ++Index)
        if (J.history[Index].completed)
            P.History += FString::Printf(TEXT("第%d戦: %s / 帰還 %d / 損失 %d  "), Index + 1,
                J.history[Index].rescued ? TEXT("季安の避難車を護送") : J.history[Index].won ? TEXT("任務達成") : J.history[Index].withdrawn ? TEXT("任務中止・撤収") : TEXT("未達"),
                J.history[Index].survivors, J.history[Index].casualties);
    BuildOfficerConversation();
    BuildOpeningPresentation();
    BuildStoryPresentation();
    BuildEpisodePresentation();
}
