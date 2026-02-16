#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "VictoryCondition.hpp"

class EndingScene : public SceneBase
{
public:
	EndingScene(VictoryCondition::VictoryType type, int score, int cities, int year, int month)
		: m_victoryType(type)
		, m_score(score)
		, m_cityCount(cities)
		, m_year(year)
		, m_month(month)
	{
	}

	void update() override
	{
		if (KeySpace.down() || MouseL.down())
		{
			System::Exit();
		}
	}

	void draw() const override
	{
		Scene::SetBackground(ColorF(0.05, 0.05, 0.1));

		int w = Scene::Width();
		int h = Scene::Height();

		// 背景エフェクト
		for (int i = 0; i < 50; ++i)
		{
			double t = Scene::Time() + i * 0.5;
			Circle(w / 2 + std::cos(t) * 300, h / 2 + std::sin(t * 1.3) * 200, 2)
				.draw(ColorF(1, 1, 1, 0.1));
		}

		// タイトル
		String title = VictoryCondition::GetVictoryName(m_victoryType);
		Color titleColor = (m_victoryType == VictoryCondition::VictoryType::Defeat)
			? Palette::Red : Palette::Gold;

		FontAsset(U"huge")(title).drawAt(w / 2, h / 4, titleColor);

		// 統計
		int y = h / 2 - 100;
		FontAsset(U"title")(U"最終年: {}年{}月"_fmt(m_year, m_month)).drawAt(w / 2, y, Palette::White);
		y += 60;
		FontAsset(U"title")(U"支配都市: {}"_fmt(m_cityCount)).drawAt(w / 2, y, Palette::Lightgreen);
		y += 60;
		FontAsset(U"title")(U"スコア: {}"_fmt(m_score)).drawAt(w / 2, y, Palette::Yellow);

		// 評価
		y += 100;
		String rank = U"C";
		if (m_score >= 50000) rank = U"S";
		else if (m_score >= 30000) rank = U"A";
		else if (m_score >= 15000) rank = U"B";

		FontAsset(U"huge")(U"評価: " + rank).drawAt(w / 2, y, Palette::Gold);

		// 操作説明
		FontAsset(U"menu")(U"クリックまたはスペースキーで終了").drawAt(w / 2, h - 100, Palette::Gray);
	}

private:
	VictoryCondition::VictoryType m_victoryType;
	int m_score;
	int m_cityCount;
	int m_year;
	int m_month;
};
