# 城郭と山並みの背景（2026-10-26）

三國志8 REMAKE PKを参考にした戦法演出に続き、戦場の周囲を城郭・集落・重なる山並みで構成した。目的は、部隊が戦っている土地の広がりを見せること。既存の軍師としての命令・連鎖・観測・任務を維持する。

## 今回の見た目

- 北側に石積みの城壁、狭間、四隅の楼閣、二層の城門、内部の屋根と街並みを配置。暗い瓦と赤茶の木組みで形を分ける。
- 背後に二層の実メッシュの山並みを配置。山裾は暗い緑、遠い稜線は青灰色へ寄せ、既存のオリジナル岩テクスチャで表情を加える。
- 大きな重複平面を外し、細かい戦場外縁と高さを合わせた地面を続ける。カメラを動かしたときに地面の縁が浮く段差を解消。
- 標準視点の仰角を48度へ調整。Kの切替は「標準→臨場→俯瞰→遠景」。遠景は城郭と戦場を一緒に見る。次の標準へ戻ると遠景へ入る前の焦点に戻る。Homeは戦場全体、Jは選択隊。

城郭は現在の任務とは別の背景。占領・攻城・城内移動のルールは追加していない。建物と山はプレイ可能な最初のHEXより北側に収め、衝突判定を持たせない。従来の目的地の門と任務表示が実際の目標になる。

## エディタでの調整

`Unreal/GunyuTactics.uproject` → `/Game/Maps/TacticalTrial` → Tactical Fieldを選択する。

- Trial / ArtのFortress Materialが城壁、Distant Materialが山並みの素材。
- BackdropTimber / BackdropRoofsコンポーネントの素材が木組み・瓦。既存のGate Roof Meshを屋根にも使う。
- `/Game/Art/Materials/M_FortressMasonryV6`のBrickWidth / BrickHeight、StoneTint / Mortar / MasonryRoughnessで石積みを調整する。
- `M_FortressTimberV6`と`M_FortressRoofV6`のTint / Roughness、`M_InkMountainV6`のDistancePalette / RockGrainScaleも編集可能。

`Unreal/Tools/prepare_fortress_backdrop.py`が新しい素材を作り、既知の旧標準素材だけを差し替える。既存の独自グラフやユーザーが差し替えた参照を上書きしない。このスクリプトとverify_editor.pyはマップを保存するため、ゲーム・他のマップ検証プロセスが終了してから実行する。外部作品の素材は追加していない。

## ファイルの役割

| ファイル（Unreal/Source/GunyuTactics内） | 担当 |
|---|---|
| TacticalFortressBackdrop.cpp | 城壁、門、楼閣、家並みのインスタンス配置 |
| TacticalMountainBackdrop.cpp | 山の形・稜線・距離による色 |
| TacticalBackdropGround.cpp | 外側の地面、境界の高さの接続 |
| TacticalSceneryMesh.h | 背景専用の頂点共有メッシュ |
| TacticalMapScenery.cpp / TacticalState.h | 地形変更時だけ背景を再構築するキャッシュ |
| TacticalCamera.cpp | 通常視点と遠景の切替 |
| TacticalBackdropTests.cpp | 背景と戦闘状態が独立することの検証 |

背景のメッシュは三角形ごとの頂点複製をやめ、隣接面で共有する。選択や移動候補の更新では再生成しない。マップ保存後のサイズは102,952,306バイト（約98.2MiB）。頂点共有前の試作167,375,866バイトから削減した。長期的な容量・起動時間の最終最適化は別途必要。

## 確認

- ビルド：`Intermediate/fortress-delivery-build.log`、Succeeded。
- UE内20検証群：`Intermediate/fortress-delivery-smoke.log`、全PASS。背景の再生成前後で保存対象の戦闘状態が一致し、建物・山が盤外に収まり、外縁の高さが連続することも確認。
- カメラ検証：48度でのHEX選択、遠景への切替と元の焦点への復帰。既存の戦法演出・連鎖・保存の検証もPASS。
- エディタ：`Intermediate/fortress-editor-final.log`、GUNYU_EDITOR_CHECK: PASS。編集可能な10種類の素材参照、13種類のモデル参照、肖像とマップ保存を確認。
- 実描画：`Intermediate/fortress-delivery-photo.png`（背景）、`Intermediate/fortress-delivery.png`（遠景とUI）。通常戦闘・戦法の720pキャプチャも確認。

近接での兵の動作、屋根や建築の造形の細部は引き続き改善する。背景追加を新しい戦術・歴史区間の完成とは扱わない。
