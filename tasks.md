# 修正タスク一覧

## タスク 1: 型変換警告の修正 (10件)
**優先度**: 高  
**ファイル数**: 6  
**推定時間**: 15分

### 修正内容
- VictoryCondition.hpp: size_t → int 変換 (2箇所)
- ArmyConfigScene.cpp: size_t → int 変換 (1箇所)
- Officerportrait.hpp: uint64 → uint32 変換 (2箇所)
- Loyaltymanager.hpp: uint64 → uint32 変換 (2箇所)
- Csvdataloader.hpp: int → uint8 変換 (3箇所)
- Saveloadmanager.hpp: int → uint8 変換 (3箇所)

### 検証
- インクリメンタルビルドで C4267, C4244 警告が消えることを確認

---

## タスク 2: 未使用関数警告の修正 (14件)
**優先度**: 中  
**ファイル数**: 2  
**推定時間**: 10分

### 修正内容
- BattleSystem.hpp: 11個の static 関数に `inline` を追加
- CityFacility.hpp: 3個の static 関数に `inline` を追加

### 検証
- インクリメンタルビルドで C4505 警告が消えることを確認

---

## タスク 3: 未使用パラメータ警告の修正 (5件)
**優先度**: 中  
**ファイル数**: 4  
**推定時間**: 5分

### 修正内容
- HistoricalEventManager.hpp: `cities` パラメータに `[[maybe_unused]]` (3箇所)
- Aicontroller.hpp: `playerFaction` パラメータに `[[maybe_unused]]` (1箇所)
- AudioManager.hpp: `type` パラメータに `[[maybe_unused]]` (1箇所)
- Seasoneventmanager.hpp: `year` パラメータに `[[maybe_unused]]` (1箇所)

### 検証
- インクリメンタルビルドで C4100 警告が消えることを確認

---

## タスク 4: 未使用ローカル変数警告の修正 (5件)
**優先度**: 中  
**ファイル数**: 3  
**推定時間**: 5分

### 修正内容
- Officerportrait.hpp: `hash` 変数を削除 (2箇所)
- Diplomacyscene.hpp: `mouseY` 変数を削除 (1箇所)
- Aicontroller.hpp: `defenderLoss` に `[[maybe_unused]]` を追加 (1箇所)

### 検証
- インクリメンタルビルドで C4189 警告が消えることを確認

---

## タスク 5: 変数名の隠蔽警告の修正 (2件)
**優先度**: 中  
**ファイル数**: 1  
**推定時間**: 3分

### 修正内容
- WorldMapScene_Visual.cpp: 
  - 行 463: `scale` → `particleScale`
  - 行 491: `scale` → `effectScale`

### 検証
- インクリメンタルビルドで C4456 警告が消えることを確認

---

## タスク 6: 戻り値破棄警告の修正 (1件)
**優先度**: 中  
**ファイル数**: 1  
**推定時間**: 2分

### 修正内容
- CityScene_Visual.cpp: 行 456-457 の RectF 戻り値を適切に処理

### 検証
- インクリメンタルビルドで C4834 警告が消えることを確認

---

## タスク 7: 最終検証
**優先度**: 高  
**推定時間**: 5分

### 検証内容
1. フルリビルドを実行
2. エラー 0件、警告 0件を確認
3. ビルド成功を確認

### 成功条件
- ✅ すべての警告 (43件) が解消されている
- ✅ 新しいエラー/警告が発生していない
- ✅ ビルドが成功している

---

## 合計
- **総タスク数**: 7
- **総推定時間**: 45分
- **修正対象ファイル数**: 11
- **修正対象警告数**: 43
