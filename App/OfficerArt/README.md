# 武将肖像 v1

内蔵 image_gen で生成したオリジナル肖像。3枚 × 24人 = 72人。

- shu-portraits-v1.png：劉備軍
- wei-portraits-v1.png：曹操軍
- wu-portraits-v1.png：孫権軍

各シートは4列×6行、実際の行境界は生成結果に合わせて `PortraitCrop` に記録。左から右・上から下。名前とセルの対応は `NewGame/PortraitCatalog.hpp`。生成指示は `PROMPTS.md`。部隊、情報パネル、戦法カットインで同じ対応を使います。人物像は創作表現です。旧 StoryArt の素材は保持します。
