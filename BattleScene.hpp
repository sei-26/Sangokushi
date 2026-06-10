#pragma once
#include "SceneBase.hpp"
#include "GameManager.hpp"
#include "BattleGameManager.hpp"
#include "OfficerSkill.hpp"

enum class BattlePhase {
    Planning,      // 作戦フェーズ（陣形・計略選択）
    Combat,        // 戦闘フェーズ（リアルタイム進行）
    SpecialSkill,  // 武将スキル発動
    Result         // 結果表示
};

// 武将の特殊スキル追加
struct BattleOfficerSkill {	
    String name;           // 例：「関羽の一騎当千」
    int cooldown;          // クールダウン
    double damageMultiplier; // ダメージ倍率
    String effect;         // 追加効果（士気低下など）
};

// 戦況の動的変化
struct BattleState {
    int playerMorale = 100;  // 士気（逃亡率に影響）
    int enemyMorale = 100;
    Array<String> eventLog;  // 戦闘ログ
    bool canRetreat = true;  // 撤退可能か
};

class BattleScene : public SceneBase
{
public:
	// ★ 修正：4引数バージョンに変更（isPlayerAttacker を追加）
	BattleScene(GameManager* gm, CityData& playerCity, CityData& enemyCity, bool isPlayerAttacker);

	void update() override;
	void draw() const override;

private:
	GameManager* m_gameManager;
	BattleGameManager m_battleManager;

	// ★ 修正：名前を明確化
	CityData* m_playerCityData;  // プレイヤーの都市
	CityData* m_enemyCityData;   // 敵の都市

	bool m_isPlayerAttacker;     // プレイヤーが攻撃側かどうか
	BattleSystem::Weather m_weather = BattleSystem::Weather::Sunny;
};
