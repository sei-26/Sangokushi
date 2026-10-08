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
