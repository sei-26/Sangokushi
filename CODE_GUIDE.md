# コードを読むための地図

作戦の費用差・不足・実際の士気・武将の助言は `TacticalPreparationRead.cpp`。改善の経緯と次の候補は [IMPROVEMENT_LOG.md](IMPROVEMENT_LOG.md)。

会話から作戦書・出陣への接続は `TacticalNovelPlan.cpp` / `TacticalNovelPlanInterface.cpp`。[仕様](NOVEL_OPERATION_FLOW.md)。

会話のノベル表示と共通の意匠は `Unreal/Source/GunyuTactics/TacticalNovel*.cpp` と `TacticalStyle.h`。[仕様と原画の入口](NOVEL_VISUAL_UPGRADE.md)。

大河ドラマを意識した序章の本幕は `NewGame/JourneyEpisode*.cpp`、全画面の閲覧と入力は `Unreal/Source/GunyuTactics/TacticalEpisode*.cpp`。担当と実装範囲は [EPISODE_DRAMA_UPGRADE.md](EPISODE_DRAMA_UPGRADE.md)。

2026-10-21以降の開発先はUnreal Engine。エディタで開く入口は `Unreal/GunyuTactics.uproject`。`Unreal/Source/GunyuTactics/` の `TacticalOrders.cpp` が既存の `StrategistJourney` へ命令を渡し、`TacticalField.cpp` が仮の3D地形・部隊を描く。`TacticalPresentation.cpp` は観測情報と成立予測を表示へ変換、`TacticalInterface.cpp` は武将カード・命令・連鎖編集、`TacticalController.cpp` はカメラ・入力、`TacticalSmoke.cpp` は結合検証を担当。素材とライトはUEエディタから編集できる。詳しくは [Unreal/README.md](Unreal/README.md)。以下は共通ルールと旧Siv3D画面の案内。

まず Main.cpp → NewGame/CampaignScene.cpp → NewGame/CampaignTurn.cpp の順に読むと、起動・画面・ゲームルールのつながりが分かる。

UEの行動演出は `TacticalPlayback.cpp`（実結果を順に表示）、`TacticalUnits.cpp`（隊列・旗・糧車・衝撃・音）、`TacticalPlaybackTests.cpp`（結果の不変性とスキップ）に分けている。武将肖像は旧版と同じ `PortraitCatalog.hpp` の対応を利用する。ゲームのフルスクリーン起動は `Unreal/Play.cmd`、編集は `.uproject` が入口。

UEの会話・軍議・戦後は `TacticalJourney.cpp`、その画面は `TacticalJourneyInterface.cpp`。`TacticalJourneyTests.cpp` は同じ公開命令で出会い・募兵・敗北と再挑戦・３戦・終幕を通す。準備に必要な資源は共通ルールの `StrategistJourney::PreparationCosts` と `CanPrepare` から読み、表示だけの費用と実際の支払いを別々に定義しない。

`TacticalConversations.cpp` は作戦・護送・損耗・撤収に応じた３人の反応、`TacticalPromiseTests.cpp` は約束を選んだ護送と撤収・再挑戦・次戦の人物の検証。共通ルールの `StrategistJourneyObjectives.cpp` が `SelectPromise` と `Withdraw` を処理し、実績は `StrategistJourney::history` に記録する。人物との会話を繰り返すだけで実績や絆を加算しない。参照原稿と複数の課題を一緒に解決する狙いは [STORY_STRATEGY_IMPROVEMENTS.md](STORY_STRATEGY_IMPROVEMENTS.md)。

ヘッダー（.hpp）は主に「持っているデータと、呼べる関数」。実際の処理は同名や役割名の.cppにある。同じクラスの関数を複数の.cppに分けているため、別ファイルでも同じゲーム状態を扱う。ファイルを分けるために別のシステムへ作り直したわけではない。

戦法の予約不可理由と対象更新は `Unreal/Source/GunyuTactics/TacticalTacticAvailability.cpp`、その回帰確認は `TacticalTacticAvailabilityTests.cpp`。[仕様](TACTIC_AVAILABILITY_FIX.md)。

戦い方と主人公への評価は `NewGame/JourneyBattleStory.cpp`、台詞は `JourneyDrama.cpp` / `JourneyCouncilDrama.cpp`。UEの掛け合い表示は `TacticalStoryPresentation.cpp` / `TacticalDialogueInterface.cpp`。[仕様](BATTLE_STORY_UPGRADE.md)。

## 最初に読む5ファイル

新しい本編序章の入口は `CampaignSceneInput.cpp` → `StrategistJourneyScene.cpp` → `StrategistJourney.cpp`。従来の全国戦略は以下の５ファイルを参照する。

| ファイル | 分かること |
|---|---|
| Main.cpp | 起動設定、フォント登録、更新と描画を毎フレーム呼ぶ場所 |
| NewGame/CampaignScene.cpp | 自由戦略の画面全体と英雄譚への切替 |
| NewGame/CampaignTypes.hpp | 都市・武将・部隊・任務が持つ数値 |
| NewGame/Campaign.hpp | 自由戦略で呼べる命令と、ゲームが保持する状態 |
| NewGame/CampaignTurn.cpp | 1日を進めるとき、何をどの順で処理するか |

## 本編序章と連鎖戦闘

| ファイル | 担当 |
|---|---|
| StrategistJourney.hpp / .cpp | 主人公の進行、出会い・軍議・準備・３戦・戦後、同じCampaignの資源と信頼 |
| StrategistJourneyAI.cpp | 観測に基づく敵の移動、伏兵と通常交戦、指揮区切り |
| JourneyMission.hpp | 突破・護送・防衛の任務データ、期限、配置と目的地 |
| StrategistJourneyObjectives.cpp | 固守、糧車の進行、二拠点の維持、目的ごとの勝敗 |
| TacticChain.hpp / .cpp | 戦法の状態、単独発動、成功順の100/125/150％、途中停止、観測コピーでの予測 |
| TacticCatalog.cpp | 得意・共通・固有戦法の担当、説明、士気費用 |
| TacticChainActions.cpp | 各戦法の成立条件、効果、支払い、１戦１回、段階ごとの変化記録 |
| JourneyEnemyPlan.hpp / .cpp | 観測盤面からの敵の進軍・護送車への接近・固守・混乱、推定攻撃圏 |
| StrategistJourneyScene.hpp / .cpp | 会話と軍議の操作、移動と戦法の予約、ズーム、カットインの順次再生 |
| StrategistJourneySceneDraw.cpp | 会話、野営地、HEX戦場、予約の成立見込み、戦後の表示 |
| tests/TacticChainTests.cpp | 成立と対抗、情報漏れ、資源と人物関係の引き継ぎ |
| tests/JourneyMissionTests.cpp / JourneyPlaythroughTests.cpp | 任務ごとの進行と精算、通常操作による３戦完遂と異なる防衛手順 |

仕様・操作・未実装範囲は [CHAIN_BATTLE.md](CHAIN_BATTLE.md)。連鎖のルールはSiv3Dに依存しないため、画面を立ち上げず検証できる。全国戦略の保存形式へ局地戦を混ぜず、今の序章はメモリ内で進行を保持する。

## 自由戦略のルール

軍師戦役の仕様と実装は [STRATEGIST_SPEC.md](STRATEGIST_SPEC.md) が入口。`StrategistBattle.cpp` が戦場と目的、`StrategistWarScene.cpp` が軍議・命令・時間の入力、`StrategistWarSceneDraw.cpp` が表示を担当する。補給・戦闘・移動は既存の `Campaign::AdvanceBattleDay()` を利用する。

`BattleIntelligence.cpp` は各軍が見た情報、`StrategistBattleIntelligence.cpp` はその情報を使う敵判断・経路・報告、`StrategistBattleOperations.cpp` は条件付き作戦の評価と合図を扱う。作戦編集の入力と画面は `StrategistWarSceneOperations.cpp`。現実の戦場の状態はCampaign、知っている情報はBattleIntelligenceへ分け、表示に未発見の敵の実状態を渡さない。

| ファイル | 担当 |
|---|---|
| Campaign.cpp | 初期配置、名称、座標、記録、旧形式の地図の初期化 |
| CampaignEconomy.cpp | 内政、募兵、事業の完了、月末収入、勝敗判定 |
| CampaignLogistics.cpp | 兵糧輸送の出発条件、積荷の支払い、到着日数の見積もり |
| CampaignAssignments.cpp | 武将の都市間異動、移動日数、到着と陥落時の中止 |
| CampaignOfficers.cpp | 武将の空き状況、親密度、役割による部隊連携 |
| CampaignDiplomacy.cpp | 停戦、使者、謀略、任務の結果 |
| CampaignOrders.cpp | 出陣、進路探索、撤退、移動、帰還 |
| CampaignSupply.cpp | 補給できる領地の探索、兵糧消費、糧切れ |
| CampaignCombat.cpp | 戦法を予約できる条件、包囲する方向数 |
| CampaignDailyCombat.cpp | 戦法の発動、野戦・攻城の損害、都市の陥落 |
| CampaignAI.cpp | AIの命令期の判断 |
| CampaignTurn.cpp | 上記を1日単位で順に呼ぶ入口 |
| CampaignSave.cpp / CampaignLoad.cpp | JSONへの保存 / 読込と不正な値の検証 |
| WorldLayout.hpp / MapCamera.hpp | 地形・都市の配置 / 拡縮と画面座標の変換 |

この表のファイルはすべて NewGame/ 内にある。

## 自由戦略の画面

| ファイル | 担当 |
|---|---|
| CampaignScene.cpp | 全体の描画を呼び出す |
| CampaignSceneInput.cpp | スタート、保存、時間進行、選択、出陣などの入力 |
| CampaignSceneCamera.cpp | 地図の拡縮、移動、マウス位置からマスを選ぶ |
| CampaignSceneLayout.cpp | ボタンを置く位置 |
| CampaignSceneMap.cpp | 地形、都市、部隊、進路、ミニマップの描画 |
| CampaignScenePanel.cpp | 左側の都市・部隊の情報と命令ボタン |
| CampaignSceneMenu.cpp | スタート画面 |
| CampaignSceneLogistics.cpp | 輸送先・積荷・到着目安と輸送命令 |
| CampaignSceneAssignments.cpp | 武将の送り先・到着日・異動中の表示 |
| CampaignSceneCouncil.cpp | 武将評定・親密度・任務と、群雄の記録 |
| CampaignScenePreview.cpp | 開発用の画面プレビュー |
| CampaignUI.cpp | 共通のボタンと勢力の色 |

たとえば「兵糧の減り方」を変えるなら CampaignSupply.cpp。「兵糧の見せ方」を変えるなら CampaignScenePanel.cpp を見る。

## 英雄譚のルール

| ファイル | 担当 |
|---|---|
| HeroStory.hpp / HeroStory.cpp | 英雄譚の状態、座標や章の基本情報、記録 |
| HeroStoryProgression.cpp | 会話の選択、任務の配置、内政・訪問、勝利・失敗・再挑戦 |
| HeroStoryRelationships.cpp | 親密度、準備担当、役割連携 |
| HeroStoryBattle.cpp | 移動、通常攻撃、敵と民の手番、戦闘終了条件 |
| HeroStorySkills.cpp | 英雄の戦法、闘志、絆の号令、戦場での決断 |
| StoryBook.hpp | 章のタイトル、会話、選択肢、史話 |
| StorySave.cpp / StoryLoad.cpp | 英雄譚の保存 / 読込 |
| OfficerCore.hpp / OfficerTraits.hpp | 両モード共通の役割・連携 / 自由戦略の個性・戦法の定義 |

## 英雄譚の画面と演出

| ファイル | 担当 |
|---|---|
| HeroStoryScene.cpp | 開く・戻る・保存、ボタンの位置などの共通処理 |
| HeroStorySceneInput.cpp | 選択、移動、攻撃、戦法などの入力 |
| HeroStorySceneDraw.cpp | 現在の状態に応じ、以下の画面を呼び分ける |
| HeroStorySceneMenu.cpp | 英雄譚の入口 |
| HeroStorySceneDialogue.cpp | 会話、準備、結末、失敗時の表示 |
| HeroStorySceneCivil.cpp | 内政と訪問の画面 |
| HeroStorySceneBattle.cpp | 戦場の命令と情報、戦場での選択イベント |
| HeroStorySceneNotes.cpp | 史話、人物、選択の記録、仲間同士の関係 |
| HeroStoryScenePreview.cpp | 開発用プレビュー |
| StoryPresentation.cpp | 損害表示と武将の活躍演出を始める |
| StoryPresentationDraw.cpp | 背景・人物・戦場・演出の描画 |
| StoryPresentationAudio.cpp | 音の生成、再生、停止、音量切替 |

## 更新と描画の違い

入力・シミュレーションの関数がゲーム状態を変える。draw系の関数は、その状態を画面に表示する。戦闘ルールを直すときは Battle / Combat、画面を直すときは Scene / Presentation を探す。

1日の処理順は CampaignTurn.cpp が決める。同日の損害を一括計算してから移動する順番は、戦闘結果に関わるため維持する。

## 編集後の確認

Sangokushi.slnには分割した.cppを登録済み。Visual Studioのソリューションエクスプローラーでは「Source Files → 自由戦略／英雄譚 → ルール・画面・保存・演出」に分類している。対応するヘッダーも同じ分類で探せる。NewGame/でファイル名がCampaignで始まるものが自由戦略、HeroStoryまたはStoryで始まるものが英雄譚。

tests/run.ps1はシミュレーションの.cppをライブラリへまとめ、実際の実装を使って戦闘・補給・内政・物語・保存を検証する。新しい.cppを増やす場合は、Visual Studioのビルド対象と、このスクリプトの対象リストへ追加する。

.clang-formatと.editorconfigでタブ幅4・UTF-8 BOM・改行などの書式を揃えている。セーブ形式、ゲーム内の文章、数値、ルールは今回の整理で変更していない。

ファイル分割時にはDebug・Releaseビルド、既存テスト、5画面の起動で確認済み。分割前の実装と同条件で自由戦略180日・英雄譚全6章を進め、途中の保存データが一致することも確認した。

新しい戦闘の判断は CampaignBattleRules.cpp（構え・射程・射線）、HeroStoryObjectives.cpp（章の約束・拠点・守備）を入口に読む。両方の射線判定は BattleCore.hpp の ClearRay を使う。

CampaignReturn.cpp は指定した味方都市への帰還・即時入城、CampaignSceneArmyOrders.cpp は帰還先選択と帰還ボタンの入力を扱う。

## 敵AIを読む順番

- CampaignAI.cpp：命令の回数管理と実行。10日ごとに城数に応じた命令を回復・同日の再実行防止。
- CampaignAIPlanning.cpp：危機、兵糧、武将、到達できる目標を評価し、優先順位つきの候補を作る。
- CampaignAIArmies.cpp：日々の撤退、近くの城の救援、輸送隊護衛、攻撃対象、戦法の発動判断。
- HeroStoryAI.cpp：地形・占有・射線・損害・守備から、敵の攻撃対象と移動先を選ぶ。
- tests/AITests.cpp：状況別の判断と、3勢力の長期シミュレーション。

命令を1つ実行するたびに候補を作り直す。武将や都市資源が使用済みになるため、実行前の候補を続けて流用しない。判断の理由は群雄の記録へ残す。

- `NewGame/CampaignAIOperations.cpp`：敵軍の攻城隊と支援隊の足並み、集結待機の期限、攻勢中止。日次移動の停止情報を返し、戦闘停止とは別に判断する。

- `NewGame/CampaignBattlefield.cpp`：敵部隊の補給封鎖範囲、味方護衛と守備兵による保護、方向数に基づく包囲圧力。補給・輸送経路・戦闘・AI・画面が同じ評価関数を使う。

- `NewGame/CampaignTerrainDraw.cpp`：軍略地図の地面・山並み・森林・水面・橋。地面と水面は地図テクスチャにキャッシュし、山・森の素材は `CampaignSceneTerrain.cpp` から直接描画する。
- `NewGame/CampaignMarkerDraw.cpp`：城郭、部隊の軍旗と兵力バー、既存の英雄肖像と未収録武将の紋章。
- `NewGame/CampaignVisuals.hpp`：本編の地図・部隊欄が使う描画関数。ゲームのルールやセーブには依存する状態を追加しない。

- `NewGame/HexGrid.hpp`：本編のHEX座標、六方向隣接、距離、射線。旧セーブは `Campaign::hexMap=false` で従来の四角地図を使う。
- `SANGOKUSHI14_REBUILD.md`：全面刷新の実装状況と未実装の主要項目。旧コアより現在のユーザー指定を優先する。

- `NewGame/CampaignRegions.cpp`：府・地域の生成、占領、支配拡大、都市への追加収入。
- `NewGame/CampaignSceneRegions.cpp`：府の地図表示、選択区画、地域情報パネル。

- `NewGame/CampaignLandscapeDraw.cpp`：六角マスの共有辺、川岸・海岸、畑、地図の照明、交戦エフェクト。地形の描画を入力・進行処理と分ける。

- `NewGame/CampaignMapArt.hpp/.cpp`：MapArt画像の読み込み、山・森・都市・府の画像配置、地形マスへの材質UV、画像欠落時の代替。画像本体と生成プロンプトは `App/MapArt/`。

武将の追加データは `NewGame/CampaignRoster.cpp`。既存ID 0〜23を維持し、24〜71を追加します。兵科・地形・戦法の補正は `CampaignBattleRules.cpp` の `OfficerAttackPercent` / `OfficerDamagePercent` に集約し、戦闘計算と情報パネルで共有します。人望の士気支援と不屈の士気軽減は `CampaignDailyCombat.cpp`。

武将肖像の対応表は `PortraitCatalog.hpp`、描画は `CampaignPortraitDraw.cpp`。戦法の成功イベントは `CampaignDailyCombat.cpp` が `tacticEvents` に発行し、次の日の処理でクリアします。演出は `CampaignPresentation.cpp`（順番と時間）、`CampaignPresentationDraw.cpp`（カットイン・戦場効果）、`CampaignPresentationAudio.cpp`（外部素材なしの効果音合成）に分割。演出はゲーム状態を書き換えず、UIが日数進行を待ちます。

情報メニュー：`CampaignInformation.hpp` が閲覧用の一覧、絞り込み、並べ替え、任務状況、親密度、勢力集計を提供します。64bitで資源を集計し、ゲーム状態は変更しません。UIは `CampaignSceneInformation.cpp`（入力・選択・地図への移動）、`CampaignSceneInformationDraw.cpp`（一覧）、`CampaignSceneInformationDetail.cpp`（詳細）、`CampaignInfoLayout.hpp`（共通の座標）に分割しています。`tests/InformationTests.cpp` で一覧と集計を検証します。

## 城の特色と太守

城情報の「都市運営・太守の任命」から、待機中の所属武将を太守へ任命できる。任命は金100・命令1、解任は命令1。太守は他の任務に使用できず、置き換えると前任者は通常任務へ戻る。命令期間のみ実行でき、閲覧中は日数を停止する。

新マップの30都市は穀倉（都市兵糧収入+40%）、商都（金収入+40%）、軍都（募兵+500）、要塞（守備兵被害-15%）、兵站拠点（実効兵站+20）の5種類。分類はゲーム用の設定。旧レイアウトは一般都市。太守の政治で収入、統率で防衛、兵站適性で実効兵站、内政担当との親密度で開発量が変わる。携行糧は都市の実在する蓄えから差し引く。府収入には都市収入補正を重ねない。陥落時は太守を解任し、既存の武将避難処理を使用する。

`CityIdentity.hpp` に特色の定義、`CampaignGovernance.cpp` に任命・効果計算、`CampaignSceneGovernance.cpp` に候補比較と任命画面を分離。敵AIも命令予算を使い、指揮官を残して政治に優れた候補を任命。セーブ形式13は太守を保存し、形式12以前は空席として読み込む。`GovernanceTests.cpp` と保存テストで兼務禁止・資源保存・収入・補給・攻城・避難・旧データ互換性を確認する。

## 英雄譚の映像・会話演出

6章は `App/StoryArt/chapter-scenes-v1.png` の3列2行アトラスから別々の情景を表示する。オリジナルの物語イラストであり、史料の図版ではない。生成方法と全文プロンプトは `App/StoryArt/CHAPTER_ART_PROMPT.md` に記載。章開幕は全画面で3.6秒表示、クリック・Space・Enterで省略可能。

各章の導入・準備・勝利後に2つずつ創作会話を加え（36場面）、既存の決断会話を3ページ目に表示する。クリックまたはSpace・Enterは、表示途中なら全文表示、全文表示後なら次の会話へ進む。2つの決断は3ページ目で選択する。会話の表示位置・途中ページは保存対象外なので再開時は当該段階の会話から読み直せる。既存の決断・資源・任務・成果の保存形式は維持する。

`StoryDrama.hpp` は創作会話、`HeroStorySceneCinema.cpp` は章開幕・会話の送り、`HeroStorySceneDialogue.cpp` は映画調の会話・成果を振り返るエンディングを担当。歴史の背景は既存の史話で確認できる。ストーリーの拠点確保・救出・勝利・敗北・第一部完了でカットインを表示する。戦法演出はSpace・Enterで省略。ストーリー武将の顔は本編と同じ武将アトラスを使い、NPCの会話枠は情景イラストを使う。

本編の戦法はカットイン・速度線・衝撃波・火花を強化。戦法効果や日数の計算は変更せず、演出中は既存の進行停止を維持する。プレビュー名は `story-intro`、`story-cinema`、`story-spectacle`、既存の `story-prepare`、`story-ending`、`tactic-cutin`。

## 戦場の動きと戦況に応じた武将の反応

Gキーで追加の戦場演出を切り替える。交戦の火花・砂煙・弓兵の矢の軌跡、攻城中の火煙と投射物、進軍の土煙を表示する。交戦時は合成した太鼓・金属音の環境音を流す。Mキーの効果音設定は追加音にも適用し、情報画面や戦法カットイン中は環境音を止める。

城の所有者変更、実際の敗走、補給路の途絶・復旧、大きな損耗から、顔付きの戦況速報・創作の台詞を表示。普通の反応は武将ごとに10日間抑制し、待機報告は最大6件。城の速報を優先する。演出・台詞は保存しない。帰還・入城を敗走と誤認せず、占領後の守備兵移管を損耗の台詞として扱わない。戦争の計算には介入しない。

`BattlefieldMoments.hpp` は小さな観測データとイベント判定、`CampaignSpectacle.cpp` は時間・台詞・音、`CampaignSpectacleDraw.cpp` は地図効果と速報を担当。新しい観測の回帰テストは `BattlefieldMomentsTests.cpp`。プレビューは `battle-immersion` と `city-drama`。ストーリーの長坂に雨、赤壁に火の粉、桃園に花びらを追加し、会話・章開幕の前景に表示する。

## 攻城の状況表示と戦況速報からの移動

攻城配置中の都市は、守備兵・攻撃側の部隊数と兵力・攻城隊数・最大包囲方向数・攻城補正・補給途絶を地図上に表示する。城の周囲の線は、敵のいる方向を示す。同じ勢力の包囲方向数から最大の補正を計算し、別々の勢力の包囲数を足さない。戦闘部隊は隣接してその城を目標としている場合だけ攻城配置に数え、輸送隊・帰還中・停戦相手は除く。敵部隊との交戦が優先される場合もあるため、与ダメージ予測ではなく現在の配置として表示する。

戦況速報をクリックすると、その現場へカメラを移し城や部隊を選択する。×は次の速報へ進める。マウスを重ねている間は速報の表示時間を保持する（日数進行は変更しない）。速報の上で地図のクリック・ズーム・ドラッグを誤って操作しないよう入力を区別する。`SiegeStatus.hpp` は集計、`CampaignSceneSieges.cpp` は表示を担当し、`BattlefieldMomentsTests.cpp` で同方向の重複・輸送・帰還・停戦と実際の包囲補正を確認する。プレビュー名は `siege-overview`。

## 部隊連携の確認

部隊を選択してLキー、または右の部隊パネルの「連携を見る L」をクリックすると部隊連携画面を開く。閲覧中は日数進行を停止し、閉じると元の進行状態に戻る。同じ勢力の出陣部隊の名前をクリックすると、その部隊が受ける効果へ切り替える。「地図で見る」は選択部隊の位置へ移動する。

地図では選択部隊が受けている連携を色付きの線と輪で表示する。橙は攻撃、青は防御、緑は糧消費、金は士気。複数の効果がある場合は代表色。詳細画面は親密度・距離・現在の補正と、発動しない理由（距離、親密、帰還、士気、兵糧、輸送隊、役割）を表示する。同じ効果の補正は最大値のみ。離れた部隊の「見込」は役割と親密度に基づく潜在効果で、移動や他の条件を満たす必要がある。

`Campaign::FormationLink` で1対1の連携判定を共有し、戦闘の合計 `Formation` と画面の集計が同じ結果を使う。既存の連携計算の条件・補正量は維持する。`FormationStatus.hpp` は仲間と発動条件、`CampaignSceneFormation.cpp` は画面・地図の線を担当。`FormationTests.cpp` で総効果・同役割・距離・士気・兵糧・帰還・輸送・親密の一致を確認する。プレビューは `formation-info` と `formation-map`。

## 共同訓練で関係を育てる

城情報の「都市運営・共同訓練」→都市運営画面の「共同訓練 / 親密度」から開始。同じ城に所属する待機中の武将2人が必要。金300・糧500・命令1を支払い、10日後に親密度+12（上限100）。都市ごとに同時に1組。親密段階40・60・80を超えると連携段階が上がる。戦場での効果は相手の役割と配置・士気・補給にも依存し、親密度は共同内政・使者の協力にも使う。

訓練中は2人とも出陣・内政・使者・異動・太守の任命に使用できない。中止は追加命令を使わず、返金・親密度の成果なし。城が陥落すると即座に中断し、既存の避難処理へ戻す。敵AIは30〜59日の安全な後方で、余剰人材がいる時に親密40未満の組を育てる。

`CampaignTraining.cpp` に開始・中止・日次進行・中断を分離し、`CampaignSceneTraining.cpp` が2人の選択と成果予告・残り日数を表示。形式14で訓練を保存し、形式13以前は訓練なしで読み込む。情報メニューも共同訓練の担当と残り日数を表示する。`TrainingTests.cpp` と保存テストでコスト・兼務禁止・10日完了・上限・中止・陥落・継続保存を確認。プレビューは `joint-training`、`training-progress`、`training-compact`。

## 音楽
`MusicPlayer.hpp/.cpp` は両モードのBGMストリーミング、クロスフェード、カットイン時の減音、B/Nキーと画面ボタンを担当します。`MusicCues.hpp` はゲーム状態から曲を選ぶ純粋な関数で、`tests/MusicCueTests.cpp` が平時・交戦・敗北・勝利・ストーリー各段階の選曲を検証します。既存の効果音プレイヤーとは分離しています。

音源は `App/Music`、作曲・合成スクリプトは `tools/build_story_music.py`（Python + NumPy）。同梱READMEに曲と生成方法を記載。`StoryBattleSpectacle.cpp` はストーリーの戦法後半のマップ上の演出、`HeroStorySceneCinema.cpp` は章開幕と仲間の肖像を担当します。

### 英雄譚の戦場描画
`StoryBattleTerrain.cpp` は地形・拠点・船、`StoryBattleUnits.cpp` は隊列・旗・肖像と移動・攻撃の軌跡、`StoryBattleSpectacle.cpp` は戦法・火攻めの効果を担当します。`StoryPresentation::Capture` は前後の実際の状態から移動と損害を抽出し、操作側が渡す武将番号を攻撃の起点に使います。敵手番全体の集計から攻撃者を推測せず、対象への被弾演出で表示します。状態とセーブは変更しません。

`HeroStorySceneBattle.cpp` はカーソル下の敵の予定を可視化し、通常攻撃の結果を状態コピーで予測します。プレビュー `story-strike` / `story-firestorm` は弓と火攻めの効果の確認に使えます。

`CampaignSceneTerritory.cpp` は領土の凡例とその画面領域を担当。勢力の共通色は `CampaignUI.cpp`、領土の塗りと境界はキャッシュされる `CampaignSceneMap.cpp` にあります。補給表示時は領土の塗りを抑えて補給色も読み取れるようにしています。

## 内政画面と命令の規模
`CampaignSceneDomestic.cpp` は仕事選択・担当武将の比較・開始可否・進捗表示を担当します。`CampaignSceneCouncil.cpp` の画面7として表示し、太守・共同訓練画面にもつながります。仕事の計算と決済は既存の `Develop` / `CityWorkGain` / `CancelWork` を使います。

`Campaign::CommandLimit(faction)` は全シナリオの都市所有者から命令容量を算出。`commandBudget` は現在の命令期間の容量で、`FinishDay` の10日境界で容量と残数を更新します。AIは期間の初回計画で同じ容量を設定し、同日再計画やロードで予算を増やしません。形式15は `commandBudget` を保存し、旧形式では残数を維持します。`tests/CommandBudgetTests.cpp` は0～30城・命令期間更新・城喪失・AI再計画・旧盤面を検証し、`SaveDataTests.cpp` は予算の往復・不正残数・旧形式を検証します。

## 都市発展と国策
`CityGrowth.hpp` は規模名と必要能力、`PolicyDefinition.hpp` は国策名と負担を定義。`CampaignDevelopment.cpp` は日次昇格・募集上限・政策枠・登用と撤回・適性と在席の判定を担当します。`CampaignGovernance.cpp` / `CampaignEconomy.cpp` が収入・兵站・募兵・防衛・内政開発・月末治安に効果を反映します。国策を担当していても `Busy` に追加せず、内政・太守と兼務可。出陣・移動・外交中は効果を休止します。

`CampaignScenePolicies.cpp` は国策の選択・候補・効果・負担、`CampaignSceneDomestic.cpp` は規模と次の条件を表示します。形式16で都市level・武将policy・9人分のpolicyCouncilを保存。読み込みは範囲・人数・所属勢力・同一武将/政策の重複を検証します。`DevelopmentTests.cpp` と保存テストが条件・永続成長・休止・費用・個性/得意・旧形式を検証。プレビューは `city-growth`、`national-policy`、`policy-compact`、`metropolis-map`。

### 自然地形の表示
`CampaignMapArt.cpp` は世界座標の連続UV・気候の色調・新しい山/森林の素材を担当。`CampaignSceneTerrain.cpp` が表示範囲の山・森と隣接数を取得し、元画像の縦横比を維持して画面へ直接描画します。主稜線と支稜線の大きさは隣接数で変わります。領土境界は素材の上に描きます。城・府は別の既存素材を使用。地面・水面の地図キャッシュは48px/マスで、領土表示フラグの変更時にも再描画します。`CampaignSceneTerritory.cpp` がTキーの表示モードのボタン、`CampaignSceneInput.cpp` が入力を処理します。初期モードは領土色、強い勢力色は領土色モードで使用します。

新素材は `App/MapArt/terrain-relief-v2.png`、生成プロンプトは `App/MapArt/TERRAIN_V2_PROMPT.md`。画像欠落時は旧素材または手続き描画へ戻ります。`metropolis-map`、`world`、`territory-world` で拡大・自然地形・領土色を確認できます。


`CampaignSceneTerrain.cpp::drawTerrainTransitions` は森林・山の近傍分布を共有頂点で補間し、地面キャッシュへ林縁・山麓の色を描きます。`drawTerrainRelief` は画面内の素材・森林の隙間と林縁・内陸の勢力境界を描画。`CampaignLandscapeDraw.cpp::Shore` は共有端点を固定した水際の曲線と砂浜・浅瀬を担当します。海との境界は太い勢力線を省略し、地形ルールには影響しません。


`CampaignSceneScenery.cpp` は都市周辺と水面の景観を担当。`drawCityLandscape` は平野に小道・農地・集落を配置し、農業値と都市規模から農地の密度と家の数を決めます。地図更新時にキャッシュへ描画。`drawWaterSurface` は表示範囲の川・海だけに時間依存の光の筋を描き、橋には重ねません。遠景では筋を減らし、1マス12px未満では省略します。小道は景観であり、経路・移動速度・補給の計算を変更しません。


## 本編UIの構成
三國志14の公式メイン画面の役割分担を参考に、地図を中心に再配置。市販ゲームのUI画像は使わず、描画コードと既存の独自素材で構成しています。

- `CampaignSceneHUD.cpp`：上部の君主・経過年月旬・命令書・勢力資源・進行操作、下部の勢力コマンド、作戦/報告欄、設定ポップアップ。年表示は開始からの経過年で史実年ではありません。
- `CampaignScenePanel.cpp` / `CampaignSceneRegions.cpp` / `CampaignSceneLogistics.cpp`：左側の選択対象と命令。都市の内政値はゲージで表示。
- `CampaignSceneLayout.cpp`：コマンドの共通座標。保存・読込・勢力選択のボタンは設定を開いた時だけ有効。
- `CampaignSceneCamera.cpp`：右下ミニマップ、地図UIの当たり判定、全図/拡縮/移動。
- `CampaignUI.cpp::Panel` / `DrawButton`：紺のグラデーションと銀・金の罫線を使う共通意匠。情報・内政・施政・任免と勢力選択にも適用。

設定を開くと進行を停止し、閉じると元の進行に戻ります。Escで選択対象を解除。補給は下部のボタン/Tab、領土色は右下のTボタン。初期表示は領土色（地形を残す薄い塗り）。プレビュー `ui-settings` と `ui-compact` を追加しています。


## 中国全土の全国図
`CampaignSceneAtlas.cpp` は全国図の表示・都市紋章・地名の重なり回避・部隊の実際の進路を担当。`ChinaAtlas.hpp` は既存30都市の表示用アンカーと、戦場マスから全国図への滑らかな投影を担当します。都市ID・論理地形・セーブ形式は変更しません。全国図の勢力色は都市の支配勢力をもとにした概況であり、個々のHEXの占領/補給状態は戦場図で表示します。勢力色画像は都市所有者が変化した時だけ作り直し、海は元画像の色から除外します。

背景は `App/MapArt/china-atlas-v1.png`、内蔵image_genで作った独自の地形画。ユーザーの参考画像は雰囲気の参考に使用し、ゲーム画面の素材を転用していません。生成プロンプトは `App/MapArt/CHINA_ATLAS_PROMPT.md`。位置は表示上の概略で、測量データや史実シナリオではありません。

新規開始/読込はHEX戦場図。全国図はFで任意に開きます。F/下部ボタンで戦場図と切替、出陣と補給表示は戦場図へ移ります。全国図でも都市選択と命令が可能。都市が密集する場所は最寄りの紋章を選択します。`ChinaAtlasTests.cpp` が都市アンカーと全マスの投影の有限性・範囲・隣接の連続性を確認。プレビューは `china-atlas`、`china-atlas-city`、`china-atlas-compact`、`china-atlas-route`。


## HEXグリッドを維持した戦場の描画
通常表示を戦場図に戻し、六角グリッドを初期ONに変更。`CampaignSceneTerrain.cpp::drawHexGrid` は `tileCenter` と `TileShape` を使い、実際の選択・経路と同じ座標で山・森の上に境界線を描画します。表示範囲だけ描画し、H/下部「マス」で切替。参考画像の都市紋章と金色の名前は `CampaignLandscapeDraw.cpp::CityBanner` に反映。名札の描画・クリック領域は `CityBannerRect` を共有し、都市名からも選択できます。詳細マップの所有色は実際のHEXの所有者を使います。


## 褐色の中国マップを維持したグリッド表示
ユーザーの指示を修正し、褐色の全国図の背景・都市位置・勢力色を維持して、その上へ整った六角グリッドを重ねます。新規開始・読込はこの全国図を表示。`CampaignSceneAtlas.cpp::ensureAtlasGrid` が透過RenderTextureへ線をキャッシュし、`BlendState::MaxAlpha` でアルファを書き込みます（既定の2Dブレンドでは透明な描画先のアルファが増えません）。H/「マス」で切替。全国図のグリッドは表示の補助で、既存の96×64論理HEXの地形・移動判定を変更しません。既存の都市選択・部隊進路表示も維持します。


## 全国図のズーム・移動
`AtlasCamera.hpp` は正規化座標でカメラ中心と倍率を保持。ホイールのカーソル位置を維持した1〜6倍のズーム、中ドラッグ・矢印キーの移動、端のクランプ、全図リセットを担当します。`CampaignSceneAtlas.cpp` の `atlasViewport` は固定表示枠、`atlasRect` はカメラから計算する画像の描画範囲です。地形・勢力色・グリッド・都市・部隊・進路・クリック/ホバーが共通の変換を使い、枠外はシザーで切り取ります。グリッドのキャッシュ生成時のみシザーを無効にし、全画像の線を保存。`ChinaAtlasTests.cpp` でカーソル固定・最大最小倍率・移動境界・リセットを検証します。プレビュー `china-atlas-zoom` / `china-atlas-zoom-compact` は倍率3の経路表示です。

## UEの戦場ビジュアル第２段

`TacticalSurface.cpp` が地表の高さとマス選択、`TacticalUnits.cpp` が隊列・装備・荷車、`TacticalStandards.cpp` が揺れる軍旗と幌を担当する。`TacticalVisualTests.cpp` は地形と選択位置、兵の足元、布の法線を確認する。地表・植生・門は `TacticalTerrain.cpp`、衝撃と地形に沿うリングは `TacticalEffects.cpp`。生成元のOBJと素材を保ち、新しい版の素材は `Tools/prepare_battle_cinema.py` で作成する。詳しくは [BATTLE_VISUALS_V2.md](BATTLE_VISUALS_V2.md)。
## UEのマップ第３段

序章の護送経路と戦後の達成方法は `NewGame/JourneyOpening.cpp`、実際の車の進行は `StrategistJourneyObjectives.cpp`。`TacticalOpeningPresentation.cpp` が軍議と戦果の表示内容、`TacticalOpeningInterface.cpp` が経路選択、`TacticalOpeningTests.cpp` が公開命令による勝利確認を担当する。詳細は [OPENING_UPGRADE.md](OPENING_UPGRADE.md)。

戦術ポーズ付き同時進行の時計・継続命令・経路は `NewGame/JourneyRealtime.cpp`、UEの入力接続は `TacticalRealtime.cpp`、操作欄は `TacticalRealtimeInterface.cpp`。既存の敵AIを計画と適用に分け、交戦・補給・護送の更新を手動指揮と共有する。任意切替の12秒周期の試作であり、詳細は [REALTIME_COMMAND_PROTOTYPE.md](REALTIME_COMMAND_PROTOTYPE.md)。

連鎖の合図待ちは `NewGame/JourneyChainSignal.cpp` で現在の予約と同一の計画だけを許可し、観測情報から全手の成立を確認する。`TacticalChainSignal.cpp` が発動・解除・発動後の停止を担当。予測は命令・戦況の更新時に行い、演出中は未来の解決結果を表示しない。通常の手動発動と同じ戦法処理を呼び出す。

伝令の報告は `NewGame/JourneyBattleWatch.cpp` が観測情報と味方の実際の損耗・移動結果から抽出する。`JourneyOrderRead.cpp` は経路の残りと進路なしを読み取る。`TacticalBattleWatch.cpp` は演出後まで報告を保留し、`TacticalBattleWatchInterface.cpp` が履歴・カメラ復帰を担当。詳細は [BATTLE_WATCH.md](BATTLE_WATCH.md)。

行軍計画は `JourneyMarch.cpp` が地点・到着後の命令、`JourneyRealtimeRouting.cpp` が区間と全体の経路を担当する。時計・同時移動は `JourneyRealtime.cpp` に残した。UE側は `TacticalMarch.cpp` が命令と表示、`TacticalMarchInterface.cpp` が計画画面、`TacticalMarchOverlay.cpp` が森でも見える経路を担当する。詳細は [MARCH_PLAN.md](MARCH_PLAN.md)。

地形の高さと稜線の接続キャッシュは TacticalSurface.cpp、地表とHEXは TacticalTerrain.cpp、橋・集落・畑と背景キャッシュは TacticalMapScenery.cpp、曲線の水路と川岸は TacticalMapWater.cpp に分担。TacticalMapMesh.h は共通の上向き地表メッシュを作る。オリジナル岩素材と斜面への混合は prepare_map_art.py。仕様・素材の生成プロンプト・確認結果は [MAP_VISUALS_V3.md](MAP_VISUALS_V3.md)。

## UEの拠点と森の視認性

TacticalFortifications.cpp が公開された都市と任務地点の建築・旗、TacticalFoliage.cpp が観測済みの表示部隊に追従する葉の透過を担当する。形状は make_battlefield_props.py、素材は prepare_field_readability.py。透過には表示用の部隊情報だけを使い、未知の敵の座標を渡さない。詳しくは [FIELD_VISUALS_V4.md](FIELD_VISUALS_V4.md)。

## UE序章の記録（2026-10-26）

`NewGame/JourneySave.hpp` が保存セッションの入口。アーカイブは世界・戦闘・セッションの３CPPに分け、検証は `JourneySaveValidation.cpp` に置く。UEの状態変換は `TacticalSession.cpp`、ファイル保存は `TacticalSaveStorage.cpp`、自動保存は `TacticalCheckpoint.cpp`、確認と画面は `TacticalSaveMenu.cpp` / `TacticalSaveInterface.cpp`。変更時は [JOURNEY_SAVE.md](JOURNEY_SAVE.md) の互換性と再挑戦の資源整合を確認する。

## 画面の責務分割（2026-10-26）

`TacticalInterface.cpp` は組立、`TacticalChrome` は共通装飾。戦闘ヘッダー・武将カード・命令・伝令・地図の札・出会いはそれぞれ `TacticalBattleHeader.cpp` / `TacticalRosterInterface.cpp` / `TacticalCommandsInterface.cpp` / `TacticalBattleFeed.cpp` / `TacticalMapLabels.cpp` / `TacticalEncounterInterface.cpp`。座標、肖像の比率と各画面の意図は [UI_VISUAL_REFINEMENT.md](UI_VISUAL_REFINEMENT.md)。

## 戦場の視点と隊形（2026-10-26）

`TacticalCamera.cpp` に視点操作、`TacticalFormation` に代表兵の配置、`TacticalLabelLayout.cpp` / `TacticalLabelLeaders.cpp` に観測済み部隊札の配置と引き出し線を分けた。操作と、描画がシミュレーションを変えない検証は [BATTLE_VIEW_UPGRADE.md](BATTLE_VIEW_UPGRADE.md)。

## 連鎖の予約を読む画面（2026-10-26）

`TacticalChainInterface.cpp` が顔付き手順カード、`TacticalChainOverlay.cpp` が表示中の担当と対象を結ぶ点線、`TacticalChainVisual.h` が色・状態名を担当する。`TacticalChainPresentationTests.cpp` と視点検証で、予測済み・停止・後続未判定、地図番号の並べ替えを確認。詳細は [CHAIN_VISUAL_UPGRADE.md](CHAIN_VISUAL_UPGRADE.md)。

## 戦場の環境素材と光（2026-10-26）

`prepare_landscape_finish.py` がバージョン付きの地面・木立・岩・草・水と照明、`make_landscape_details.py` が新しい岩の形状を担当。`TacticalSurface.cpp` で連続する稜線、`TacticalTerrain.cpp` で幹・枝・樹冠と法線、`TacticalMapWater.cpp` で深浅のある川を描く。[設定と検証](LANDSCAPE_VISUAL_REFINEMENT.md)。

## 戦法のカットインと演出（2026-10-26）

表示内容は TacticalCinematicRead.cpp、見た目は TacticalCinematicStyle / TacticalInkStroke / TacticalCinematicInterface.cpp、人物絵は TacticalCutinPortraits.cpp、自動の視点は TacticalCinematicCamera.cpp、3D効果は TacticalTacticEffects に分担。TacticalCinematicTests.cpp と視点検証で、実戦果・公開時点・失敗・スキップ・表示だけの視点変更を確認。[仕様と素材](TACTIC_CINEMA_UPGRADE.md)。

## 城郭と山の背景（2026-10-26）

城郭は `TacticalFortressBackdrop.cpp`、山は `TacticalMountainBackdrop.cpp`、外縁の地面は `TacticalBackdropGround.cpp`、背景専用の頂点共有は `TacticalSceneryMesh.h`。`TacticalBackdropTests.cpp` は盤外配置と戦闘データの不変性を確認する。`TacticalMapScenery.cpp` のキャッシュは選択変更で背景を再生成しない。[仕様・素材・確認結果](FORTRESS_BACKDROP_UPGRADE.md)。

## 兵の関節と動作（2026-10-26）

`TacticalSoldierMotion` が観測済みの一手から関節と陣形の補間を求め、`TacticalInfantry` が腕・脚・武器をメッシュの変換へ変える。`TacticalUnits.cpp` は部隊・荷車・旗とバッチ更新に集中する。`TacticalInfantryTests.cpp` は公開時点・未知の敵・関節と武器の接続・一時停止・状態の不変性を確認する。モジュールはCPP間の名前の混入を避けるためunityビルドを使わない。[実装と21検証群](INFANTRY_MOTION_UPGRADE.md)。

## 戦闘中の担当と行動段階（2026-10-26）

`TacticalActivity` は表示中の担当・対象・段階・色、`TacticalActivityRead.cpp` は通常の報告の公開点、`TacticalActivityMarkers.cpp` と `TacticalActivityOverlay.cpp` は向きと対象の盤上表示を担当する。札、武将カード、対象欄へ同じ読み取り結果を接続した。`TacticalActivityTests.cpp` は実連鎖・公開済み損害・未知の敵・支援と停止・状態の不変性を確認。[仕様と22検証群](BATTLE_ACTIVITY_UPGRADE.md)。

## 戦法の抑揚と受け渡し（2026-10-26）

`TacticalSpectacle` は戦法ごとの視点と公開済みの衝撃、`TacticalSignatureEffects.cpp` は大きな斬光と波、`TacticalCinematicLens.cpp` は一時的なブルーム等を担当する。`TacticalCinematicRelay.cpp` は次の予約担当の顔と名前を表示し、次の成否は読まない。`TacticalSpectacleTests.cpp` と視点検証で、味方への支援、未発見の対象、停止、最後の手、OFF時の復帰を確認する。[仕様・実描画・23検証群](TACTIC_SPECTACLE_UPGRADE.md)。

## 連鎖の戦術記録（2026-10-27）

`TacticalChainReview` は実行イベントから各手の役割・成果・つながりを作り、`TacticalChainReviewRead.cpp` は現在の段階と観測情報から表示可能な記録を読む。`TacticalChainReviewMenu.cpp` は停止と開閉、`Rows` / `Interface` は画面、`Tests` / `UiTests` は保存・実連鎖・排他の確認を担当する。既存のlastChainを使い、保存形式を増やさない。[仕様と25検証群](CHAIN_REVIEW_UPGRADE.md)。

## 任務別の戦場構成（2026-10-28）

`NewGame/JourneyBattlefield.hpp` は三つの地形を文字の表で定義し、`JourneyMission.hpp` は任務・拠点・期限・配置を定義する。`StrategistJourneyObjectives.cpp` が出陣時に適用するので、移動・射線・AI・行軍・描画は同じ地形を使う。Unrealの配色・尾根高・樹木の縮尺は `TacticalBattlefieldStyle.h`、水と橋の方向は `TacticalMapWater.cpp` / `TacticalMapScenery.cpp`、烽火台は `TacticalFortifications.cpp`。共有検証は `tests/JourneyBattlefieldTests.cpp`、実描画と選択レイの検証は `TacticalVisualTests.cpp`。追加・編集の説明は [任務別の戦場構成](BATTLEFIELD_VARIETY.md)。

地表の細部は `TacticalGroundDetail.h`、共通の護送経路から道・轍を作る処理は `TacticalMapRoads.cpp`。道の整地と部隊の足元・選択は `TacticalSurface.cpp` の同じ高さを使う。`TacticalMapDetailTests.cpp` が南の林道と街道の表示一致・キャッシュ更新・描画の不変性を確認する。川岸は `TacticalMapWater.cpp` に分離している。


任務別の周辺景観は `TacticalSceneryLayout.h` に集約。農村は `TacticalVillageBackdrop.cpp`、河港の荷船・桟橋は `TacticalRiverPort.cpp`、盤外の川は `TacticalSceneryWater.cpp`。`TacticalFortressBackdrop.cpp` と `TacticalMountainBackdrop.cpp` が構図のデータから城壁と峰を作る。`TacticalCamera.cpp` は同じデータで景観視点を合わせ、`TacticalBackdropTests.cpp` と `TacticalCameraTests.cpp` が三戦の差・非衝突・描画の不変性・カメラ追従を確認する。


## 軍議の戦場見取り図（2026-10-28）

`TacticalPreparationMap.h` は公開情報だけの表示データ、`TacticalPreparationMapRead.cpp` は次の任務の読み取り、`TacticalPlanningMap.h/.cpp` は地図とクリック座標、`TacticalPlanningMapInterface.cpp` は画面、`TacticalPlanningSelection.cpp` は地点・経路の選択を担当する。既存の作戦書とキーの接続は `TacticalNovelPlanInterface.cpp` / `TacticalEpisode.cpp`。`TacticalPlanningMapTests.cpp` は実出陣との一致、敵方針の非表示、4,536通りの座標、クリック、閲覧の不変性、案内料と出陣への引き継ぎを確認する。[操作と範囲](COUNCIL_MAP.md)。


## 軍議の担当と配置（2026-10-28）

`JourneyAssignment.hpp/.cpp` は指定条件と配置成立、`JourneyAssignmentOrders.cpp` は既存行軍への変換。`TacticalPlanningDuty.cpp` は公開操作、`TacticalPlanningDutyInterface.cpp` は担当パネル。戦後は `JourneyBattleStory.cpp` / `JourneyDrama.cpp`、保存は版4。`JourneyAssignmentTests.cpp` と `TacticalPlanningDutyTests.cpp` が実行と再開を検証する。[範囲と操作](COUNCIL_DUTIES.md)。


## 作戦立案の案内（2026-10-28）

`TacticalPlanningGuide.h/.cpp` は公開地形だけから地点名・命令の文章・仮経路を作る。`TacticalPlanningGuideRead.cpp` は武将選択・理由・確認文の接続、`TacticalPlanningDutyInterface.cpp` は①誰→②どこ→③何の手順、`TacticalPlanningMapInterface.cpp` は地図と確定済み三人の一覧。選択と確定は別操作。`TacticalPlanningGuideTests.cpp` は公開情報の分離、全地点へのHEX経路、選択時の不変性と誤確定の防止を確認する。


## 難易度と作戦の見本（2026-10-28）

`JourneyDifficulty.hpp` に敵兵率と期限追加、`JourneyPlanExample.hpp/.cpp` に三戦の見本と軍議での変更を集約。実際の出陣は `StrategistJourney.cpp`、難易度の保存は版5。UEの新規開始の初期値は `TacticalField.cpp`、操作は `TacticalAccessibilityInterface.cpp` / `TacticalPlanningExample.cpp`。`JourneyAccessibilityTests.cpp` と `TacticalAccessibilityTests.cpp`、両難易度の `JourneyPlaythroughTests.cpp` が実行と継続を確認する。[仕様](BATTLE_ACCESSIBILITY.md)。
