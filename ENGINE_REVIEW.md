# エンジンの見直しとルールの独立

2026-10-21。ユーザーの「一番かっこよく、グラフィックもゲーム部分もこだわれるなら何でもいい」「AIでグラフィックを上げやすい」「アンリアルで作り直そう」という決定を受け、開発先を **Unreal Engine 5** に変更。以前挙げたGodotはUI中心の場合の候補で、最終採用ではない。

## 現状の判断

軍師の戦術、主人公の進行、物資、親密度、敵の観測は標準C++で動作する。画面、入力、音声、肖像、カットイン、現在の保存処理はSiv3Dへ依存する。別エンジン側の接続と画面作成が必要。

立体的な戦場、遠景、大軍の見せ方、武将のドラマを重視して選定した。UEには広い屋外地形の [Landscape](https://dev.epicgames.com/documentation/en-us/unreal-engine/landscape-outdoor-terrain-in-unreal-engine)、カメラ・人物・アニメーションを時間軸で編集する [Sequencer](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-sequencer-movie-tool-overview)、制作工程を自動化する [Editor Python](https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python) がある。この機能の組み合わせが本作の目標に合うという設計判断であり、他エンジンでは実現できないという意味ではない。

このPCにはUE5.7.4が導入済み。CPUはi7-13700HX、RAM約32GB、GPUはRTX4060 Laptop・VRAM約8GB。これは開発環境の確認であり、大規模な完成戦場の性能保証ではない。まず仮の戦場で接続と操作を確認し、一戦を磨いて実測する。最初の試作ではLumen・レイトレーシング・仮想シャドウを有効化せず、戦術の視認性を先に確保する。

## AIで磨くための構成

`Unreal/GunyuTactics.uproject` がソース付きの編集用プロジェクト。共通C++ルールはそのまま静的ライブラリへビルドし、UEの入力・描画から呼ぶ。マップのライトと試作マテリアルはエディタで編集する。作戦と勝敗の計算をBlueprintへ二重実装しない。

`Unreal/ArtSource/manifest.json` に画像の元ファイル・保存先・出典を記録し、Editor Pythonで取り込む。AIで肖像・背景・地形テクスチャを作り、画像の取り込みと表示を分けて反復できる。Pythonは制作時だけの仕組み。ゲーム実行中の処理はC++／UEで行う。3D武将や兵士のモデル、リグ、動作、テクスチャの各種マップは画像生成だけでは完成しないため、別途制作・調整する。

最初のUE版は仮のHEX地形・兵士で連鎖、護送、防衛を接続する移行試作。既存の会話・音楽・カットイン・全国戦略・保存の移植や、販売用パッケージの検証は別工程。操作と再現手順は [Unreal/README.md](Unreal/README.md)。

## 今回の独立ビルド

ルートのCMakeLists.txtは `sangokushi_rules` 静的ライブラリを作る。Siv3DのSDK・画像・音源・ウィンドウに依存しない。Visual StudioのSangokushi.slnは既存ゲーム用として継続する。

```sh
cmake -S . -B Build/rules -DCMAKE_BUILD_TYPE=Debug
cmake --build Build/rules --config Debug
ctest --test-dir Build/rules -C Debug --output-on-failure
```

検証対象は連鎖、護送・防衛の目的、３戦のプレイヤー命令による完遂、従来のCampaign、観測と条件付き作戦の５テスト。Releaseでもassertを有効にする。Windowsではテストのスタックを8MiB確保する。

このPCではMinGW GCCで、Siv3Dをリンクせずにビルドし、５テストを通過した。MSVC＋Siv3Dの従来ゲームと全テストも通過済み。UE用にはMSVC14.44・動的CRT・Releaseで同じライブラリを作り、UE5.7.4のEditorモジュールへリンクした。macOS/Linuxや販売用パッケージはまだ未検証。JSONの保存処理は独立ライブラリには含めない。
