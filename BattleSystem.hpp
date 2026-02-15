#pragma once
#include <Siv3D.hpp>

// 戦闘システム強化版
namespace BattleSystem
{
	// 地形タイプ
	enum class Terrain
	{
		Plains,   // 平地
		Mountain, // 山地
		Forest,   // 森
		River     // 川
	};

	// 天候
	enum class Weather
	{
		Sunny,  // 晴
		Rain,   // 雨
		Snow    // 雪
	};

	// 陣形
	enum class Formation
	{
		Offensive,  // 攻撃重視（攻+30%, 防-10%）
		Defensive,  // 防御重視（防+30%, 攻-10%）
		Balanced    // バランス
	};

	// 計略タイプ
	enum class Strategy
	{
		None,
		FireAttack,   // 火計（大ダメージ、雨天不可）
		Ambush,       // 伏兵（先制攻撃）
		Provoke,      // 挑発（敵の防御低下）
		Rally         // 鼓舞（味方の士気上昇）
	};

	// 地形ボーナスを取得
	static double GetTerrainBonus(Terrain terrain, bool isDefender)
	{
		switch (terrain)
		{
		case Terrain::Plains:
			return 1.0;  // ボーナスなし
		case Terrain::Mountain:
			return isDefender ? 1.4 : 0.8;  // 防御側+40%, 攻撃側-20%
		case Terrain::Forest:
			return isDefender ? 1.2 : 0.9;  // 防御側+20%, 攻撃側-10%
		case Terrain::River:
			return isDefender ? 1.5 : 0.7;  // 防御側+50%, 攻撃側-30%
		default:
			return 1.0;
		}
	}

	// 天候効果を取得
	static double GetWeatherEffect(Weather weather, Strategy strategy)
	{
		if (weather == Weather::Rain && strategy == Strategy::FireAttack)
		{
			return 0.3;  // 雨天時、火計の効果が30%に
		}
		if (weather == Weather::Snow)
		{
			return 0.9;  // 雪天時、全体的に戦闘力-10%
		}
		return 1.0;
	}

	// 陣形ボーナスを取得
	static std::pair<double, double> GetFormationBonus(Formation formation)
	{
		switch (formation)
		{
		case Formation::Offensive:
			return { 1.3, 0.9 };  // 攻+30%, 防-10%
		case Formation::Defensive:
			return { 0.9, 1.3 };  // 攻-10%, 防+30%
		case Formation::Balanced:
		default:
			return { 1.0, 1.0 };  // 変化なし
		}
	}

	// 地形名を取得
	static String GetTerrainName(Terrain terrain)
	{
		switch (terrain)
		{
		case Terrain::Plains: return U"平地";
		case Terrain::Mountain: return U"山地";
		case Terrain::Forest: return U"森林";
		case Terrain::River: return U"河川";
		default: return U"";
		}
	}

	// 地形色を取得
	static Color GetTerrainColor(Terrain terrain)
	{
		switch (terrain)
		{
		case Terrain::Plains: return Color(150, 200, 100);
		case Terrain::Mountain: return Color(120, 100, 80);
		case Terrain::Forest: return Color(50, 150, 50);
		case Terrain::River: return Color(100, 150, 255);
		default: return Palette::White;
		}
	}

	// 天候名を取得
	static String GetWeatherName(Weather weather)
	{
		switch (weather)
		{
		case Weather::Sunny: return U"晴";
		case Weather::Rain: return U"雨";
		case Weather::Snow: return U"雪";
		default: return U"";
		}
	}

	// 天候アイコンを取得
	static String GetWeatherIcon(Weather weather)
	{
		switch (weather)
		{
		case Weather::Sunny: return U"☀️";
		case Weather::Rain: return U"🌧️";
		case Weather::Snow: return U"❄️";
		default: return U"";
		}
	}

	// 陣形名を取得
	static String GetFormationName(Formation formation)
	{
		switch (formation)
		{
		case Formation::Offensive: return U"攻撃陣形";
		case Formation::Defensive: return U"防御陣形";
		case Formation::Balanced: return U"平衡陣形";
		default: return U"";
		}
	}

	// 計略名を取得
	static String GetStrategyName(Strategy strategy)
	{
		switch (strategy)
		{
		case Strategy::None: return U"なし";
		case Strategy::FireAttack: return U"火計";
		case Strategy::Ambush: return U"伏兵";
		case Strategy::Provoke: return U"挑発";
		case Strategy::Rally: return U"鼓舞";
		default: return U"";
		}
	}

	// ランダムな地形を生成
	static Terrain GenerateRandomTerrain()
	{
		int r = Random(0, 99);
		if (r < 40) return Terrain::Plains;    // 40%
		if (r < 65) return Terrain::Forest;    // 25%
		if (r < 85) return Terrain::Mountain;  // 20%
		return Terrain::River;                 // 15%
	}

	// ランダムな天候を生成
	static Weather GenerateRandomWeather()
	{
		int r = Random(0, 99);
		if (r < 70) return Weather::Sunny;  // 70%
		if (r < 90) return Weather::Rain;   // 20%
		return Weather::Snow;               // 10%
	}
}
