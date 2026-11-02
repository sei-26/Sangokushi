# 戦場全体の質感と光（2026-10-26）

## 2026-11-02：曲面、指揮所、土煙と戦法の見せ場

地形の高さとHEXを保ち、地面の分割を4から8へ細かくした。同じ位置の頂点・法線・UV・色は共有し、戦場と外周の分割をそろえる。曲面の細かさを上げながら、保存したマップアセットは約103MBから81MBへ縮小した。フレームレートが何割上がるという意味ではない。

移動可能範囲だけの変更では、地面・樹木・建物を再生成せず、グリッドの表示だけを更新する。視界、地形、任務、都市の情報が変わる時は静的な場面も更新する。生成物の高精細化と、指示時の不要な作り直しを分けた。

河港を奪う本編の指揮所は、出口の門から、天幕・軍鼓・作戦卓・軍旗のある陣所へ変更。部隊の中心とクリック対象は従来の任務マスに保つ。装飾は公開された任務地点だけを使い、敵の位置や非公開の命令から配置しない。新しい通行・衝突判定も加えない。退路の門と旧試作の護送先は維持する。

新素材 `M_GroundDustV6` は、輪郭と地面との交差をぼかした透過の土煙。観測済みの部隊が実際に動いている時だけ行軍の煙を出し、被弾の煙は公開された兵の損害と発動の時点（48％以降）を確認して出す。停止した連鎖・未確認の敵・支援・予定の経路から被弾を作らない。最大6隊で行軍と被弾を合わせても144個以内。煙の座標は描画時に同じ地面へ合わせる。

戦法カメラが有効な見せ場では、操作パネルと下部の武将一覧を退け、戦場を横いっぱいに見せる。発動者の名前・戦法・コンボと実際の戦果を画面脇へ寄せ、部隊の頭上を覆わない。表示部隊だけを使うカメラは従来より寄り、プレイヤー自身の視点設定は書き換えない。戦況の見出し・伝令・スキップは残す。通常の行軍・同時進行の指示と、戦法カメラを無効にした時の操作は従来の画面を保つ。戦法終了時には操作画面へ戻る。

責務は次のファイルへ分ける。

- `TacticalGroundMesh.h`：共有頂点を使った地面の生成。
- `TacticalGroundGrid.h`：グリッドと移動範囲だけの更新判定。
- `TacticalCommandPost.h`：公開された指揮所の造形。
- `TacticalGroundAtmosphere.cpp`：表示中の実移動・実損害から出す煙。
- `prepare_ground_atmosphere.py`：編集可能な新素材の作成と、マップのDustコンポーネントへの接続。既存の独自素材は置き換えない。
- `TacticalCinematicCamera.cpp` / `TacticalCinematicInterface.cpp`：見せ場の距離と戦果の配置。

新素材のDustColor・Density・GroundFadeはEditorで編集できる。形状と素材をコードから再現でき、従来の素材・照明と戦闘ルール、保存形式10を保つ。

検証はDevelopmentビルド、UE通し55群、最後のカメラ調整後の重点2群、更新した単体版の重点6群が成功。三戦で合計1,008本の視線による選択、共有頂点、範囲表示での既存インスタンス保持、視界と都市の情報による再生成、実際の伏兵から描画コンポーネントまでの煙、プレイヤーの視点設定の保持を確認した。1280×720で河港・稜線・行軍・被弾を描画し、単体版でも戦果の脇への配置と演出終了後の操作画面への復帰を目視確認した。

画像検証の `GunyuCapture` と `GunyuCinema` を併用した時だけ、手動の再生速度1へ固定し、CueTimeの指定をマップの再生設定に依存させない。通常のゲーム設定はそのまま使う。QAは別UserDirへ分離した。

---

UI整理に続き、地形・樹木・岩・川・照明をまとめて改良した。任務・戦法・移動可能なマス・敵の判断・保存形式は変更しない。山の見た目の高さは変え、描画とクリック判定が同じ SurfaceHeight を使う構造を維持する。

## 変えた見た目

- 地面：既存のオリジナル地表テクスチャと細部の凹凸を残し、黄褐色への偏りを抑える色調の新素材を作成。地表の低い起伏にも連続した法線を計算し、三角形ごとの陰影差を減らした。
- 稜線：一様な高さの頂部を、位置に応じて連続的に変わる稜線へ変更。川への下降、橋、護送道路と共有する高さ処理を維持する。
- 木立：角柱の幹を円柱にし、各木に根元と接続する３本の枝を追加。樹冠の幅・傾き・葉の色を変える。同じインスタンス描画を使い、未知の敵の位置を素材に渡さない既存の葉の透過を残す。
- 岩：オリジナルの新しい岩 OBJ を作成。輪郭を細かくし、丸みと風化した形にする。素材も地面になじむ色調へ変更する。
- 川：既存の通れない川マス内で、浅い岸辺と中央の深い色を別の頂点へ割り当てる。視線角度による反射色と既存の時間変化する波の法線を組み合わせる。水面自体の高さは -9cm のまま。橋の周囲を避け、川マス内だけに半分埋まった小石を追加した。
- 光：夕色への偏りを弱め、空からの補助光を増やして森の暗部を読みやすくする。遠方には薄い青灰色の霞を置く。既存の非ボリューム霧を使う。

地形や植生を毎フレーム作り直す処理は追加していない。インスタンス群と静的な地形キャッシュを維持する。新しい草素材は画素ごとの Noise を使わず、インスタンスごとの色の違いだけで描く。Lumen・レイトレーシングは有効化していない。

## Editorでの調整

`/Game/Maps/TacticalTrial` の Tactical Field を選択する。

| 設定 / 新素材 | 主な用途 |
|---|---|
| Trial / Art / Terrain Material → `M_LandscapeGroundV5` | EarthBalance で地面の色調 |
| Trial / Art / Water Material → `M_LandscapeWaterV5` | ReflectedSky、ReflectionStrength、WaterRoughness、WaterSpecular |
| Trial / Art / Foliage Material → `M_LandscapeCanopyV5` | CoolLeaf / WarmLeaf と観測部隊の葉の切り抜き |
| Trial / Art / Rock Material → `M_LandscapeRockV5` | StoneBalance |
| Trial / Art / Rock Mesh → `SM_WeatheredBoulderV5` | 岩の輪郭 |
| TreeTrunks / GrassTufts のコンポーネント素材 | `M_LandscapeBarkV5` / `M_LandscapeGrassV5` |
| DirectionalLight / SkyLight / ExponentialHeightFog | 光の方向、暗部の明るさ、遠景の霞 |

既存の旧素材は残す。新素材の作成スクリプトは既存の独自素材の参照を置き換えず、照明は GunyuLandscapeV5 タグを付けて一度だけ設定する。以後の再実行ではEditorで調整した照明を保持する。

## ファイルの責務

- `Unreal/Tools/make_landscape_details.py`：独自の岩 OBJ の生成。既存ファイルを上書きしない。
- `Unreal/ArtSource/Battle/SM_WeatheredBoulderV5.obj`：DCCで編集できる元形状。
- `Unreal/Tools/prepare_landscape_finish.py`：新素材・参照・照明。C++ビルド後、既存と同じ Python commandlet で実行する。
- `TacticalTerrain.cpp`：幹と枝、樹冠、滑らかな地表の陰影。
- `TacticalSurface.cpp`：起伏のある稜線と共通の高さ。
- `TacticalMapWater.cpp`：浅瀬から中央への断面、小石の配置。
- `TacticalTrial.h`：編集可能な WaterMaterial を追加。

## 検証記録

- ビルド成功：`Intermediate/landscape-release-build.log`。
- 素材生成・反映成功：`Intermediate/landscape-art-final.log` の GUNYU_LANDSCAPE_ART。
- UEの18検証群が通過：`Intermediate/landscape-release-smoke.log`。３任務の地表への1008本の選択光線、回転したカメラのマス選択、保存・再開、連鎖、同時進行を含む。
- 幹と枝の構成数、装飾が移動を妨げないこと、川の頂点が川・橋マス内に収まり高さ -9cm を維持すること、未確認の川でも深浅の色の違いを持つことを追加確認。
- Editor確認：`Intermediate/landscape-editor-final.log` の GUNYU_EDITOR_CHECK。新しい水素材を含む８つの素材参照と、既存の葉の透過マスク、マップの保存成功を確認。マップ保存を行うEditor検証は、マップを読み込むゲーム検証と同時実行しない。
- 実画面：`Intermediate/landscape-complete.png`、`Intermediate/water-final.png`、`Intermediate/landscape-overview.png`。SM5で新素材が通常描画されることを確認する。
- 変更前の比較画像：`Intermediate/landscape-before.png` / `Intermediate/water-before.png`。
- マップ変更前のコピー：`Intermediate/landscape-backup/TacticalTrial.umap`。Intermediate は Git 対象外。

兵士は引き続き代表兵のインスタンスモデル。今回の改良は地形と環境の質感で、スケルトン兵の導入や大規模な新戦場の追加ではない。
