# 戦法用カットイン素材 v1

2026-10-26。imagegenスキルと組み込み image_gen で新規制作した、劉備・関羽・張飛のオリジナル人物絵。既存のプロジェクト肖像シートを顔・衣装の参照とし、他作品の人物画像は生成入力にもゲーム素材にも使用していない。

| 人物 | 元PNG | Unrealテクスチャ |
|---|---|---|
| 劉備 | [liu-bei-cutin-v1.png](liu-bei-cutin-v1.png) | /Game/Art/Officers/T_LiuCutinV1 |
| 関羽 | [guan-yu-cutin-v1.png](guan-yu-cutin-v1.png) | /Game/Art/Officers/T_GuanCutinV1 |
| 張飛 | [zhang-fei-cutin-v1.png](zhang-fei-cutin-v1.png) | /Game/Art/Officers/T_ZhangCutinV1 |

各1024×1536、RGBA。透過画素を確認。出力をそのままコピーし、画像の切り抜き・拡大・合成などの後処理はしていない。元の肖像シートは変更しない。UnrealではUI用、透過を保持する圧縮、非ストリーミングで取り込む。表示は比率を保って縮小する。

最終プロンプト全文、参照画像、生成方法、各ファイルのSHA256は [prompts-v1.json](prompts-v1.json)。Unrealへの取り込みは Tools/prepare_tactic_cinema_art.py。ゲーム側の配置・カメラ・実戦果の接続は [TACTIC_CINEMA_UPGRADE.md](../../../TACTIC_CINEMA_UPGRADE.md)。
