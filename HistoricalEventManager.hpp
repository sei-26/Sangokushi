#pragma once
#include <Siv3D.hpp>
#include "CityData.hpp"

// 史実イベントシステム
class HistoricalEventManager
{
public:
	// 史実イベント
	struct HistoricalEvent
	{
		int year;
		int month;
		String title;
		String description;
		std::function<void(Array<CityData>&)> effect;
		bool triggered = false;
	};

	Array<HistoricalEvent> events;

	HistoricalEventManager()
	{
		InitializeEvents();
	}

	void InitializeEvents()
	{
		// 184年 - 黄巾の乱
		events.push_back({
			184, 3,
			U"⚔️ 黄巾の乱勃発！",
			U"張角率いる黄巾賊が各地で蜂起しました。\n全ての都市で治安が大幅に低下します。",
			[](Array<CityData>& cities) {
				for (auto& city : cities)
				{
					city.order = Max(0, city.order - 30);
				}
			}
		});

		// 189年 - 董卓の専横
		events.push_back({
			189, 4,
			U"👿 董卓、洛陽を制圧！",
			U"董卓が洛陽を制圧し、朝廷を牛耳りました。\n洛陽の治安が低下し、各地が混乱します。",
			[](Array<CityData>& cities) {
				for (auto& city : cities)
				{
					if (city.name == U"洛陽")
					{
						city.order = Max(0, city.order - 50);
					}
					else
					{
						city.order = Max(0, city.order - 15);
					}
				}
			}
		});

		// 190年 - 反董卓連合
		events.push_back({
			190, 1,
			U"🤝 反董卓連合結成！",
			U"各地の諸侯が董卓討伐のため連合を結成しました。\n全ての勢力の士気が上昇します。",
			[](Array<CityData>& cities) {
				for (auto& city : cities)
				{
					city.order = Min(100, city.order + 20);
				}
			}
		});

		// 200年 - 官渡の戦い
		events.push_back({
			200, 10,
			U"⚔️ 官渡の戦い！",
			U"曹操と袁紹が官渡で激突しました。\n天下の趨勢を決する大戦です。",
			[](Array<CityData>& cities) {
				// 特に効果なし（演出用）
			}
		});

		// 208年 - 赤壁の戦い
		events.push_back({
			208, 11,
			U"🔥 赤壁の戦い！",
			U"曹操の南下を劉備・孫権連合軍が阻止しました。\n天下三分の計が現実となります。",
			[](Array<CityData>& cities) {
				// 特に効果なし（演出用）
			}
		});

		// 221年 - 劉備、皇帝即位
		events.push_back({
			221, 4,
			U"👑 劉備、皇帝に即位！",
			U"劉備が蜀の皇帝に即位し、漢の復興を宣言しました。",
			[](Array<CityData>& cities) {
				// 特に効果なし（演出用）
			}
		});
	}

	// イベントチェック
	Optional<HistoricalEvent*> CheckEvent(int year, int month)
	{
		for (auto& event : events)
		{
			if (event.year == year && event.month == month && !event.triggered)
			{
				event.triggered = true;
				return &event;
			}
		}
		return none;
	}

	// イベントを適用
	void ApplyEvent(HistoricalEvent* event, Array<CityData>& cities)
	{
		if (event && event->effect)
		{
			event->effect(cities);
		}
	}
};
