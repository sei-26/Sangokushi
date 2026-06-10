# ビルド警告修正計画

## 目標
C++ ビルドツールアップグレード後の **43件の警告をすべて修正** し、クリーンビルドを達成します。

## 修正方針

### 1. 型変換警告 (C4267, C4244) - 10件
**アプローチ**: 明示的な `static_cast` を使用してデータ損失の可能性を認識していることを示す

#### 1.1 size_t → int 変換 (3件)
- **VictoryCondition.hpp (2件)**
  - 行 20: `int totalCities = static_cast<int>(cities.size());`
  - 行 75: `score += static_cast<int>(city.officers.size()) * 100;`
- **ArmyConfigScene.cpp (1件)**
  - 行 22: `% static_cast<int>(m_fromCity->officers.size());`

#### 1.2 uint64 → uint32 変換 (4件)
- **Officerportrait.hpp (2件)**
  - 行 13, 141: `uint32 hash = static_cast<uint32>(officer.name.hash());`
- **Loyaltymanager.hpp (2件)**
  - 行 108, 109: `uint32 hash1 = static_cast<uint32>(officer1.hash());`

#### 1.3 int → Color::value_type 変換 (6件)
- **Csvdataloader.hpp (3件)**
  - 行 193: `Color(static_cast<uint8>(r), static_cast<uint8>(g), static_cast<uint8>(b))`
- **Saveloadmanager.hpp (3件)**
  - 行 99-101: `static_cast<uint8>(cityJSON[U"colorR"].get<int>())`

### 2. 未使用関数警告 (C4505) - 14件
**アプローチ**: 将来使用する可能性がある関数は `inline` キーワードを追加（ODR 違反を防ぐ）、または関数を削除

#### 2.1 BattleSystem.hpp (11件)
**判断**: これらはユーティリティ関数で将来使用される可能性が高い
- **解決策**: すべての static 関数に `inline` を追加
- 対象関数: `GetTerrainBonus`, `GetWeatherEffect`, `GetFormationBonus`, `GetTerrainName`, `GetTerrainColor`, `GetWeatherName`, `GetWeatherIcon`, `GetFormationName`, `GetStrategyName`, `GenerateRandomTerrain`, `GenerateRandomWeather`

#### 2.2 CityFacility.hpp & FileName.cpp (6件 - 重複)
**判断**: FileName.cpp が CityFacility.hpp をインクルードしているため重複警告
- **解決策**: CityFacility.hpp の関数に `inline` を追加
- 対象関数: `GetTypeName`, `GetTypeIcon`, `GetTypeColor`

### 3. 未使用パラメータ警告 (C4100) - 5件
**アプローチ**: `[[maybe_unused]]` 属性を追加して意図的に未使用であることを示す

- **HistoricalEventManager.hpp (3件)**
  - 行 80, 90, 100: `[[maybe_unused]] Array<CityData>& cities`
- **Aicontroller.hpp (1件)**
  - 行 68: `[[maybe_unused]] const Faction& playerFaction`
- **AudioManager.hpp (1件)**
  - 行 172: `[[maybe_unused]] SEType type`
- **Seasoneventmanager.hpp (1件)**
  - 行 76: `[[maybe_unused]] int year`

### 4. 未使用ローカル変数警告 (C4189) - 5件
**アプローチ**: 変数を削除するか、意図的に未使用の場合は `[[maybe_unused]]` を追加

#### 4.1 削除すべき変数 (3件)
- **Officerportrait.hpp (2件)**
  - 行 13, 141: `hash` 変数を削除（未使用）
- **Diplomacyscene.hpp (1件)**
  - 行 40: `mouseY` 変数を削除（未使用）

#### 4.2 今後使用予定の変数 (2件)
- **Aicontroller.hpp (1件)**
  - 行 156: `[[maybe_unused]] int defenderLoss` (戦闘ログなどで使用予定の可能性)

### 5. 変数名の隠蔽警告 (C4456) - 2件
**アプローチ**: 内側のスコープの変数名を変更

- **WorldMapScene_Visual.cpp**
  - 行 463: `double scale` → `double particleScale`
  - 行 491: `double scale` → `double effectScale`

### 6. 戻り値破棄警告 (C4834) - 1件
**アプローチ**: 戻り値を使用するか、明示的に破棄

- **CityScene_Visual.cpp**
  - 行 456-457: RectF の戻り値が使用されていない
  - **修正**: 2行を1つの式にまとめるか、戻り値を変数に格納して使用

## 実行順序

### フェーズ 1: 型変換警告の修正 (優先度: 高)
データ損失の可能性がある警告を最優先で修正

1. VictoryCondition.hpp
2. ArmyConfigScene.cpp
3. Officerportrait.hpp
4. Loyaltymanager.hpp
5. Csvdataloader.hpp
6. Saveloadmanager.hpp

### フェーズ 2: 未使用要素の整理 (優先度: 中)
コード品質を向上

7. BattleSystem.hpp (未使用関数に inline 追加)
8. CityFacility.hpp (未使用関数に inline 追加)
9. HistoricalEventManager.hpp (未使用パラメータ)
10. Aicontroller.hpp (未使用パラメータと変数)
11. AudioManager.hpp (未使用パラメータ)
12. Seasoneventmanager.hpp (未使用パラメータ)
13. Officerportrait.hpp (未使用変数削除)
14. Diplomacyscene.hpp (未使用変数削除)

### フェーズ 3: コード可読性の向上 (優先度: 中)
変数名の衝突と戻り値破棄の修正

15. WorldMapScene_Visual.cpp (変数名の隠蔽)
16. CityScene_Visual.cpp (戻り値破棄)

## 検証計画

各フェーズ後に以下を実行:
1. インクリメンタルビルド (`cppupgrade_build_and_get_issues`)
2. 警告数の減少を確認
3. 新しいエラー/警告が発生していないことを確認

最終検証:
1. フルリビルド (`cppupgrade_rebuild_and_get_issues`)
2. **エラー 0件、警告 0件** を達成

## 期待される成果
- ✅ すべての警告 (43件) を解消
- ✅ クリーンビルドの達成
- ✅ コード品質の向上
- ✅ 将来のメンテナンス性の向上
