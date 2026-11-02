# 戦闘効果音

`Tools/prepare_presentation.py` で数式と固定シードのノイズから生成したオリジナルの試作用効果音。外部録音・市販ゲームの音源は使用していない。

- S_March.wav: 行軍・追撃の足音と低い打音。
- S_Command.wav: 指揮・戦法開始の打音。
- S_Impact.wav: 命中・隊列衝突の打音。

22,050 Hz、16 bit、モノラル。UEの `Content/Audio/` に同名アセットを取り込む。演出の開始と命中のタイミングで鳴らし、音量は戦場アクターの `Effects Volume` で調整する。BGMは別工程。

`Tools/make_tactic_response_audio.py` は、一手成立・2連鎖・3連鎖・鼓舞・停止の短い応答音 `S_Tactic*V1.wav` を固定シードから生成する。0.55〜0.94秒、同じPCM形式。命中後の実結果で選び、停止は抑えた下降音にする。録音や他作品の音源は使用しない。UEの取り込みは `Tools/prepare_tactic_response_audio.py`。既存のWAVとSoundWaveは上書きせず、不足分だけを生成する。
