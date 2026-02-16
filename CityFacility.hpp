#pragma once
#include <Siv3D.hpp>

// 都市施設システム
namespace CityFacility
{
	// 施設タイプ
	enum class Type
	{
		None,      // なし
		Farm,      // 農場
		Market,    // 市場
		Barracks,  // 兵舎
		School,    // 学校
		Wall       // 城壁
	};

	// 施設データ
	struct Facility
	{
		Type type = Type::None;
		int level = 0;              // レベル（1-5）
		bool isBuilding = false;    // 建設中か
		int buildTurnsLeft = 0;     // 残り建設ターン数

		// 施設名を取得
		String GetName() const
		{
			switch (type)
			{
			case Type::Farm: return U"農場";
			case Type::Market: return U"市場";
			case Type::Barracks: return U"兵舎";
			case Type::School: return U"学校";
			case Type::Wall: return U"城壁";
			default: return U"なし";
			}
		}

		// 施設アイコンを取得
		String GetIcon() const
		{
			switch (type)
			{
			case Type::Farm: return U"🌾";
			case Type::Market: return U"🏪";
			case Type::Barracks: return U"⚔️";
			case Type::School: return U"📚";
			case Type::Wall: return U"🏰";
			default: return U"";
			}
		}

		// 施設の説明を取得
		String GetDescription() const
		{
			switch (type)
			{
			case Type::Farm: return U"農業値+{}, 兵糧収入+"_fmt(GetEffect());
			case Type::Market: return U"商業値+{}, 金収入+"_fmt(GetEffect());
			case Type::Barracks: return U"徴兵効率+{}%"_fmt(GetEffect());
			case Type::School: return U"内政効率+{}%"_fmt(GetEffect());
			case Type::Wall: return U"防御力+{}%"_fmt(GetEffect());
			default: return U"";
			}
		}

		// 施設の効果値を取得
		int GetEffect() const
		{
			if (type == Type::None || level == 0) return 0;

			// レベルごとの基本効果
			switch (type)
			{
			case Type::Farm:
			case Type::Market:
				return 10 + (level - 1) * 5;  // Lv1:10, Lv2:15, Lv3:20, Lv4:25, Lv5:30
			case Type::Barracks:
			case Type::School:
			case Type::Wall:
				return 10 + (level - 1) * 10; // Lv1:10%, Lv2:20%, Lv3:30%, Lv4:40%, Lv5:50%
			default:
				return 0;
			}
		}

		// 建設コストを取得
		int GetBuildCost() const
		{
			if (type == Type::None) return 0;

			// レベルごとのコスト
			int baseCost = 0;
			switch (type)
			{
			case Type::Farm: baseCost = 500; break;
			case Type::Market: baseCost = 600; break;
			case Type::Barracks: baseCost = 800; break;
			case Type::School: baseCost = 1000; break;
			case Type::Wall: baseCost = 1200; break;
			default: baseCost = 0; break;
			}

			// レベルが上がるほどコスト増加
			return baseCost * level;
		}

		// 建設期間を取得（ターン数）
		int GetBuildTurns() const
		{
			if (type == Type::None) return 0;

			// レベルごとの建設期間
			return 1 + (level - 1);  // Lv1:1ターン, Lv2:2ターン, ...
		}

		// 建設開始
		void StartBuild(Type newType, int newLevel)
		{
			type = newType;
			level = newLevel;
			isBuilding = true;
			buildTurnsLeft = GetBuildTurns();
		}

		// 建設を1ターン進める
		bool AdvanceBuild()
		{
			if (!isBuilding) return false;

			buildTurnsLeft--;
			if (buildTurnsLeft <= 0)
			{
				isBuilding = false;
				buildTurnsLeft = 0;
				return true;  // 完成
			}
			return false;
		}

		// レベルアップ可能か
		bool CanUpgrade() const
		{
			return type != Type::None && level < 5 && !isBuilding;
		}
	};

	// 施設タイプ名を取得
	static String GetTypeName(Type type)
	{
		switch (type)
		{
		case Type::Farm: return U"農場";
		case Type::Market: return U"市場";
		case Type::Barracks: return U"兵舎";
		case Type::School: return U"学校";
		case Type::Wall: return U"城壁";
		default: return U"なし";
		}
	}

	// 施設タイプのアイコンを取得
	static String GetTypeIcon(Type type)
	{
		switch (type)
		{
		case Type::Farm: return U"🌾";
		case Type::Market: return U"🏪";
		case Type::Barracks: return U"⚔️";
		case Type::School: return U"📚";
		case Type::Wall: return U"🏰";
		default: return U"";
		}
	}

	// 施設の色を取得
	static Color GetTypeColor(Type type)
	{
		switch (type)
		{
		case Type::Farm: return Color(100, 200, 100);
		case Type::Market: return Color(255, 200, 50);
		case Type::Barracks: return Color(200, 50, 50);
		case Type::School: return Color(100, 150, 255);
		case Type::Wall: return Color(150, 150, 150);
		default: return Palette::Gray;
		}
	}
}
