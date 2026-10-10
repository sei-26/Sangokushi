#pragma once
#include <string>

namespace frontline
{
	enum class Trait
	{
		Benevolent,
		Valiant,
		Guardian,
		Strategist,
		Merchant,
		Negotiator,
		Farmer,
		Administrator,
		Quartermaster,
		Raider,
		CavalryExpert,
		ArcherExpert,
		SiegeExpert,
		TerrainExpert,
		Resolute,
		Inspiring
	};
	enum class Tactic
	{
		Rally,
		Charge,
		Fortify,
		Volley,
		Fire,
		Supply,
		MountedCharge,
		Ambush,
		SiegeStrike
	};
	inline std::u32string TraitName(Trait t)
	{
		const char32_t* names[] = {U"仁徳",     U"勇猛", U"守城",     U"機略", U"富商", U"弁舌",
		                           U"屯田",     U"能吏", U"兵站巧者", U"奇襲", U"騎将", U"弓将",
		                           U"攻城巧者", U"地利", U"不屈",     U"人望"};
		return names[static_cast<int>(t)];
	}
	inline std::u32string TraitDescription(Trait t)
	{
		const char32_t* effects[] = {U"治安開発 +5。周囲の親しい味方から連携を得やすい。",
		                             U"野戦攻撃 +12%。",
		                             U"野戦で受ける損害 -12%。所在都市の謀略防御 +15。",
		                             U"謀略成功率 +15。所在都市の謀略防御 +10。",
		                             U"商業開発 +6。停戦交渉の成功率 +8。",
		                             U"停戦交渉の成功率 +18。治安開発 +3。",
		                             U"農政開発 +6。",
		                             U"すべての内政開発 +3。",
		                             U"兵站開発 +6。部隊の兵糧消費をさらに軽減。",
		                             U"謀略成功率 +10。森で野戦攻撃 +18%。",
		                             U"騎兵で平地にいると野戦攻撃 +20%。",
		                             U"弓兵の野戦攻撃 +18%。",
		                             U"都市への攻撃 +25%。",
		                             U"森・山で野戦攻撃 +15%、受ける損害 -15%。",
		                             U"戦闘・包囲による士気低下を半減。",
		                             U"2マス内の味方戦闘部隊の士気 +2/日。複数人では重複しない。"};
		return effects[static_cast<int>(t)];
	}
	inline std::u32string TacticName(Tactic t)
	{
		const char32_t* names[] = {U"鼓舞",     U"奮迅",     U"鉄壁", U"斉射", U"火計",
		                           U"兵站整備", U"騎兵突撃", U"伏兵", U"破城"};
		return names[static_cast<int>(t)];
	}
	inline std::u32string TacticDescription(Tactic t)
	{
		const char32_t* effects[] = {U"2マス内の味方の士気 +20。",
		                             U"5日間、自隊の攻撃 +35%。",
		                             U"5日間、自隊が受ける損害 -30%。",
		                             U"5日間、弓兵の射程 +1、攻撃 +20%。弓兵のみ。",
		                             U"2マス内の敵1隊、または隣接する攻撃先の敵都市に知力に応じた損害。",
		                             U"補給元から糧300を携行糧へ移し、士気 +15。接続が必要。",
		                             U"5日間、平地の騎兵の野戦攻撃 +60%。騎兵のみ。",
		                             U"5日間、森・山で野戦攻撃 +45%、被害 -20%。森・山で発動。",
		                             U"5日間、都市への攻撃 +60%。攻城隊のみ。"};
		return effects[static_cast<int>(t)];
	}
} // namespace frontline
