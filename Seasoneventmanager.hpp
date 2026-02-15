#pragma once
#include <Siv3D.hpp>
#include "CityData.hpp"

// 季節イベントシステム
class SeasonEventManager
{
public:
	// 季節
	enum class Season
	{
		Spring,  // 春（1-3月）
		Summer,  // 夏（4-6月）
		Autumn,  // 秋（7-9月）
		Winter   // 冬（10-12月）
	};

	// イベントタイプ
	enum class EventType
	{
		GoodHarvest,   // 豊作
		BadHarvest,    // 凶作
		Plague,        // 疫病
		Flood,         // 洪水
		Drought,       // 干ばつ
		Earthquake,    // 地震
		Prosperity,    // 好景気
		Recession      // 不景気
	};

	// イベント結果
	struct EventResult
	{
		String title;
		String description;
		Array<String> effects;
		Color color;
	};

	// 月から季節を取得
	static Season GetSeason(int month)
	{
		if (month >= 1 && month <= 3) return Season::Spring;
		if (month >= 4 && month <= 6) return Season::Summer;
		if (month >= 7 && month <= 9) return Season::Autumn;
		return Season::Winter;
	}

	// 季節名を取得
	static String GetSeasonName(Season season)
	{
		switch (season)
		{
		case Season::Spring: return U"春";
		case Season::Summer: return U"夏";
		case Season::Autumn: return U"秋";
		case Season::Winter: return U"冬";
		default: return U"";
		}
	}

	// 季節色を取得
	static Color GetSeasonColor(Season season)
	{
		switch (season)
		{
		case Season::Spring: return Color(100, 200, 100);  // 緑
		case Season::Summer: return Color(255, 200, 50);   // 黄
		case Season::Autumn: return Color(200, 100, 50);   // 橙
		case Season::Winter: return Color(150, 200, 255);  // 青
		default: return Palette::White;
		}
	}

	// 季節イベント発生チェック
	static Optional<EventResult> CheckSeasonEvent(int year, int month, Array<CityData>& cities)
	{
		Season season = GetSeason(month);

		// 季節の最初の月のみイベント判定
		if (month % 3 != 1) return none;

		// ランダムでイベント発生（30%の確率）
		if (!RandomBool(0.3)) return none;

		// 季節ごとのイベント（デフォルト値を設定）
		EventType eventType = EventType::GoodHarvest;  // デフォルト

		switch (season)
		{
		case Season::Spring:
			eventType = Random(0, 1) == 0 ? EventType::GoodHarvest : EventType::Prosperity;
			break;
		case Season::Summer:
			eventType = Random(0, 1) == 0 ? EventType::Flood : EventType::Drought;
			break;
		case Season::Autumn:
			eventType = Random(0, 1) == 0 ? EventType::GoodHarvest : EventType::BadHarvest;
			break;
		case Season::Winter:
			eventType = Random(0, 1) == 0 ? EventType::Plague : EventType::Recession;
			break;
		default:
			eventType = EventType::GoodHarvest;
			break;
		}

		return ApplyEvent(eventType, cities);
	}

	// イベントを適用
	static EventResult ApplyEvent(EventType eventType, Array<CityData>& cities)
	{
		EventResult result;

		switch (eventType)
		{
		case EventType::GoodHarvest:
			result.title = U"🌾 豊作！";
			result.description = U"今年は豊作です。各都市の兵糧が増加します。";
			result.color = Color(100, 200, 100);
			for (auto& city : cities)
			{
				int bonus = city.agriculture * 2;
				city.food += bonus;
				result.effects.push_back(city.name + U": 兵糧+" + Format(bonus));
			}
			break;

		case EventType::BadHarvest:
			result.title = U"🍂 凶作...";
			result.description = U"天候不順により凶作です。各都市の兵糧が減少します。";
			result.color = Color(200, 100, 50);
			for (auto& city : cities)
			{
				int penalty = city.agriculture;
				city.food = Max(0, city.food - penalty);
				result.effects.push_back(city.name + U": 兵糧-" + Format(penalty));
			}
			break;

		case EventType::Plague:
			result.title = U"☠️ 疫病発生！";
			result.description = U"疫病が流行しています。各都市で兵士と治安が減少します。";
			result.color = Color(150, 50, 150);
			for (auto& city : cities)
			{
				int troopLoss = city.troops / 10;  // 10%減少
				city.troops = Max(0, city.troops - troopLoss);
				city.order = Max(0, city.order - 10);
				result.effects.push_back(city.name + U": 兵-" + Format(troopLoss) + U", 治安-10");
			}
			break;

		case EventType::Flood:
			result.title = U"🌊 洪水発生！";
			result.description = U"大雨により洪水が発生しました。農業と治安が低下します。";
			result.color = Color(50, 150, 255);
			for (auto& city : cities)
			{
				city.agriculture = Max(0, city.agriculture - 10);
				city.order = Max(0, city.order - 15);
				result.effects.push_back(city.name + U": 農業-10, 治安-15");
			}
			break;

		case EventType::Drought:
			result.title = U"☀️ 干ばつ発生！";
			result.description = U"雨が降らず干ばつが発生しました。兵糧が減少します。";
			result.color = Color(255, 200, 50);
			for (auto& city : cities)
			{
				int foodLoss = city.food / 5;  // 20%減少
				city.food = Max(0, city.food - foodLoss);
				result.effects.push_back(city.name + U": 兵糧-" + Format(foodLoss));
			}
			break;

		case EventType::Prosperity:
			result.title = U"💰 好景気！";
			result.description = U"商業が活発です。各都市の金が増加します。";
			result.color = Color(255, 215, 0);
			for (auto& city : cities)
			{
				int bonus = city.commerce * 3;
				city.gold += bonus;
				result.effects.push_back(city.name + U": 金+" + Format(bonus));
			}
			break;

		case EventType::Recession:
			result.title = U"📉 不景気...";
			result.description = U"経済が停滞しています。各都市の金が減少します。";
			result.color = Color(100, 100, 100);
			for (auto& city : cities)
			{
				int penalty = city.commerce;
				city.gold = Max(0, city.gold - penalty);
				result.effects.push_back(city.name + U": 金-" + Format(penalty));
			}
			break;
		}

		return result;
	}
};
