# コードを読むための地図

まず Main.cpp → NewGame/CampaignScene.cpp → NewGame/CampaignTurn.cpp の順に読むと、起動・画面・ゲームルールのつながりが分かる。

ヘッダー（.hpp）は主に「持っているデータと、呼べる関数」。実際の処理は同名や役割名の.cppにある。同じクラスの関数を複数の.cppに分けているため、別ファイルでも同じゲーム状態を扱う。ファイルを分けるために別のシステムへ作り直したわけではない。

## 最初に読む5ファイル

| ファイル | 分かること |
|---|---|
| Main.cpp | 起動設定、フォント登録、更新と描画を毎フレーム呼ぶ場所 |
| NewGame/CampaignScene.cpp | 自由戦略の画面全体と英雄譚への切替 |
| NewGame/CampaignTypes.hpp | 都市・武将・部隊・任務が持つ数値 |
| NewGame/Campaign.hpp | 自由戦略で呼べる命令と、ゲームが保持する状態 |
| NewGame/CampaignTurn.cpp | 1日を進めるとき、何をどの順で処理するか |

## 自由戦略のルール

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
| CampaignScenePanel.cpp | 右側の都市・部隊の情報と命令ボタン |
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

- CampaignAI.cpp：命令の回数管理と実行。10日3命令・同日の再実行防止。
- CampaignAIPlanning.cpp：危機、兵糧、武将、到達できる目標を評価し、優先順位つきの候補を作る。
- CampaignAIArmies.cpp：日々の撤退、近くの城の救援、輸送隊護衛、攻撃対象、戦法の発動判断。
- HeroStoryAI.cpp：地形・占有・射線・損害・守備から、敵の攻撃対象と移動先を選ぶ。
- tests/AITests.cpp：状況別の判断と、3勢力の長期シミュレーション。

命令を1つ実行するたびに候補を作り直す。武将や都市資源が使用済みになるため、実行前の候補を続けて流用しない。判断の理由は群雄の記録へ残す。

- `NewGame/CampaignAIOperations.cpp`：敵軍の攻城隊と支援隊の足並み、集結待機の期限、攻勢中止。日次移動の停止情報を返し、戦闘停止とは別に判断する。

- `NewGame/CampaignBattlefield.cpp`：敵部隊の補給封鎖範囲、味方護衛と守備兵による保護、方向数に基づく包囲圧力。補給・輸送経路・戦闘・AI・画面が同じ評価関数を使う。

- `NewGame/CampaignTerrainDraw.cpp`：軍略地図の地面・山並み・森林・水面・橋。固定の地形描画は地図テクスチャにキャッシュする。
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
