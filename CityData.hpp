#pragma once
#include <Siv3D.hpp>
#include "Officer.hpp"

// 前方宣言
namespace CityFacility { struct Facility; enum class Type; }

// 都市データ
struct CityData
{
	String name;
	Point pos;
	String owner;
	int troops = 5000;
	int gold = 1000;
	int food = 1000;
	int agriculture = 75;
	int commerce = 80;
	int barracks = 0;
	int order = 65;
	Color color = Palette::White;

	Array<Officer> officers;

	// ★ 施設（5つ）
	Array<CityFacility::Facility> facilities;

	// デフォルトコンストラクタ
	CityData()
	{
		// 施設スロットを初期化
		facilities.clear();
	}

	// 新しいコンストラクタ：名前・座標・所有者を指定して作成
	CityData(const String& name_, const Point& pos_, const String& owner_)
		: name(name_)
		, pos(pos_)
		, owner(owner_)
		, troops(5000)
		, gold(1000)
		, food(1000)
		, agriculture(75)
		, commerce(80)
		, barracks(0)
		, order(65)
		, color(Palette::White)
	{
		// 施設スロットは明示的にクリアしておく（他コードがresize等を行う）
		facilities.clear();
	}

	// 施設の効果を取得（CityFacility.hppをincludeした後に実装）
	int GetFacilityBonus(CityFacility::Type type) const;
	int GetBuildingCount() const;
	int GetEmptySlot() const;
	Array<String> AdvanceFacilities();
};

// CityFacility.hppをincludeした後に実装
#include "CityFacility.hpp"

inline int CityData::GetFacilityBonus(CityFacility::Type type) const
{
	int total = 0;
	for (const auto& facility : facilities)
	{
		if (facility.type == type && !facility.isBuilding)
		{
			total += facility.GetEffect();
		}
	}
	return total;
}

inline int CityData::GetBuildingCount() const
{
	int count = 0;
	for (const auto& facility : facilities)
	{
		if (facility.isBuilding) count++;
	}
	return count;
}

inline int CityData::GetEmptySlot() const
{
	for (int i = 0; i < facilities.size(); ++i)
	{
		if (facilities[i].type == CityFacility::Type::None)
		{
			return i;
		}
	}
	return -1;
}

inline Array<String> CityData::AdvanceFacilities()
{
	Array<String> completed;
	for (auto& facility : facilities)
	{
		if (facility.AdvanceBuild())
		{
			completed.push_back(facility.GetName() + U"Lv" + Format(facility.level) + U"が完成");
		}
	}
	return completed;
}
