#pragma once
#include <Siv3D.hpp>

// 外交関係の種類
enum class DiplomaticStatus
{
	Enemy = 0,      // 宿敵 (-100〜-80)
	Hostile = 1,    // 敵対 (-79〜-40)
	Neutral = 2,    // 中立 (-39〜+39)
	Friendly = 3,   // 友好 (+40〜+79)
	Alliance = 4    // 同盟 (+80〜+100)
};

// 外交関係データ
struct DiplomaticRelation
{
	String faction1;  // 勢力1
	String faction2;  // 勢力2
	int relationValue = 0;  // 関係値 (-100〜+100)
	bool isAlliance = false;  // 同盟中
	int truceMonthsLeft = 0;  // 停戦残り月数

	// 関係値から状態を取得
	DiplomaticStatus GetStatus() const
	{
		if (isAlliance) return DiplomaticStatus::Alliance;
		if (relationValue >= 40) return DiplomaticStatus::Friendly;
		if (relationValue >= -39) return DiplomaticStatus::Neutral;
		if (relationValue >= -79) return DiplomaticStatus::Hostile;
		return DiplomaticStatus::Enemy;
	}

	// 関係値に応じた色
	Color GetColor() const
	{
		switch (GetStatus())
		{
		case DiplomaticStatus::Alliance:
			return Palette::Lime;
		case DiplomaticStatus::Friendly:
			return Palette::Lightgreen;
		case DiplomaticStatus::Neutral:
			return Palette::Gray;
		case DiplomaticStatus::Hostile:
			return Palette::Orange;
		case DiplomaticStatus::Enemy:
			return Palette::Red;
		default:
			return Palette::White;
		}
	}

	// 関係値に応じたテキスト
	String GetStatusText() const
	{
		switch (GetStatus())
		{
		case DiplomaticStatus::Alliance:
			return U"同盟";
		case DiplomaticStatus::Friendly:
			return U"友好";
		case DiplomaticStatus::Neutral:
			return U"中立";
		case DiplomaticStatus::Hostile:
			return U"敵対";
		case DiplomaticStatus::Enemy:
			return U"宿敵";
		default:
			return U"不明";
		}
	}

	// 戦争可能か（停戦中でないか）
	bool CanWar() const
	{
		return truceMonthsLeft <= 0 && !isAlliance;
	}
};

// 外交管理クラス
class DiplomacyManager
{
public:
	// 外交関係のマップ（勢力ペア → 関係データ）
	HashTable<String, DiplomaticRelation> relations;

	// キーを生成（常に小さい方を先に）
	static String MakeKey(const String& faction1, const String& faction2)
	{
		if (faction1 < faction2)
			return faction1 + U"_" + faction2;
		else
			return faction2 + U"_" + faction1;
	}

	// 関係を取得（なければ作成）
	DiplomaticRelation& GetRelation(const String& faction1, const String& faction2)
	{
		String key = MakeKey(faction1, faction2);

		if (!relations.contains(key))
		{
			DiplomaticRelation newRelation;
			newRelation.faction1 = faction1;
			newRelation.faction2 = faction2;
			newRelation.relationValue = 0;
			relations[key] = newRelation;
		}

		return relations[key];
	}

	// 同盟を結ぶ
	bool FormAlliance(const String& faction1, const String& faction2)
	{
		auto& relation = GetRelation(faction1, faction2);

		// 関係値が+60以上必要
		if (relation.relationValue >= 60)
		{
			relation.isAlliance = true;
			relation.relationValue = 80;
			return true;
		}
		return false;
	}

	// 同盟を破棄
	void BreakAlliance(const String& faction1, const String& faction2)
	{
		auto& relation = GetRelation(faction1, faction2);
		relation.isAlliance = false;
		relation.relationValue -= 50;  // 大幅に関係悪化
	}

	// 停戦協定
	void SignTruce(const String& faction1, const String& faction2, int months = 12)
	{
		auto& relation = GetRelation(faction1, faction2);
		relation.truceMonthsLeft = months;
	}

	// 贈り物で関係改善
	void GiveGift(const String& from, const String& to, int goldAmount)
	{
		auto& relation = GetRelation(from, to);
		int improvement = goldAmount / 100;  // 金100で+1
		relation.relationValue = Min(relation.relationValue + improvement, 100);
	}

	// 戦争による関係悪化
	void DeclareWar(const String& attacker, const String& defender)
	{
		auto& relation = GetRelation(attacker, defender);
		relation.relationValue -= 20;
		relation.isAlliance = false;
		relation.truceMonthsLeft = 0;
	}

	// 月次処理
	void AdvanceMonth()
	{
		for (auto& [key, relation] : relations)
		{
			// 停戦期間を減らす
			if (relation.truceMonthsLeft > 0)
			{
				relation.truceMonthsLeft--;
			}

			// 関係値を徐々に中立に戻す
			if (!relation.isAlliance)
			{
				if (relation.relationValue > 0)
				{
					relation.relationValue = Max(0, relation.relationValue - 1);
				}
				else if (relation.relationValue < 0)
				{
					relation.relationValue = Min(0, relation.relationValue + 1);
				}
			}
		}
	}

	// 全関係を取得
	Array<DiplomaticRelation> GetAllRelations() const
	{
		Array<DiplomaticRelation> result;
		for (const auto& [key, relation] : relations)
		{
			result.push_back(relation);
		}
		return result;
	}

	// 特定勢力の全関係を取得
	Array<DiplomaticRelation> GetFactionRelations(const String& factionName) const
	{
		Array<DiplomaticRelation> result;
		for (const auto& [key, relation] : relations)
		{
			if (relation.faction1 == factionName || relation.faction2 == factionName)
			{
				result.push_back(relation);
			}
		}
		return result;
	}

	// 2勢力が戦争可能か
	bool CanFightWar(const String& faction1, const String& faction2) const
	{
		String key = MakeKey(faction1, faction2);
		if (!relations.contains(key))
		{
			return true;  // 関係がなければ戦争可能
		}
		return relations.at(key).CanWar();
	}
};
