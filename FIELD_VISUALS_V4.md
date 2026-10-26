# 戦場の拠点と部隊の見やすさ / 2026-10-24

敵を読み、地形を使い、狙った勝ち方へ部隊を動かすコアに沿い、拠点・突破口の識別と森の中の隊列の視認性を改善する。

## 拠点と任務地点

- 既存の通行不可の都市マス内に、先端を尖らせた木柵、梯子・床・手すりを持つ物見櫓、入口を開いた天幕、板と帯で組んだ物資箱を配置。
- 都市の公開済みの所有者に合わせ、青緑または赤の軍旗を表示。任務地点は金色の旗と既存の金色HEXで区別する。軍旗の意匠は創作。
- 任務地点の門は石の基礎、細い柱、梁、外向きの支え、反った屋根で組む。柱間は幅176、梁下は高さ246。石の基礎の間は幅158を確保し、隊列と軍旗が通る場所を開ける。
- 橋の四隅に石の橋台を追加。地面や橋の通行高さ、目的地の判定を変更しない。
- 建築は全て装飾のインスタンスまたは静的な布メッシュ。追加の移動・遮蔽・資源獲得判定は持たない。

## 森の表示補助

- 描画中の観測済み部隊と同じ位置を使い、その周囲の葉だけを透過する。選択中の部隊は半径130、その他は105、周辺35の幅でディザーを使ってなじませる。幹と森の地表色は残す。
- ６枠のパラメータは毎回観測結果から作り直す。隠れた予備隊や退場した部隊は無効な遠方座標へ戻し、葉の透過から位置が漏れないようにする。連鎖演出中も実際に表示している補間位置に追従する。
- 値が変わったパラメータだけを更新し、地形・植生・建築の作り直しは行わない。
- `Trial | Presentation | Readable Foliage` で表示補助を切替可能。通常は有効。森の移動条件・伏兵条件・視界・敵AI・行動消費は既存のまま。
- Editorのレベルには編集できる素材アセットを保存する。動的な素材インスタンスはゲーム中だけ作成する。

## ファイルの分担

| ファイル | 役割 |
|---|---|
| TacticalFortifications.cpp | 公開都市の拠点、任務の門、旗 |
| TacticalMapScenery.cpp | 集落・畑・橋台・周辺地形 |
| TacticalFoliage.cpp | 観測済み部隊の位置と葉の透過 |
| Tools/make_battlefield_props.py | ４種の独自OBJ形状を再現 |
| Tools/prepare_field_readability.py | 素材作成、モデル取込、標準素材の更新 |

OBJの保存先は `Unreal/ArtSource/Battle/SM_PalisadeStakeV4.obj`、`SM_WatchtowerV4.obj`、`SM_CampTentV4.obj`、`SM_SupplyCrateV4.obj`。単位はcm、Zが上。取り込み済みモデル・既存OBJ・独自の素材参照を再実行で上書きしない。外部モデルや新しい生成画像は使用していない。

素材は `/Game/Art/Materials/M_CommandCanopyV4`、`M_CampCanvasV4`、`M_CrateWoodV4`。４つのモデル参照はTacticalFieldの `Trial | Art` から差し替えられる。

## 確認

- Development Editorビルド成功：`Intermediate/field-v4-verified-build.log`。
- ４種類のOBJ取込、素材作成、参照の設定が成功：`Intermediate/field-v4-art.log`。
- 実行テスト７項目PASS：`Intermediate/field-v4-smoke.log`。既存の連鎖・旅程・戦法に加え、隠れた予備隊の透過パラメータ無効化、索敵で観測後の有効化、透過の完全オフ、行動非消費を確認。
- ３任務・２方向で計1,008のマス中央の選択と部隊の接地を維持。拠点の実モデル・２拠点の配置・装飾の衝突無効も確認。
- Editorでの13モデル・７素材・３肖像の参照、Masked素材の接続、任務変更後の再生成を確認：`Intermediate/field-v4-editor.log`。
- 同じ1600×900の撮影で、読み込み後90フレーム平均は森の透過あり5.73ms、なし5.80ms。通常プレイのFPS保証を表す測定ではない。
- 画像：透過あり `Intermediate/field-v4-forest.png`、なし `Intermediate/field-v4-forest-disabled.png`、拠点 `Intermediate/field-v4-camp.png`、門 `Intermediate/field-v4-gate.png`。

建築は後漢の特定の軍営を考証して復元したものではなく、この試作戦場のための独自形状。
