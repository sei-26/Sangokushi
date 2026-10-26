# 直近の連鎖を振り返る戦術記録（2026-10-27）

連鎖の見応えに続き、実際に何をつなぎ、どこで止まったかを読める画面を追加した。敵を動かす役、隙や混乱を作る役、攻撃する役、支える役を実行順に見比べ、次の判断に使う。

## 開き方

- 戦闘中、連鎖の再生が終わってから **Y / 伝令欄の「戦法記録」** で開く。
- 戦後・撤収後は「直近の連鎖を振り返る / Y」からも開ける。
- Y、Esc、閉じるボタンで閉じる。リアルタイムの指揮は閲覧時に停止し、閉じても停止を保つ。Spaceで再開する。
- 再生中は開けない。初戦の前や、再挑戦の軍議・次戦の準備には前の記録を出さない。

記録するのは、その戦いで最後に実行した連鎖。全戦闘の履歴一覧や、一戦全体の総損害ではない。戦後画面にある任務の達成・護送・帰還兵の振り返りと併用する。

## 表示する内容

担当の顔と名前、戦法、役割、実際の対象、成果、前の手とのつながり、成立・停止の報告を並べる。成功した手の実損害を合計し、止まった手には理由を残す。停止後の未実行の手を成功として並べたり、未実行の損害を合計したりしない。

例：誘導→伏兵→突破では「第１手で動いた同じ敵に伏兵」「第２手の混乱に続いて突破」と示す。義勇の誓い→大喝→青龍偃月では、鼓舞が攻撃で消費された記録と、同じ敵への混乱を照合し、準備役と主攻の両方の貢献を示す。

一つの条件だけが唯一の勝ち筋だったとは断定しない。突破には混乱または士気60以下と押し出す空き、青龍偃月には隙・混乱または士気60以下など、既存ルールの代替条件を残す。基礎ルールをこの表示のために変えない。

鼓舞は実際にイベントに記録された味方の状態変化を数える。士気が上限の場合に18や36の回復を実績として加算しない。対象名は味方と観測済みの敵の情報から作り、未発見の別働隊の実データを見て名前や位置を補わない。過去の対象の現在位置へカメラを飛ばす操作は付けない。

## 保存と画面の独立性

既存の `StrategistJourney::lastChain` のイベントを使う。既存の保存形式にすでに含まれるため、新しい保存項目や形式変更は不要。読み込み後もその記録から画面を作り直す。予測結果を実行記録として表示しない。

画面を開いた時点で表示用のコピーを保持し、閉じると破棄する。閲覧は部隊・資源・予約・戦果を変更せず、リアルタイムの停止だけを行う。地図操作・戦闘命令・Enterによる進行は閲覧中に通さない。保存、伝令帳、行軍計画を開く操作では、この画面を閉じて重複を避ける。

## ファイルの役割

以下は `Unreal/Source/GunyuTactics/` 内。

| ファイル | 担当 |
|---|---|
| TacticalChainReview.h / .cpp | 実行イベントから役割・実損害・条件のつながりを作る |
| TacticalChainReviewRead.cpp | 現在の段階・再生中の制限、観測情報から名前を作る |
| TacticalChainReviewMenu.cpp | 開閉、時間停止、Y / Esc、表示コピーの寿命 |
| TacticalChainReviewRows.cpp / TacticalChainReviewInterface.cpp | 各手の行、全体の画面と重なりの防止 |
| TacticalBattleFeed.cpp / TacticalJourneyInterface.cpp | 戦闘・戦後から開く入口 |
| TacticalController.cpp / TacticalSaveMenu.cpp / TacticalMarchInterface.cpp / TacticalBattleWatchInterface.cpp | 入力制限と他メニューとの排他 |
| TacticalChainReviewTests.cpp / TacticalChainReviewUiTests.cpp | 実連鎖・停止・観測・保存・一時停止・開閉・撮影 |

## 検証

- `Intermediate/chain-review-final-build.log`：UEビルド成功。
- `Intermediate/chain-review-final-smoke.log`：UE内25検証群すべてPASS。GUNYU_CHAIN_REVIEWとGUNYU_CHAIN_REVIEW_UIを追加。
- 実連鎖の敵兵損耗969、停止後の損害除外、未知の敵名の除外、再生中の非公開、読み取り前後の保存対象データ一致、保存・読み込み後の再構成、戦後・再挑戦、士気上限での支援を検証。
- 軍議で兵糧を厚くする準備を選び、公開操作で配置してから固有連鎖を実行。実損害1022、前の混乱と鼓舞の消費の両方を確認。伏兵準備では関羽が移動済みになるので、この検証では準備の選択も手順に含める。
- 開く前に実際に指揮が進む状態を作り、閲覧で停止すること、閉じても停止とデータを保つこと、保存・伝令帳・行軍計画との排他を検証。
- `Intermediate/chain-review-complete.png`：1600×900、誘導・伏兵・突破。
- `Intermediate/chain-review-stopped.png`：1280×720、１手成立と第２手の停止。
- `Intermediate/chain-review-signature-final.png`：1280×720、誓い・大喝・青龍偃月。
- `Intermediate/chain-review-debrief.png`：1280×720、戦後から同じ記録を閲覧。

撮影は `-GunyuCapture -GunyuChainReview` に `-GunyuChainReviewStop` / `-GunyuChainReviewSignature` / `-GunyuChainReviewDebrief` を追加。既存の撮影用保存領域を使い、プレイヤーのセーブとは分ける。画像とマップの新規素材は追加しない。
