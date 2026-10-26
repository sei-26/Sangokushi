# 戦場の美術素材 / 2026-10-22

`ground-albedo-v1.png` は内蔵 image_gen で生成したオリジナルの地表用画像。生成時に指定したサイズは2048角で、返された元画像の実寸は1254×1254。UE側のビルド時に1024×1024へ縮小し、ミップマップを生成する。元画像は保持し、拡大で精細さを補ったとは扱わない。画像を拡大した背景として貼るのではなく、UEの地面のUVに繰り返して使う。実際の地形色・街道・視界は共通C++の観測情報から作った頂点カラーで重ねる。凹凸の法線はマテリアル内で隣接画素の差から弱く作る方式で、計測由来のPBRデータではない。

- 元画像：`Unreal/ArtSource/Battle/ground-albedo-v1.png`
- 取込先：`/Game/Art/Battle/T_BattleGround`
- 地表：`/Game/Art/Materials/M_BattleGroundTextured`
- 生成方式：内蔵 image_gen。CLI・外部アセット購入は使用していない。

## 最終生成プロンプト

```text
Use case: photorealistic-natural. Asset type: original seamless tiling base-color texture for a 3D historical Chinese tactical battlefield in Unreal Engine. Generate a square 2048x2048 flat surface texture seen exactly perpendicular from above, filling the entire frame edge to edge. A realistic dry grassy earth surface in late-summer northern China, with irregular small patches of short desaturated olive meadow grass interspersed with warm ochre brown compacted soil, scattered tiny grey pebbles, worn straw and fine root detail. Approximately two meters of ground across the whole image. Restrained natural colors and detailed realistic microstructure, not cartoon, no exaggerated contrast. Broad diffuse neutral lighting with NO directional shadows, NO baked ambient occlusion, NO perspective, no horizon, no vignetting. Match left/right and top/bottom edges for repeat tiling. This will be repeated across real 3D terrain, not used as a scene backdrop. No trees, no bushes, no buildings, no roads, no large rocks, no troops, no hex grid, no text, no labels, no border, no watermark.
```

## 形状の素材

以下のOBJは `Tools/make_battle_meshes.py` による独自の形状。画像から自動復元したモデルや既存ゲームから流用したモデルではない。Blender等で直接編集し、UEで再インポートできる。単位はcm、Zが上。

| ファイル | 役割 |
|---|---|
| SM_LeafCrown.obj | 折れ目のある葉を集めた樹冠。板状の葉は両面描画し、頂点シェーダーで揺れる |
| SM_RockOutcrop.obj | 不規則な稜線と平面を持つ岩塊 |
| SM_GrassClump.obj | 先が細くなる複数の草葉 |
| SM_LamellarArmor.obj | 胸・肩・腰を覆う小札状の鎧 |
| SM_WarHelmet.obj | 兜の鉢・庇・首の防具 |
| SM_InfantryShield.obj | 中央が膨らんだ盾と縁 |
| SM_GateRoof.obj | 軒が反る門の屋根 |

再作成の入口は `Tools/prepare_battle_visuals.py`。既存のOBJ・取り込み済みStaticMesh・作成済みマテリアルのグラフは上書きしない。地表画像は `ArtSource/manifest.json` にも記録する。形状・見た目は試作段階で、人体のリグと骨格アニメーションは別工程。

## コアとの関係

薄いHEX線は実際のマス座標に一致させる。装飾の草木・岩・起伏・門・霧は追加の移動判定や視界を持たない。森・山・川のルールは既存の共通C++が決める。戦法の衝撃は解決済みの結果を再生し、演出から追加のダメージを作らない。

## 2026-10-24の第２段

`SM_SpearheadV2.obj` と `SM_CartWheelV2.obj` は `Tools/make_battle_details.py` による独自形状。槍先は中央の稜と細い付け根を持ち、車輪は輪とスポークの実形状を持つ。既存ファイルは保持する。

軍旗・幌は `TacticalStandards.cpp` が描く独自の分割曲面。旗の縁取りと武将ごとの紋章もコードから作成した意匠で、歴史資料の旗を複製したものではない。今回、新たな外部画像・モデルは使用していない。地表は既存のオリジナル画像を細部と大きな色むらへ別々の縮尺で使う。新しい素材と色の調整は `Tools/prepare_battle_cinema.py`。
## マップ素材の第３段

2026-10-24に内蔵image_genで作成したオリジナルの岩素材を rock-albedo-v3.png に保存。最終プロンプト・取込先・用途は MAP_VISUALS_V3.md を参照。

## 戦場拠点の第４段

SM_PalisadeStakeV4.obj、SM_WatchtowerV4.obj、SM_CampTentV4.obj、SM_SupplyCrateV4.obj は Tools/make_battlefield_props.py による独自形状。既存のOBJと取り込み済みモデルは保持する。追加画像は使っていない。素材・表示補助・検証は FIELD_VISUALS_V4.md を参照。
