#pragma once
#include "SceneBase.hpp"
#include "GameManager.hpp"

// First playable career slice: choose an officer, serve monthly, earn promotion.
class CareerScene : public SceneBase
{
public:
	CareerScene(GameManager* gm, Array<CityData>* cities) : m_gm(gm), m_cities(cities) {}
	void update() override
	{
		auto& p = m_gm->career;
		if (p.officerName.isEmpty())
		{
			Array<std::pair<String, String>> candidates;
			for (const auto& city : *m_cities)
				if (city.owner == m_gm->playerFactionName)
					for (const auto& officer : city.officers)
						if (officer.name != city.owner) candidates.push_back({officer.name, city.name});
			if (candidates.isEmpty()) { p.report = U"この勢力には選択できる配下武将がいません。"; }
			else
			{
				if (Button(0).leftClicked()) m_choice = (m_choice + 1) % static_cast<int>(candidates.size());
				if (Button(1).leftClicked())
				{
					p.officerName = candidates[m_choice].first;
					p.cityName = candidates[m_choice].second;
					p.report = U"任務を達成して功績300で太守を目指しましょう。";
				}
				m_candidate = candidates[m_choice].first + U"（" + candidates[m_choice].second + U"）";
			}
		}
		else
		{
			for (int i = 0; i < 3; ++i) if (Button(i).leftClicked()) Serve(i);
		}
		if (Button(4).leftClicked()) { m_sceneEnd = true; m_nextScene = U"WorldMap"; }
	}
	void draw() const override
	{
		Scene::SetBackground(ColorF(0.10, 0.12, 0.16));
		const auto& p = m_gm->career;
		FontAsset(U"title")(U"武将の道").drawAt(Scene::Center().x, 70, Palette::Gold);
		FontAsset(U"menu")(p.officerName.isEmpty() ? m_candidate : p.officerName + U" / " + p.RoleName() + U" / " + p.cityName).drawAt(Scene::Center().x, 130);
		FontAsset(U"small")(U"功績：{} / 300　任務は月に1回・1コマンド"_fmt(p.merit)).drawAt(Scene::Center().x, 175);
		const Array<String> labels = p.officerName.isEmpty()
			? Array<String>{U"次の武将", U"この武将で開始"}
			: Array<String>{U"農業開発（金100）", U"商業振興（金100）", U"治安維持（金50）"};
		for (int i = 0; i < static_cast<int>(labels.size()); ++i)
		{
			Button(i).draw(Button(i).mouseOver() ? ColorF(0.35, 0.4, 0.5) : ColorF(0.2, 0.25, 0.3));
			FontAsset(U"menu")(labels[i]).drawAt(Button(i).center());
		}
		FontAsset(U"small")(p.report).draw(40, 470);
		Button(4).draw(ColorF(0.2, 0.25, 0.3));
		FontAsset(U"menu")(U"地図へ戻る").drawAt(Button(4).center());
	}
private:
	Rect Button(int i) const { return Rect(Scene::Center().x - 210, 220 + i * 65, 420, 50); }
	void Serve(int task)
	{
		auto& p = m_gm->career;
		const int date = m_gm->year * 12 + m_gm->month;
		if (p.completedMonth == date) { p.report = U"今月の任務は達成済みです。地図でEnterを押して翌月へ。"; return; }
		for (auto& city : *m_cities)
		{
			if (city.name != p.cityName || city.owner != m_gm->playerFactionName) continue;
			for (const auto& officer : city.officers)
			{
				if (officer.name != p.officerName) continue;
				const int cost = task == 2 ? 50 : 100;
				if (city.gold < cost) { p.report = U"都市の金が不足しています。"; return; }
				if (task == 2 && city.order >= 100) { p.report = U"治安は十分です。他の任務を選んでください。"; return; }
				if (!m_gm->turnManager.ExecuteCommand()) { p.report = U"今月のコマンドが残っていません。"; return; }
				city.gold -= cost;
				const int bonus = 10 + officer.GetAdministrationPower() / 10;
				if (task == 0) city.agriculture += bonus;
				else if (task == 1) city.commerce += bonus;
				else city.order = Min(100, city.order + bonus);
				p.completedMonth = date;
				p.merit += 50;
				p.report = U"任務達成！ 功績が50増えました。";
				if (!p.governor && p.merit >= 300) { p.governor = true; p.report += U" 太守に任命され、担当都市の統治が可能になりました！"; }
				return;
			}
		}
		p.report = U"担当武将が都市にいません。地図へ戻って所属を確認してください。";
	}
	GameManager* m_gm;
	Array<CityData>* m_cities;
	int m_choice = 0;
	String m_candidate;
};
