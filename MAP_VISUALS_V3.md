# 戦場マップの作り込み / 2026-10-24

狙いは「敵を読み、地形と武将を使って勝ち筋を組む」戦場を、眺めても場所の意味が伝わる景観へ進めること。

- 山は隣接する山岳マスをつなぐ稜線と共有斜面へ変更。地面・部隊・選択光線・グリッドは同じ高さを参照する。地形の接続は任務の地形が変わった時だけ作り直す。斜面の法線も共有の高さから求め、山肌の陰影を滑らかにする。
- 北側のプレイ範囲外には、手前と奥の２層の山並みを立体メッシュで追加。
- 山肌は独自生成の岩素材を使用。地表の傾斜に応じて岩を出し、森・草地・山麓の色を近隣マスの間で連続させる。
- 川は実際の河川・橋マスの中を通る連続した曲線の水路として描画し、湿った土、小石、葦を流れに沿って追加。湿った岸も既存の河川マスに含まれ、通行条件は変わらない。橋は板張り、支柱、欄干、下梁を実形状で作る。橋の床上面と部隊が使う高さは一致する。
- 南側のプレイ範囲外に家並みと畑、既存の通行不可の都市マスに囲いを持つ拠点を置く。開けた通行可能マスの中央に大きな装飾建物を追加しない。
- グリッドの辺も分割し、起伏に沿って表示する。森・川・山の移動条件、視界・連鎖・敵AI・勝敗条件は変更しない。

## 操作と編集

`Unreal/MapPreview.cmd` または `Unreal/Tools/launch.ps1 -MapPreview` で戦場の全景からフルスクリーン起動。ホイールで拡大縮小、矢印で移動、Homeで全景、TabでUI表示切り替え。

Unreal Editorの `/Game/Maps/TacticalTrial` にあるTacticalFieldの `Trial | Art` から地表と岩のマテリアルを編集できる。`prepare_map_art.py` は既存の独自マテリアル参照を保持し、標準素材だけを新バージョンへ差し替える。

| ファイル | 担当 |
|---|---|
| TacticalSurface.cpp | 共有の高さ、稜線のキャッシュ、選択判定 |
| TacticalTerrain.cpp | 地表、グリッド、植生、周辺地形 |
| TacticalMapMesh.h | 分割地表と線の共通メッシュ作成 |
| TacticalMapScenery.cpp | 村、畑、拠点、橋、周辺の丘陵 |
| TacticalMapWater.cpp | 曲線の水路、岸辺、葦、岸の石 |
| prepare_map_art.py | 岩画像の取込、斜面素材、漆喰素材 |

## 岩素材の出典と再現

生成方式：内蔵 `image_gen`（builtin）。他ゲームの画像や素材は使用していない。

保存先：`Unreal/ArtSource/Battle/rock-albedo-v3.png`（実ファイル1254×1254、UEでは最大1024・ミップ使用）。取込先：`/Game/Art/Battle/T_MapRockV3`。生成画像は地図の引き伸ばし背景ではなく、実際の立体地形に繰り返して使用する表面素材。

最終生成プロンプト：

```text
Use case: photorealistic-natural. Asset type: seamless tiling base-color texture for real 3D rock ridges and cliff meshes in an original historical Chinese tactical strategy game made in Unreal Engine. Generate a square 1536 x 1536 texture, straight perpendicular close-up of a weathered limestone rock face, filling the whole image. Natural stratified pale warm grey and taupe stone, irregular bedding layers, thin dark mineral fissures, fine granular erosion, subtle ochre dust in shallow cracks and very sparse muted lichen. Restrained natural colors, high photographic surface detail at two scales. This is flat albedo material data under broad neutral diffuse light: NO directional shadows, NO strong baked highlights, NO perspective, NO sky, NO horizon, NO standalone rocks, NO 3D objects, NO mountain landscape, NO vegetation silhouettes, NO text, NO labels, NO hex grid, NO borders, NO watermark. Left/right and top/bottom edges should repeat seamlessly. Avoid a single central distinctive feature or overly deep black cracks. The material will be tiled at real scale on modeled rocky ridges, not enlarged as a map backdrop.
```

## 検証

- Development Editorのビルド成功：`Intermediate/map-v3-channel-final-build.log`。
- UE実行テスト７項目すべてPASS：`Intermediate/map-v3-channel-final-smoke.log`。命令・連鎖・旅程・約束・戦法・敵情報・地表選択を確認。
- ３任務・２方向から計1,008のマス中央を正しく選択。川際を連続した斜面にし、光線は最初の交差から細分化することで、奥の岸を選ぶ問題を修正。
- 曲線の水面の全頂点が実際の河川・橋マス内に収まり、共通の水面高さを保つことも検証。
- Editorから任務・素材を変更できること、７素材・９モデル・３肖像の参照を確認：`Intermediate/map-v3-channel-editor.log`。
- 素材作成の再実行では新規取込０件で既存の参照を維持。Python構文と素材マニフェストの参照元も確認。
- 最新の全景：`Intermediate/map-v3-valley-overview.png`。川・橋の拡大：`Intermediate/map-v3-channel-river.png`。UIを表示した2560×1440の確認：`Intermediate/map-v3-final-ui.png`。
- 今回の端末の1600×900・オフスクリーン撮影では、読み込み後90フレームの平均が全景5.50ms、川の拡大5.21ms。通常UIの2560×1440撮影は13.46ms。測定条件が異なるため比較ベンチマークやFPS保証には使わない。
- Editorで任務の値を変えた後に最新の景観へ再生成されることも、岩のインスタンス数で確認。

## 現時点の限界

地理と建築は歴史戦場の雰囲気を作る試作景観。特定の古戦場や後漢建築の考証済み復元ではない。家屋は拡大時の最終品質のモデルへ順次置き換える余地がある。水路は既存の河川マスを使うため、曲がりの配置は任務の地形に従う。川岸は通行可能な新しい地形を追加するものではない。
