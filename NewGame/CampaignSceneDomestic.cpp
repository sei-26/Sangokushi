#include "CampaignScene.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"
using namespace frontline;
using namespace campaignui;
namespace
{
	Rect Close()
	{
		return Rect(Scene::Width() - 220, 116, 140, 32);
	}
	int DetailX()
	{
		return Scene::Width() / 2 + 24;
	}
	Rect Task(int duty)
	{
		const int width = (DetailX() - 104) / 2;
		return Rect(80 + (duty % 2) * (width + 12), 278 + (duty / 2) * 122, width, 110);
	}
	Rect Officer(bool next)
	{
		return Rect(DetailX() + (next ? 160 : 0), 386, 148, 32);
	}
	Rect Start()
	{
		return Rect(DetailX(), Scene::Height() - 118, 306, 38);
	}
	Rect Cancel()
	{
		return Rect(DetailX() + 318, Scene::Height() - 118, Scene::Width() - DetailX() - 398, 38);
	}
	int Level(const City& c, int duty)
	{
		return duty == 0 ? c.farming : duty == 1 ? c.commerce : duty == 2 ? c.order : c.logistics;
	}
	const char32_t* Purpose(int duty)
	{
		const char32_t* text[] = {U"兵糧の月収を増やす", U"金の月収を増やす", U"収入と出陣士気を支える",
		                          U"出陣時の携行兵糧を増やす"};
		return text[duty];
	}
} // namespace
std::vector<int> CampaignScene::domesticCandidates() const
{
	if (m_domesticCity < 0)
		return {};
	auto staff = m_game.Available(m_domesticCity);
	const auto duty = static_cast<Duty>(m_domesticDuty);
	std::stable_sort(staff.begin(), staff.end(), [&](int a, int b) {
		return m_game.WorkGain(a, duty) + m_game.GovernorWorkBonus(m_domesticCity, a) >
		       m_game.WorkGain(b, duty) + m_game.GovernorWorkBonus(m_domesticCity, b);
	});
	return staff;
}
void CampaignScene::openDomestic(int city)
{
	if (city < 0 || city >= static_cast<int>(m_game.cities.size()) ||
	    m_game.cities[city].owner != m_game.player)
	{
		city = -1;
		for (int i = 0; i < static_cast<int>(m_game.cities.size()); ++i)
			if (m_game.cities[i].owner == m_game.player)
			{
				city = i;
				break;
			}
	}
	if (city < 0)
	{
		m_message = U"内政を行う自勢力の都市がありません。";
		return;
	}
	m_domesticCity = city;
	m_domesticDuty = 0;
	const auto staff = domesticCandidates();
	m_domesticOfficer = staff.empty() ? -1 : staff.front();
	m_councilMode = 7;
	m_infoOpen = false;
	m_message = U"仕事 → 担当武将 → 費用と効果を確認 → 開始。";
}
String CampaignScene::domesticBlockReason() const
{
	const auto& city = m_game.cities[m_domesticCity];
	if (m_game.result != 0)
		return U"ゲーム終了後は内政を開始できません。";
	if (city.owner != m_game.player)
		return U"自勢力の都市で行ってください。";
	if (m_daysLeft > 0)
		return U"日数進行中。命令期間に入ると開始できます。";
	if (city.worker >= 0)
		return U"この城は事業が進行中。同時に1つまでです。";
	if (Level(city, m_domesticDuty) >= 100)
		return U"この能力は上限100に達しています。";
	if (m_domesticOfficer < 0)
		return U"待機武将がいません。帰還・任務終了・異動を待とう。";
	if (m_game.commands <= 0)
		return U"命令が不足。次の命令期間で回復します。";
	if (city.gold < 500)
		return U"この城の金が不足。月収で金500を確保しよう。";
	return {};
}
void CampaignScene::updateDomestic()
{
	if (m_domesticCity < 0 || m_domesticCity >= static_cast<int>(m_game.cities.size()))
	{
		m_councilMode = 0;
		return;
	}
	if (Close().leftClicked() || KeyEscape.down())
	{
		m_councilMode = 0;
		return;
	}
	if (Rect(1024, 168, 176, 32).leftClicked())
	{
		openPolicies();
		return;
	}
	for (bool next : {false, true})
		if (Rect(next ? 404 : 80, 168, 48, 32).leftClicked())
			for (int offset = 1; offset <= static_cast<int>(m_game.cities.size()); ++offset)
			{
				const int count = static_cast<int>(m_game.cities.size());
				const int city = (m_domesticCity + (next ? offset : -offset) + count) % count;
				if (m_game.cities[city].owner == m_game.player)
				{
					openDomestic(city);
					return;
				}
			}
	for (int duty = 0; duty < 4; ++duty)
		if (m_game.cities[m_domesticCity].worker < 0 && Task(duty).leftClicked())
		{
			m_domesticDuty = duty;
			const auto staff = domesticCandidates();
			m_domesticOfficer = staff.empty() ? -1 : staff.front();
		}
	const auto staff = domesticCandidates();
	if (std::find(staff.begin(), staff.end(), m_domesticOfficer) == staff.end())
		m_domesticOfficer = staff.empty() ? -1 : staff.front();
	for (bool next : {false, true})
		if (m_game.cities[m_domesticCity].worker < 0 && !staff.empty() && Officer(next).leftClicked())
		{
			auto it = std::find(staff.begin(), staff.end(), m_domesticOfficer);
			const int count = static_cast<int>(staff.size());
			const int current = static_cast<int>(it - staff.begin());
			m_domesticOfficer = staff[(current + (next ? 1 : count - 1)) % count];
		}
	if (Rect(560, 168, 220, 32).leftClicked())
	{
		m_governCity = m_domesticCity;
		m_governChoice = -1;
		m_governPage = 0;
		m_councilMode = 4;
		return;
	}
	if (Rect(792, 168, 220, 32).leftClicked())
	{
		m_governCity = m_domesticCity;
		m_trainingFirst = m_trainingSecond = -1;
		m_trainingPage = 0;
		m_trainingSlot = 0;
		m_councilMode = 6;
		return;
	}
	if (Start().leftClicked() && domesticBlockReason().isEmpty())
	{
		m_message = m_game.Develop(m_domesticCity, m_domesticOfficer, static_cast<Duty>(m_domesticDuty))
		                ? U"内政を開始。閉じて日数を進めると、30日後に効果が反映されます。"
		                : U"開始できませんでした。担当武将と都市の状態を確認してください。";
	}
	const auto& city = m_game.cities[m_domesticCity];
	if (Cancel().leftClicked() && city.worker >= 0 && city.owner == m_game.player && m_daysLeft == 0 &&
	    m_game.result == 0)
	{
		m_game.CancelWork(m_domesticCity);
		m_message = U"事業を中止。金は戻りません。担当武将は通常任務に戻りました。";
	}
}
void CampaignScene::drawDomestic() const
{
	if (m_domesticCity < 0 || m_domesticCity >= static_cast<int>(m_game.cities.size()))
		return;
	const auto& city = m_game.cities[m_domesticCity];
	const int d = DetailX(), w = Scene::Width() - d - 80;
	Scene::Rect().draw(ColorF(.015, .03, .025, .90));
	campaignui::Panel(RectF(60, 102, Scene::Width()-120, Scene::Height()-126),true);
	FontAsset(U"campaignTitle")(U"拠点内政").draw(80, 116, ColorF(.96, .86, .61));
	DrawButton(Close(), U"地図へ戻る");
	DrawButton(Rect(80, 168, 48, 32), U"←");
	FontAsset(U"campaignBody")(text(city.name) + U" / " +
	                           String(cityidentity::Name(m_game.CityKind(m_domesticCity))) + U" / " +
	                           String(citygrowth::Name(city.level)))
	    .draw(142, 171, ColorF(.95, .89, .70));
	DrawButton(Rect(404, 168, 48, 32), U"→");
	DrawButton(Rect(560, 168, 220, 32), U"太守・都市の特色");
	DrawButton(Rect(792, 168, 220, 32), U"共同訓練・親密度");
	DrawButton(Rect(1024, 168, 176, 32), U"国の政策");
	FontAsset(U"campaignSmall")(U"城の金 {} / 兵糧 {}　月収：金 {}・兵糧 {}"_fmt(
	                                city.gold, city.food, m_game.CityIncome(m_domesticCity, false),
	                                m_game.CityIncome(m_domesticCity, true)))
	    .draw(80, 212, ColorF(.84, .89, .79));
	FontAsset(U"campaignBody")(city.worker >= 0 ? U"① 進行中の事業" : U"① 仕事を選ぶ")
	    .draw(80, 242, ColorF(.98, .84, .51));
	FontAsset(U"campaignBody")(city.worker >= 0 ? U"② 担当武将" : U"② 担当武将を選ぶ")
	    .draw(d, 242, ColorF(.98, .84, .51));
	for (int duty = 0; duty < 4; ++duty)
	{
		const auto r = Task(duty);
		const bool selected = duty == (city.worker >= 0 ? city.work : m_domesticDuty);
		const int current = Level(city, duty);
		int gain = m_domesticOfficer >= 0 ? m_game.WorkGain(m_domesticOfficer, static_cast<Duty>(duty)) +
		                                        m_game.GovernorWorkBonus(m_domesticCity, m_domesticOfficer)
		                                  : 0;
		if (city.worker >= 0)
			gain = city.work == duty ? m_game.CityWorkGain(m_domesticCity) : 0;
		const int next = Min(100, current + gain);
		r.rounded(4)
		    .draw(selected ? ColorF(.21, .31, .41) : ColorF(.09, .16, .24))
		    .drawFrame(selected ? 2 : 1, ColorF(selected ? ColorF(.96, .78, .38) : ColorF(.41, .50, .57)));
		FontAsset(U"campaignBody")(text(Campaign::DutyName(static_cast<Duty>(duty))))
		    .draw(r.x + 12, r.y + 8, ColorF(.97, .88, .63));
		FontAsset(U"campaignSmall")(Purpose(duty)).draw(r.x + 12, r.y + 34, ColorF(.85, .91, .82));
		FontAsset(U"campaignBody")(current >= 100 ? U"100 / 上限" : U"{} → {} / 100"_fmt(current, next))
		    .draw(r.x + 12, r.y + 58, ColorF(.73, .92, .70));
		RectF(r.x + 12, r.y + 94, r.w - 24, 5).draw(ColorF(.035));
		RectF(r.x + 12, r.y + 94, (r.w - 24) * next / 100., 5).draw(ColorF(.86, .68, .28));
		RectF(r.x + 12, r.y + 94, (r.w - 24) * current / 100., 5).draw(ColorF(.36, .76, .49));
	}
	const int shown = city.worker >= 0 ? city.worker : m_domesticOfficer;
	if (shown >= 0)
	{
		const auto& g = m_game.generals[shown];
		campaignvisual::OfficerCard(RectF(d, 280, 76, 90), g, m_campaignFaces);
		FontAsset(U"campaignBody")(text(g.name) + (city.worker >= 0 ? U" / 事業担当" : U" / 待機中"))
		    .draw(d + 94, 281, ColorF(.95, .88, .66));
		(void)FontAsset(U"campaignSmall")(
		    U"政治 {} / 得意：{}\n個性：{}"_fmt(g.politics, text(Campaign::DutyName(g.specialty)),
		                                        text(TraitName(g.trait))))
		    .draw(RectF(d + 94, 315, w - 94, 66), ColorF(.82, .88, .78));
		FontAsset(U"campaignSmall")(U"担当中は出陣・異動できません").draw(d + 94, 360, ColorF(.82, .88, .78));
	}
	else
		FontAsset(U"campaignBody")(U"待機武将がいません").draw(d, 290, ColorF(.91, .70, .42));
	const auto staff = domesticCandidates();
	DrawButton(Officer(false), U"前の候補", city.worker < 0 && staff.size() > 1);
	DrawButton(Officer(true), U"次の候補", city.worker < 0 && staff.size() > 1);
	FontAsset(U"campaignSmall")(U"開発量が高い順 / {}人"_fmt(staff.size()))
	    .draw(d + 324, 394, ColorF(.73, .82, .70));
	FontAsset(U"campaignBody")(U"③ 費用と期間を確認").draw(d, 436, ColorF(.98, .84, .51));
	FontAsset(U"campaignSmall")(
	    U"この城の金 500 / 命令 1 / 期間 30日\n効果は完了時に反映。各城で同時に1事業。")
	    .draw(d, 469, ColorF(.87, .90, .81));
	FontAsset(U"campaignSmall")(
	    U"命令 {} / {}　次期上限 {}\n基本3＋1城目を除く2城ごとに1"_fmt(m_game.commands, m_game.commandBudget,
	                                                                   m_game.CommandLimit(m_game.player)))
	    .draw(d, 523, ColorF(.92, .84, .59));
	if (city.worker >= 0)
	{
		FontAsset(U"campaignBody")(U"進行中：{} / 残り{}日"_fmt(
		                               text(Campaign::DutyName(static_cast<Duty>(city.work))), city.workLeft))
		    .draw(80, 524, ColorF(.91, .85, .56));
		RectF(80, 557, d - 116, 8).draw(ColorF(.025));
		RectF(80, 557, (d - 116) * (30 - city.workLeft) / 30., 8).draw(ColorF(.40, .82, .51));
	}
	else
		(void)FontAsset(U"campaignSmall")(
		    domesticBlockReason().isEmpty() ? U"準備OK。右下の「内政を開始」で確定。" : domesticBlockReason())
		    .draw(RectF(80, 524, d - 116, 52), ColorF(.92, .78, .49));
	FontAsset(U"campaignSmall")(U"閉じて「進行開始」で日数を進める。\n10日進行なら3回で完了。")
	    .draw(80, Scene::Height() - 117, ColorF(.82, .88, .77));
	FontAsset(U"campaignSmall")(
	    city.level < 3 ? U"次の規模：{} / 4能力を各{}以上へ"_fmt(String(citygrowth::Name(city.level + 1)),
	                                                             citygrowth::Required(city.level + 1))
	                   : U"最大規模：都府 / 守備兵の募集上限27000")
	    .draw(80, 574, ColorF(.93, .80, .49));
	FontAsset(U"campaignSmall")(U"募集上限 {} / 城兵の被害 {}%"_fmt(m_game.GarrisonCapacity(m_domesticCity),
	                                                                m_game.CityDamagePercent(m_domesticCity)))
	    .draw(d, 574, ColorF(.82, .89, .73));
	DrawButton(Start(), U"内政を開始 / 金500・命令1", domesticBlockReason().isEmpty());
	DrawButton(Cancel(), U"中止 / 返金なし",
	           city.worker >= 0 && m_daysLeft == 0 && city.owner == m_game.player && m_game.result == 0);
	(void)FontAsset(U"campaignSmall")(m_message).draw(
	    RectF(80, Scene::Height() - 60, Scene::Width() - 160, 36), ColorF(.95, .85, .60));
}
